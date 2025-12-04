#pragma once

#include <cstdint>
#include <vector>

namespace binghamton {

/// @brief 
/// @param y 
/// @param y_lsb 
void encode_lsb(
    const std::vector<std::uint8_t>& y,
    std::vector<std::uint8_t>& y_lsb);

/// @brief 
/// @param y 
/// @param y_lsb 
void decode_lsb(
    const std::vector<std::uint8_t>& y,
    const std::vector<std::uint8_t>& y_lsb,
    std::vector<std::uint8_t>& y_embedded);

}