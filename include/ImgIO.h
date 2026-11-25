#pragma once

#include <cstdint>
#include <string>
#include <vector>

class ImgIO
{
public:
    enum class Format
    {
        Gray,
        RGB
    };

    struct Image16
    {
        unsigned width = 0;
        unsigned height = 0;
        Format format = Format::Gray;
        std::vector<uint16_t> pixels;
    };

    static bool loadPng(const std::string& path, Format format,
                        Image16& image, std::string& error);
    static bool savePng(const std::string& path, const Image16& image,
                        std::string& error);
};
