#pragma once

#include <cstdint>
#include <vector>

namespace binghamton {

/// @brief Encodes RGB pixels to BT.601 Y pixels to separate luminance
/// @param rgb the RGB pixels to take as input
/// @param y the Y pixels to take as output
void encode_y(
    const std::vector<std::uint8_t>& rgb,
    std::vector<std::uint8_t>& y);

/// @brief Encodes RGB pixels to BT.601 YCbCr pixels to separate luminance
/// @param rgb the RGB pixels to take as input
/// @param ycbcr the YCbCr pixels to take as output
void encode_ycbcr(
    const std::vector<std::uint8_t>& rgb,
    std::vector<std::uint8_t>& ycbcr);

/// @brief Decodes RGB pixels from BT.601 Y pixels and original pixels
/// @param rgb the original RGB pixels to take as input
/// @param y the Y pixels to take as input
/// @param rgb_embedded the RGB pixels modified by the Y pixels
void decode_y(
    const std::vector<std::uint8_t>& rgb,
    const std::vector<std::uint8_t>& y,
    std::vector<std::uint8_t>& rgb_embedded);

/// @brief Decodes RGB pixels from BT.601 YCbCr pixels
/// @param ycbcr the YCbCr pixels to take as input
/// @param rgb the RGB pixels to take as output
void decode_ycbcr(
    const std::vector<std::uint8_t>& ycbcr,
    std::vector<std::uint8_t>& rgb);

}
