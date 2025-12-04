#include <stb_image.h>
#include <stb_image_write.h>

#include "gtest_env.hpp"
#include "gtest_texture.hpp"

void gtest_env::load_default_image(std::vector<std::uint8_t>& rgb, std::size_t& width, std::size_t& height)
{
    int _width, _height, _channels;
    unsigned char* _rgb = stbi_load_from_memory(gtest_texture_png, gtest_texture_png_len, &_width, &_height, &_channels, 3);

    if (!_rgb) {
        throw std::runtime_error("Failed to load input.png");
    }
    if (_channels != 3) {
        throw std::runtime_error("Failed to load input.png, wrong channels count");
    }

    rgb = std::vector<std::uint8_t>(_rgb, _rgb + _width * _height * 3);
    width = static_cast<std::size_t>(_width);
    height = static_cast<std::size_t>(_height);
    stbi_image_free(_rgb);
}

void gtest_env::load_image(const std::filesystem::path& path, std::vector<std::uint8_t>& rgb, std::size_t& width, std::size_t& height)
{
    int _width, _height, _channels;
    const std::string _path = path.string();
    unsigned char* _rgb = stbi_load(_path.c_str(), &_width, &_height, &_channels, 3);

    if (!_rgb) {
        throw std::runtime_error("Failed to load input.png");
    }
    if (_channels != 3) {
        throw std::runtime_error("Failed to load input.png, wrong channels count");
    }

    rgb = std::vector<std::uint8_t>(_rgb, _rgb + _width * _height * 3);
    width = static_cast<std::size_t>(_width);
    height = static_cast<std::size_t>(_height);
    stbi_image_free(_rgb);
}

void gtest_env::save_image(const std::filesystem::path& path, const std::vector<std::uint8_t>& rgb, const std::size_t& width, const std::size_t& height)
{
    const std::string _path = path.string();
    stbi_write_png(_path.c_str(), static_cast<int>(width), static_cast<int>(height), 3, rgb.data(), static_cast<int>(width) * 3);
}