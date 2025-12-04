#pragma once

#include <cstdint>
#include <vector>

namespace binghamton {
    
/// @brief
/// @param cover_symbols the binary cover data
/// @param syndrome_bits the binary message to be hidden
/// @param pricevector the vector of distortion weights
/// @param constraint_height the constraint height of the matrix
/// @param stego_symbols the computed stego data
double encode_stc(
    const std::vector<std::uint8_t>& cover_symbols,
    const std::vector<std::uint8_t>& syndrome_bits,
    const std::vector<std::uint8_t>& pricevector,
    const std::uint32_t constraint_height,
    std::vector<std::uint8_t>& stego_symbols);

/// @brief
/// @param stego_symbols
/// @param constraint_height
/// @param payload_bit_count
/// @param payload_bits_out
void decode_stc(
    const std::vector<std::uint8_t>& stego_symbols,
    const std::uint32_t constraint_height,
    const std::size_t payload_bit_count,
    std::vector<std::uint8_t>& syndrome_bits_out);

}