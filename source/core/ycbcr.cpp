#include <cmath>
#include <stdexcept>

#include <binghamton/core/ycbcr.hpp>

namespace binghamton {
namespace {

    inline std::uint8_t _clamp(float v)
    {
        if (v < 0.0f) {
            v = 0.0f;
        }
        if (v > 255.0f) {
            v = 255.0f;
        }
        return static_cast<std::uint8_t>(std::lround(v));
    }
}

void encode_y(
    const std::vector<std::uint8_t>& rgb,
    std::vector<std::uint8_t>& y)
{
    if (rgb.size() % 3 != 0) {
        throw std::runtime_error("rgb.size() must be a multiple of 3");
    }

    const std::size_t _pixels_count = rgb.size() / 3;
    y.resize(_pixels_count);

    for (std::size_t _pixel_index = 0; _pixel_index < _pixels_count; ++_pixel_index) {
        const std::uint8_t _r = rgb[3 * _pixel_index + 0];
        const std::uint8_t _g = rgb[3 * _pixel_index + 1];
        const std::uint8_t _b = rgb[3 * _pixel_index + 2];
        const float _y = 0.299f * _r + 0.587f * _g + 0.114f * _b;
        y[_pixel_index] = _clamp(_y);
    }
}

void encode_ycbcr(
    const std::vector<std::uint8_t>& rgb,
    std::vector<std::uint8_t>& ycbcr)
{
    if (rgb.size() % 3 != 0) {
        throw std::runtime_error("rgb.size() must be a multiple of 3");
    }

    const std::size_t _pixels_count = rgb.size() / 3;
    ycbcr.resize(_pixels_count * 3);

    for (std::size_t _pixel_index = 0; _pixel_index < _pixels_count; ++_pixel_index) {
        const std::uint8_t _r = rgb[3 * _pixel_index + 0];
        const std::uint8_t _g = rgb[3 * _pixel_index + 1];
        const std::uint8_t _b = rgb[3 * _pixel_index + 2];

        const float _y = 0.299f * _r + 0.587f * _g + 0.114f * _b;
        const float _cb = -0.168736f * _r - 0.331264f * _g + 0.5f * _b + 128.f;
        const float _cr = 0.5f * _r - 0.418688f * _g - 0.081312f * _b + 128.f;

        ycbcr[3 * _pixel_index + 0] = _clamp(_y);
        ycbcr[3 * _pixel_index + 1] = _clamp(_cb);
        ycbcr[3 * _pixel_index + 2] = _clamp(_cr);
    }
}

void decode_y(
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

    const std::size_t _pixels_count = rgb.size() / 3;
    rgb_embedded.resize(_pixels_count * 3);

    for (std::size_t _pixel_index = 0; _pixel_index < _pixels_count; ++_pixel_index) {
        const std::uint8_t _r = rgb[3 * _pixel_index + 0];
        const std::uint8_t _g = rgb[3 * _pixel_index + 1];
        const std::uint8_t _b = rgb[3 * _pixel_index + 2];

        const float _y = static_cast<float>(y[_pixel_index]);
        const float _cb = -0.168736f * _r - 0.331264f * _g + 0.5f * _b;
        const float _cr = 0.5f * _r - 0.418688f * _g - 0.081312f * _b;

        const float _r_embedded = _y + 1.402f * _cr;
        const float _g_embedded = _y - 0.344136f * _cb - 0.714136f * _cr;
        const float _b_embedded = _y + 1.772f * _cb;

        rgb_embedded[3 * _pixel_index + 0] = _clamp(_r_embedded);
        rgb_embedded[3 * _pixel_index + 1] = _clamp(_g_embedded);
        rgb_embedded[3 * _pixel_index + 2] = _clamp(_b_embedded);
    }
}

void decode_ycbcr(
    const std::vector<std::uint8_t>& ycbcr,
    std::vector<std::uint8_t>& rgb)
{
    if (ycbcr.size() % 3 != 0) {
        throw std::runtime_error("ycbcr.size() must be a multiple of 3");
    }

    const std::size_t _pixels_count = ycbcr.size() / 3;
    rgb.resize(_pixels_count * 3);

    for (std::size_t _pixel_index = 0; _pixel_index < _pixels_count; ++_pixel_index) {
        const float _y = ycbcr[3 * _pixel_index + 0];
        const float _cb = ycbcr[3 * _pixel_index + 1] - 128.f;
        const float _cr = ycbcr[3 * _pixel_index + 2] - 128.f;

        const float _r = _y + 1.402f * _cr;
        const float _g = _y - 0.344136f * _cb - 0.714136f * _cr;
        const float _b = _y + 1.772f * _cb;

        rgb[3 * _pixel_index + 0] = _clamp(_r);
        rgb[3 * _pixel_index + 1] = _clamp(_g);
        rgb[3 * _pixel_index + 2] = _clamp(_b);
    }
}
}