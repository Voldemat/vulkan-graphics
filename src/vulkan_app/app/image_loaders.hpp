#pragma once

#include <cstdint>
#include <filesystem>

struct ImageData {
    unsigned int width;
    unsigned int height;
    uint8_t* buffer;

    ~ImageData() {
        delete buffer;
    };
};

ImageData load_jpeg_image(const std::filesystem::path &filename);
