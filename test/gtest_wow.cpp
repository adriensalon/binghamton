#include "gtest_env.hpp"
#include <binghamton/method/wow.hpp>

struct gtest_wow : public gtest_env { };

TEST_F(gtest_wow, correctness)
{
    std::vector<std::uint8_t> _rgb;
    std::size_t _width, _height;
    load_default_image(_rgb, _width, _height);

    const std::vector<std::uint8_t> _payload = {
        1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1
    };

    std::array<std::uint8_t, 32> _steganography_key {};
    for (std::size_t _index = 0; _index < 32; ++_index) {
        _steganography_key[_index] = (std::uint8_t)_index;
    }

    const std::filesystem::path _current_dir = std::filesystem::temp_directory_path() / "binghamton";
    std::filesystem::create_directories(_current_dir);

    std::vector<std::uint8_t> _rgb_embedded;
    const double _cost = binghamton::embed_wow(_rgb, _width, _height, _steganography_key, _payload, _rgb_embedded);
    save_image(_current_dir / "output.png", _rgb_embedded, _width, _height);

    std::vector<std::uint8_t> _rgb_verify;
    std::size_t _width_verify, _height_verify;
    load_image(_current_dir / "output.png", _rgb_verify, _width_verify, _height_verify);

    std::vector<std::uint8_t> _payload_extracted;
    binghamton::extract_wow(_rgb_verify, _width_verify, _height_verify, _steganography_key, 3, _payload.size(), _payload_extracted);
    EXPECT_EQ(_payload, _payload_extracted);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}