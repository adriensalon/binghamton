#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>

#include <binghamton/core/stc.hpp>

// TODO
// https://staff.emu.edu.tr/alexanderchefranov/Documents/CMSE492/Spring2019/FillerIEEETIFS2011%20Minimizing%20Additive%20Distortion%20in%20Steganography.pdf

namespace binghamton {
namespace {

    namespace {
        inline std::uint8_t _bit_from_symbol(std::int8_t v)
        {
            return static_cast<std::uint8_t>(v & 1);
        }

        inline std::uint8_t _bit_from_symbol(std::uint8_t v)
        {
            return v & 1u;
        }
    }
}

double encode_stc(
    const std::vector<std::uint8_t>& cover_symbols,
    const std::vector<std::uint8_t>& syndrome_bits,
    const std::vector<std::uint8_t>& pricevector,
    const std::uint32_t constraint_height,
    std::vector<std::uint8_t>& stego_symbols)
{
    (void)constraint_height; // reserved for a real trellis-based STC later

    const std::size_t n = cover_symbols.size();
    const std::size_t m = syndrome_bits.size();

    if (pricevector.size() != n)
        throw std::runtime_error("stc_encode: pricevector size must match cover_symbols size");

    if (m == 0) {
        stego_symbols.assign(n, 0);
        for (std::size_t i = 0; i < n; ++i)
            stego_symbols[i] = _bit_from_symbol(cover_symbols[i]);
        return 0.0;
    }

    if (m > n)
        throw std::runtime_error("stc_encode: payload length cannot exceed cover length in this implementation");

    // Block partition: n positions -> m blocks (almost equal size)
    const std::size_t base_block_size = n / m;
    const std::size_t remainder = n % m; // first 'remainder' blocks get +1 element

    // Start stego as copy of cover bits
    stego_symbols.resize(n);
    for (std::size_t i = 0; i < n; ++i)
        stego_symbols[i] = _bit_from_symbol(cover_symbols[i]);

    double total_price = 0.0;

    std::size_t idx = 0;
    for (std::size_t bit_idx = 0; bit_idx < m; ++bit_idx) {
        const std::size_t this_block_size = base_block_size + (bit_idx < remainder ? 1 : 0);

        const std::size_t start = idx;
        const std::size_t end = idx + this_block_size;
        idx = end;

        if (start >= end)
            break; // no more room

        const std::uint8_t target_bit = syndrome_bits[bit_idx] & 1u;

        // Compute parity of this block
        std::uint8_t parity = 0;
        for (std::size_t i = start; i < end; ++i)
            parity ^= (stego_symbols[i] & 1u);

        if (parity == target_bit) {
            // Block already encodes the bit; nothing to do.
            continue;
        }

        // Need to flip one symbol; pick minimal cost in this block
        float best_cost = std::numeric_limits<float>::infinity();
        std::size_t best_idx = end; // invalid

        for (std::size_t i = start; i < end; ++i) {
            const float c = static_cast<float>(pricevector[i]);
            if (c < best_cost) {
                best_cost = c;
                best_idx = i;
            }
        }

        if (best_idx == end)
            throw std::runtime_error("stc_encode: no valid position found in block");

        // Flip that bit
        stego_symbols[best_idx] ^= 1u;
        total_price += static_cast<double>(pricevector[best_idx]);
    }

    return total_price;
}

void decode_stc(
    const std::vector<std::uint8_t>& stego_symbols,
    const std::uint32_t constraint_height,
    const std::size_t payload_bit_count,
    std::vector<std::uint8_t>& syndrome_bits_out)
{
    (void)constraint_height; // reserved for future use

    const std::size_t n = stego_symbols.size();
    const std::size_t m = payload_bit_count;

    if (m == 0) {
        syndrome_bits_out.clear();
        return;
    }

    if (m > n)
        throw std::runtime_error("stc_decode: payload_bit_count cannot exceed stego length");

    const std::size_t base_block_size = n / m;
    const std::size_t remainder = n % m;

    syndrome_bits_out.clear();
    syndrome_bits_out.reserve(m);

    std::size_t idx = 0;
    for (std::size_t bit_idx = 0; bit_idx < m; ++bit_idx) {
        const std::size_t this_block_size = base_block_size + (bit_idx < remainder ? 1 : 0);

        const std::size_t start = idx;
        const std::size_t end = idx + this_block_size;
        idx = end;

        if (start >= end) {
            syndrome_bits_out.push_back(0);
            continue;
        }

        std::uint8_t parity = 0;
        for (std::size_t i = start; i < end; ++i)
            parity ^= _bit_from_symbol(stego_symbols[i]);

        syndrome_bits_out.push_back(parity & 1u);
    }
}

}