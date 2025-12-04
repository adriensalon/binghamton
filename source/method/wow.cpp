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

bool embed_wow(
    const std::vector<std::uint8_t>& rgb,
    const std::size_t width,
    const std::size_t height,
    const std::array<std::uint8_t, 32> /*steg_key*/,
    const std::vector<std::uint8_t>& payload_bits,
    std::vector<std::uint8_t>& rgb_embedded,
    double& cost_embedded)
{
    if (rgb.size() != 3 * width * height) {
        throw std::runtime_error("embed_wow: rgb.size() must be equal to 3 * width * height");
    }

    const std::size_t pixels_count = width * height;
    constexpr std::size_t LENGTH_BITS = 32; // first 32 pixels store payload bit length (raw LSB)

    if (pixels_count <= LENGTH_BITS) {
        throw std::runtime_error("embed_wow: image too small to store length prefix");
    }

    // 1. Extract Y from RGB
    std::vector<std::uint8_t> Y;
    encode_y(rgb, Y); // from ycbcr.cpp

    if (Y.size() != pixels_count) {
        throw std::runtime_error("embed_wow: encode_y produced unexpected Y size");
    }

    // 2. Build cover symbols = LSBs of Y
    std::vector<std::uint8_t> cover_symbols;
    encode_lsb(Y, cover_symbols); // from lsb.cpp

    if (cover_symbols.size() != pixels_count) {
        throw std::runtime_error("embed_wow: encode_lsb produced unexpected symbol count");
    }

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

    // --- NEW: reserve first 32 pixels to store payload length in bits (raw) ---

    const std::size_t available_for_payload = pixels_count - LENGTH_BITS;

    if (payload_bits.size() == 0) {
        throw std::runtime_error("embed_wow: payload_bits is empty");
    }
    if (payload_bits.size() > available_for_payload) {
        throw std::runtime_error("embed_wow: payload does not fit in image (after length prefix)");
    }

    // Build cover + price for the STC part (skip first LENGTH_BITS pixels)
    std::vector<std::uint8_t> cover_symbols_stc(
        cover_symbols.begin() + static_cast<std::ptrdiff_t>(LENGTH_BITS),
        cover_symbols.end());

    std::vector<std::uint8_t> price_stc(
        price.begin() + static_cast<std::ptrdiff_t>(LENGTH_BITS),
        price.end());

    if (cover_symbols_stc.size() != available_for_payload || price_stc.size() != available_for_payload) {
        throw std::runtime_error("embed_wow: internal STC buffers have wrong size");
    }

    // 5. Run STC on the LSBs with distortion price (for payload_bits only)
    std::vector<std::uint8_t> stego_symbols_stc;
    constexpr std::uint32_t constraint_height = 3; // placeholder for real STC
    cost_embedded = encode_stc(
        cover_symbols_stc,
        payload_bits,
        price_stc,
        constraint_height,
        stego_symbols_stc); // from stc.cpp

    if (stego_symbols_stc.size() != available_for_payload) {
        throw std::runtime_error("embed_wow: encode_stc returned wrong symbol count");
    }

    // 6. Assemble full stego_symbols = [length_bits (raw)] + [STC-coded payload bits]
    std::vector<std::uint8_t> stego_symbols(pixels_count);

    // 6.1 Encode payload_bits.size() (in bits) as 32-bit big-endian, one bit per pixel LSB
    std::size_t payload_bit_len = payload_bits.size();
    std::array<std::uint8_t, LENGTH_BITS> length_bits {};
    for (std::size_t i = 0; i < LENGTH_BITS; ++i) {
        std::size_t shift = (LENGTH_BITS - 1) - i; // MSB first
        length_bits[i] = static_cast<std::uint8_t>((payload_bit_len >> shift) & 0x1u);
    }

    // First LENGTH_BITS pixels: raw length bits
    for (std::size_t i = 0; i < LENGTH_BITS; ++i) {
        stego_symbols[i] = length_bits[i]; // encode_lsb/decode_lsb treat these as bits (0/1)
    }

    // Remaining pixels: STC stego symbols
    for (std::size_t i = 0; i < available_for_payload; ++i) {
        stego_symbols[LENGTH_BITS + i] = stego_symbols_stc[i];
    }

    // 7. Apply stego LSBs back into Y
    std::vector<std::uint8_t> Y_stego;
    decode_lsb(Y, stego_symbols, Y_stego); // from lsb.cpp

    

    // 8. Rebuild RGB with new Y and original chroma
    return decode_y(rgb, Y_stego, rgb_embedded); // from ycbcr.cpp

}

void extract_wow(
    const std::vector<std::uint8_t>& rgb_stego,
    const std::size_t width,
    const std::size_t height,
    const std::array<std::uint8_t, 32> steganography_key,
    const std::uint32_t constraint_height,
    const std::size_t max_payload_bit_count,
    std::vector<std::uint8_t>& payload_bits_out)
{
    if (rgb_stego.size() != 3 * width * height) {
        throw std::runtime_error("extract_wow: rgb_stego.size() must be 3 * width * height");
    }

    const std::size_t pixels_count = width * height;
    constexpr std::size_t LENGTH_BITS = 32;

    if (pixels_count <= LENGTH_BITS) {
        throw std::runtime_error("extract_wow: image too small to contain length prefix");
    }
    if (max_payload_bit_count > width * height) {
        throw std::runtime_error("extract_wow: max_payload_bit_count > number of pixels");
    }
    if (max_payload_bit_count == 0) {
        payload_bits_out.clear();
        return;
    }

    const std::size_t available_for_payload = pixels_count - LENGTH_BITS;

    // 1. Extract Y from stego RGB
    std::vector<std::uint8_t> Y_stego;
    encode_y(rgb_stego, Y_stego);

    if (Y_stego.size() != pixels_count) {
        throw std::runtime_error("extract_wow: encode_y produced unexpected Y size");
    }

    // 2. Extract stego LSB symbols
    std::vector<std::uint8_t> stego_symbols;
    encode_lsb(Y_stego, stego_symbols);

    if (stego_symbols.size() != pixels_count) {
        throw std::runtime_error("extract_wow: encode_lsb produced unexpected symbol count");
    }

    // --- NEW: read 32-bit big-endian payload length from first LENGTH_BITS pixels ---
    std::size_t payload_bit_len = 0;
    for (std::size_t i = 0; i < LENGTH_BITS; ++i) {
        payload_bit_len = (payload_bit_len << 1) | (stego_symbols[i] & 0x1u);
    }

    if (payload_bit_len == 0) {
        // No payload
        payload_bits_out.clear();
        return;
    }

    if (payload_bit_len > max_payload_bit_count) {
        throw std::runtime_error("extract_wow: encoded payload length exceeds user cap");
    }
    if (payload_bit_len > available_for_payload) {
        throw std::runtime_error("extract_wow: encoded payload length does not fit in image");
    }

    // 3. Build STC symbols vector from remaining pixels
    std::vector<std::uint8_t> stc_symbols(
        stego_symbols.begin() + static_cast<std::ptrdiff_t>(LENGTH_BITS),
        stego_symbols.end());

    if (stc_symbols.size() != available_for_payload) {
        throw std::runtime_error("extract_wow: internal STC buffer has wrong size");
    }

    // 4. Decode with STC using the exact payload_bit_len we read from the header
    payload_bits_out.clear();
    decode_stc(stc_symbols, constraint_height, payload_bit_len, payload_bits_out);
}

} // namespace binghamton
