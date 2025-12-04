#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

struct gtest_env : public testing::Test {

    static void load_default_image(
        std::vector<std::uint8_t>& rgb,
        std::size_t& width,
        std::size_t& height);

    static void load_image(
        const std::filesystem::path& path,
        std::vector<std::uint8_t>& rgb,
        std::size_t& width,
        std::size_t& height);

    static void save_image(
        const std::filesystem::path& path,
        const std::vector<std::uint8_t>& rgb,
        const std::size_t& width,
        const std::size_t& height);
};