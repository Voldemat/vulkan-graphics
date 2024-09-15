#include "./image_loaders.hpp"

#include <stdlib.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <filesystem>

#include "assets.hpp"
#include "turbojpeg.h"

ImageData load_jpeg_image(const std::filesystem::path &filepath) {
    tjhandle _jpegDecompressor = tjInitDecompress();
    int width, height;
    int jpegSubsamp;
    const auto &compressedData = const_cast<uint8_t *>(checkJpgAsset.data());
    tjDecompressHeader2(_jpegDecompressor, compressedData, checkJpgAsset.size(),
                        &width, &height, &jpegSubsamp);
    assert(width > 0);
    assert(height > 0);
    uint8_t *buffer = new uint8_t[width * height * 4];
    tjDecompress2(_jpegDecompressor, compressedData, checkJpgAsset.size(),
                  buffer, width, 0 /*pitch*/, height, TJPF_RGBA,
                  TJFLAG_FASTDCT);
    tjDestroy(_jpegDecompressor);
    return { .width = static_cast<unsigned int>(width),
             .height = static_cast<unsigned int>(height),
             .buffer = buffer };
};
