#include <cmath>
#include <stdexcept>

#include <binghamton/core/ycbcr.hpp>

namespace binghamton {
namespace {

    static constexpr int Y_R = 77; // 0.299 * 256
    static constexpr int Y_G = 150; // 0.587 * 256
    static constexpr int Y_B = 29; // 0.114 * 256

    inline bool _ensure_range(int v)
    {
        if (v < 0) {
            return false;
        }
        if (v > 255) {
            return false;
        }
        return true;
    }

}

void encode_y(
    const std::vector<std::uint8_t>& rgb,
    std::vector<std::uint8_t>& y)
{
    if (rgb.size() % 3 != 0) {
        throw std::runtime_error("rgb.size() must be a multiple of 3");
    }

    size_t pixel_count = rgb.size() / 3;
    y.resize(pixel_count);
    for (size_t i = 0; i < pixel_count; ++i) {
        uint8_t r = rgb[i * 3 + 0];
        uint8_t g = rgb[i * 3 + 1];
        uint8_t b = rgb[i * 3 + 2];
        int Y = (Y_R * r + Y_G * g + Y_B * b) >> 8;
        y[i] = static_cast<uint8_t>(Y);
    }
}

// void encode_ycbcr(
//     const std::vector<std::uint8_t>& rgb,
//     std::vector<std::uint8_t>& ycbcr)
// {
//     if (rgb.size() % 3 != 0) {
//         throw std::runtime_error("rgb.size() must be a multiple of 3");
//     }

//     const std::size_t _pixels_count = rgb.size() / 3;
//     ycbcr.resize(_pixels_count * 3);

//     for (std::size_t _pixel_index = 0; _pixel_index < _pixels_count; ++_pixel_index) {
//         const std::uint8_t _r = rgb[3 * _pixel_index + 0];
//         const std::uint8_t _g = rgb[3 * _pixel_index + 1];
//         const std::uint8_t _b = rgb[3 * _pixel_index + 2];

//         const float _y = 0.299f * _r + 0.587f * _g + 0.114f * _b;
//         const float _cb = -0.168736f * _r - 0.331264f * _g + 0.5f * _b + 128.f;
//         const float _cr = 0.5f * _r - 0.418688f * _g - 0.081312f * _b + 128.f;

//         ycbcr[3 * _pixel_index + 0] = _clamp(_y);
//         ycbcr[3 * _pixel_index + 1] = _clamp(_cb);
//         ycbcr[3 * _pixel_index + 2] = _clamp(_cr);
//     }
// }

bool decode_y(
    const std::vector<std::uint8_t>& rgb,
    const std::vector<std::uint8_t>& y,
    std::vector<std::uint8_t>& rgb_embedded)
{
    if (rgb.size() % 3 != 0) {
        throw std::runtime_error("rgb.size() must be a multiple of 3");
    }
    if (rgb.size() != 3 * y.size()) {
        throw std::runtime_error("rgb.size() must be equal to 3 * y.size()");
    }

    const std::size_t pixels_count = rgb.size() / 3;
    rgb_embedded.resize(rgb.size());

    for (std::size_t i = 0; i < pixels_count; ++i) {
        int R0 = rgb[3 * i + 0];
        int G0 = rgb[3 * i + 1];
        int B0 = rgb[3 * i + 2];

        // Original Y0
        int Y0 = (Y_R * R0 + Y_G * G0 + Y_B * B0) >> 8;

        int Y_new = y[i];
        int delta = Y_new - Y0;

        int R1 = R0 + delta;
        int G1 = G0 + delta;
        int B1 = B0 + delta;

        if (!_ensure_range(R1)) {
            return false;
        }
        if (!_ensure_range(G1)) {
            return false;
        }
        if (!_ensure_range(B1)) {
            return false;
        }

        rgb_embedded[3 * i + 0] = static_cast<std::uint8_t>(R1);
        rgb_embedded[3 * i + 1] = static_cast<std::uint8_t>(G1);
        rgb_embedded[3 * i + 2] = static_cast<std::uint8_t>(B1);
    }

    return true;
}

// void decode_ycbcr(
//     const std::vector<std::uint8_t>& ycbcr,
//     std::vector<std::uint8_t>& rgb)
// {
//     if (ycbcr.size() % 3 != 0) {
//         throw std::runtime_error("ycbcr.size() must be a multiple of 3");
//     }

//     const std::size_t _pixels_count = ycbcr.size() / 3;
//     rgb.resize(_pixels_count * 3);

//     for (std::size_t _pixel_index = 0; _pixel_index < _pixels_count; ++_pixel_index) {
//         const float _y = ycbcr[3 * _pixel_index + 0];
//         const float _cb = ycbcr[3 * _pixel_index + 1] - 128.f;
//         const float _cr = ycbcr[3 * _pixel_index + 2] - 128.f;

//         const float _r = _y + 1.402f * _cr;
//         const float _g = _y - 0.344136f * _cb - 0.714136f * _cr;
//         const float _b = _y + 1.772f * _cb;

//         rgb[3 * _pixel_index + 0] = _clamp(_r);
//         rgb[3 * _pixel_index + 1] = _clamp(_g);
//         rgb[3 * _pixel_index + 2] = _clamp(_b);
//     }
// }
}