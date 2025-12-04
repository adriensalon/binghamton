#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace binghamton {

    double embed_wow(
        const std::vector<std::uint8_t>& rgb,
        const std::size_t width,
        const std::size_t height,
        const std::array<std::uint8_t, 32> steg_key,
        const std::vector<std::uint8_t>& payload_bits,
        std::vector<std::uint8_t>& rgb_embedded);

    void extract_wow(
        const std::vector<std::uint8_t>& rgb_stego,
        const std::size_t width,
        const std::size_t height,
        const std::array<std::uint8_t, 32> steg_key,
        const std::uint32_t constraint_height,
        const std::size_t payload_bit_count,
        std::vector<std::uint8_t>& payload_bits_out);
}