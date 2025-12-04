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

    std::array<std::uint8_t, 32> steg_key {};
    for (int i = 0; i < 32; ++i) {
        steg_key[i] = (std::uint8_t)i;
    } // TODO replace with real key

    const std::filesystem::path _current_dir = std::filesystem::temp_directory_path() / "binghamton";
    std::filesystem::create_directories(_current_dir);

    std::vector<std::uint8_t> _rgb_embedded;
    const double _cost = binghamton::embed_wow(_rgb, _width, _height, steg_key, _payload, _rgb_embedded);
    
    save_image(_current_dir / "output.png", _rgb_embedded, _width, _height);

    std::vector<std::uint8_t> _rgb_verify;
    std::size_t _width_verify, _height_verify;
    load_image(_current_dir / "output.png", _rgb_verify, _width_verify, _height_verify);

    std::vector<std::uint8_t> _payload_extracted;
    binghamton::extract_wow(_rgb_verify, _width_verify, _height_verify, steg_key, 3, _payload.size(), _payload_extracted);

    EXPECT_EQ(_payload, _payload_extracted);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}