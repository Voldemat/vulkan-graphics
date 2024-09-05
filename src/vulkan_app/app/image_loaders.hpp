#pragma once

#include <filesystem>

struct ImageData {
    unsigned int width;
    unsigned int height;
    unsigned char* buffer;
};

ImageData load_png_image(const std::filesystem::path &filename);
ImageData load_jpeg_image(const std::filesystem::path &filename);
