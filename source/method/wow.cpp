#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include <binghamton/core/lsb.hpp>
#include <binghamton/core/stc.hpp>
#include <binghamton/core/ycbcr.hpp>
#include <binghamton/method/wow.hpp>

namespace binghamton {
namespace {

    // Horizontal high-pass
    static const float Kx[3][3] = {
        { 0.f, 0.f, 0.f },
        { -1.f, 2.f, -1.f },
        { 0.f, 0.f, 0.f }
    };

    // Vertical high-pass
    static const float Ky[3][3] = {
        { 0.f, -1.f, 0.f },
        { 0.f, 2.f, 0.f },
        { 0.f, -1.f, 0.f }
    };

    // Diagonal high-pass
    static const float Kd[3][3] = {
        { -1.f, 0.f, 0.f },
        { 0.f, 2.f, 0.f },
        { 0.f, 0.f, -1.f }
    };

    constexpr float epsilon = 1e-3f; // avoid division by zero

    inline std::uint8_t _clamp_u8(float v)
    {
        if (v < 0.0f)
            v = 0.0f;
        if (v > 255.0f)
            v = 255.0f;
        return static_cast<std::uint8_t>(std::lround(v));
    }

    // Safe access with clamping at borders
    inline float _get_y_pixel(const std::vector<std::uint8_t>& Y,
        std::size_t width,
        std::size_t height,
        int x, int y)
    {
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        if (x >= (int)width)
            x = static_cast<int>(width) - 1;
        if (y >= (int)height)
            y = static_cast<int>(height) - 1;
        return static_cast<float>(Y[static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)]);
    }

    // Simple 3x3 convolution on Y plane
    void _convolve3x3(const std::vector<std::uint8_t>& Y,
        std::size_t width,
        std::size_t height,
        const float kernel[3][3],
        std::vector<float>& out)
    {
        out.resize(width * height);
        for (std::size_t yy = 0; yy < height; ++yy) {
            for (std::size_t xx = 0; xx < width; ++xx) {
                float acc = 0.0f;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        float w = kernel[ky + 1][kx + 1];
                        acc += w * _get_y_pixel(Y, width, height, static_cast<int>(xx) + kx, static_cast<int>(yy) + ky);
                    }
                }
                out[yy * width + xx] = acc;
            }
        }
    }

} // namespace

double embed_wow(
    const std::vector<std::uint8_t>& rgb,
    const std::size_t width,
    const std::size_t height,
    const std::array<std::uint8_t, 32> /*steg_key*/,
    const std::vector<std::uint8_t>& payload_bits,
    std::vector<std::uint8_t>& rgb_embedded)
{
    if (rgb.size() != 3 * width * height) {
        throw std::runtime_error("embed_wow: rgb.size() must be equal to 3 * width * height");
    }

    const std::size_t pixels_count = width * height;

    // 1. Extract Y from RGB
    std::vector<std::uint8_t> Y;
    encode_y(rgb, Y); // from ycbcr.cpp :contentReference[oaicite:6]{index=6}

    if (Y.size() != pixels_count) {
        throw std::runtime_error("embed_wow: encode_y produced unexpected Y size");
    }

    // 2. Build cover symbols = LSBs of Y
    std::vector<std::uint8_t> cover_symbols;
    encode_lsb(Y, cover_symbols); // from lsb.cpp :contentReference[oaicite:7]{index=7}

    // 3. Compute WOW-like rho on Y
    std::vector<float> Rx, Ry, Rd;
    _convolve3x3(Y, width, height, Kx, Rx);
    _convolve3x3(Y, width, height, Ky, Ry);
    _convolve3x3(Y, width, height, Kd, Rd);

    std::vector<float> rho_f(pixels_count);
    for (std::size_t i = 0; i < pixels_count; ++i) {
        float e = std::fabs(Rx[i]) + std::fabs(Ry[i]) + std::fabs(Rd[i]);
        rho_f[i] = 1.0f / (e + epsilon); // higher activity -> lower cost
    }

    // 4. Normalize rho into pricevector [0..255]
    std::vector<std::uint8_t> price(pixels_count);
    {
        float max_rho = 0.0f;
        for (float r : rho_f) {
            if (r > max_rho)
                max_rho = r;
        }

        const float scale = (max_rho > 0.0f) ? (255.0f / max_rho) : 1.0f;
        for (std::size_t i = 0; i < pixels_count; ++i) {
            float v = rho_f[i] * scale;
            price[i] = _clamp_u8(v);
        }
    }

    // TODO: use steg_key to permute indices before STC (for extra security)
    // For now, we feed cover_symbols in raster order.

    // 5. Run STC on the LSBs with distortion price
    std::vector<std::uint8_t> stego_symbols;
    constexpr std::uint32_t constraint_height = 3; // placeholder for real STC
    double cost = encode_stc(
        cover_symbols,
        payload_bits,
        price,
        constraint_height,
        stego_symbols); // from stc.cpp :contentReference[oaicite:8]{index=8}

    if (stego_symbols.size() != pixels_count) {
        throw std::runtime_error("embed_wow: encode_stc returned wrong symbol count");
    }

    // 6. Apply stego LSBs back into Y
    std::vector<std::uint8_t> Y_stego;
    decode_lsb(Y, stego_symbols, Y_stego); // from lsb.cpp

    // 7. Rebuild RGB with new Y and original chroma
    decode_y(rgb, Y_stego, rgb_embedded); // from ycbcr.cpp

    return cost;
}

void extract_wow(
    const std::vector<std::uint8_t>& rgb_stego,
    const std::size_t width,
    const std::size_t height,
    const std::array<std::uint8_t, 32> /*steg_key*/,
    const std::uint32_t constraint_height,
    const std::size_t payload_bit_count,
    std::vector<std::uint8_t>& payload_bits_out)
{
    if (rgb_stego.size() != 3 * width * height) {
        throw std::runtime_error("extract_wow: rgb_stego.size() must be 3 * width * height");
    }
    if (payload_bit_count > width * height) {
        throw std::runtime_error("extract_wow: payload_bit_count > number of pixels");
    }
    if (payload_bit_count == 0) {
        return;
    }

    const std::size_t pixels_count = width * height;

    std::vector<std::uint8_t> Y_stego;
    encode_y(rgb_stego, Y_stego);

    std::vector<std::uint8_t> stego_symbols;
    encode_lsb(Y_stego, stego_symbols);

    // 3. (Future) if you permuted indices during embed using steg_key,
    //    you MUST undo the permutation here before calling decode_stc.

    decode_stc(stego_symbols, constraint_height, payload_bit_count, payload_bits_out);
}

} // namespace binghamton
