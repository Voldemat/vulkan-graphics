#include "./image_loaders.hpp"

#include <stdlib.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>

#include "png.h"
#include "pngconf.h"
#include "turbojpeg.h"

ImageData load_png_image(const std::filesystem::path &filepath) {
    FILE *file = fopen(filepath.c_str(), "rb");
    png_structp read_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                     nullptr, nullptr, nullptr);
    png_infop info_struct = png_create_info_struct(read_struct);
    png_init_io(read_struct, file);
    png_read_info(read_struct, info_struct);
    unsigned int height = png_get_image_height(read_struct, info_struct);
    unsigned int width = png_get_image_width(read_struct, info_struct);
    png_byte color_type = png_get_color_type(read_struct, info_struct);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(read_struct, 0xFF, PNG_FILLER_AFTER);
    };
    png_read_update_info(read_struct, info_struct);
    unsigned char *buffer = new unsigned char[width * height * 4];
    unsigned char *row_pointers[height];
    for (int y = 0; y < height; y++) {
        row_pointers[y] = &buffer[y * width * 4];
    }
    png_read_image(read_struct, row_pointers);
    png_destroy_read_struct(&read_struct, nullptr, NULL);
    fclose(file);
    return { .width = width, .height = height, .buffer = buffer };
};

ImageData load_jpeg_image(const std::filesystem::path &filepath) {
    std::ifstream imageStream(filepath, std::ios::binary);
    imageStream.seekg(0, std::ios::end);
    int filesize = imageStream.tellg();
    imageStream.seekg(0, std::ios::beg);

    unsigned char *imageCompressedData = new unsigned char[filesize];
    imageStream.read((char *)imageCompressedData, filesize);
    tjhandle _jpegDecompressor = tjInitDecompress();
    int width, height;
    int jpegSubsamp;
    tjDecompressHeader2(_jpegDecompressor, imageCompressedData, filesize,
                        &width, &height, &jpegSubsamp);
    unsigned char *buffer = new unsigned char[width * height * 4];
    tjDecompress2(_jpegDecompressor, imageCompressedData, filesize, buffer,
                  width, 0 /*pitch*/, height, TJPF_RGBA, TJFLAG_FASTDCT);
    tjDestroy(_jpegDecompressor);
    return { .width = static_cast<unsigned int>(width),
             .height = static_cast<unsigned int>(height),
             .buffer = (unsigned char *)buffer };
};
