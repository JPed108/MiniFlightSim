#include "ImgIO.h"

#include "lodepng.h"

#include <cstddef>
#include <limits>
#include <utility>

namespace
{
unsigned channelCount(ImgIO::Format format)
{
    switch (format)
    {
        case ImgIO::Format::Gray: return 1;
        case ImgIO::Format::RGB: return 3;
    }

    return 0;
}

LodePNGColorType colorType(ImgIO::Format format)
{
    return format == ImgIO::Format::Gray ? LCT_GREY : LCT_RGB;
}

bool getPixelCount(unsigned width, unsigned height, unsigned channels,
                   size_t& count)
{
    if (width == 0 || height == 0 || channels == 0)
        return false;

    const size_t maxSize = std::numeric_limits<size_t>::max();
    if (static_cast<size_t>(width) > maxSize / height)
        return false;

    count = static_cast<size_t>(width) * height;
    if (count > maxSize / channels)
        return false;

    count *= channels;
    return count <= maxSize / sizeof(uint16_t);
}
}

bool ImgIO::loadPng(const std::string& path, Format format,
                    Image16& image, std::string& error)
{
    const unsigned outputChannels = channelCount(format);
    if (outputChannels == 0)
    {
        error = "Unsupported PNG pixel format";
        return false;
    }

    std::vector<unsigned char> bytes;
    unsigned width = 0;
    unsigned height = 0;
    const unsigned code = lodepng::decode(
        bytes, width, height, path, LCT_RGBA, 16);
    if (code)
    {
        error = "Failed to decode PNG '" + path + "': " + lodepng_error_text(code);
        return false;
    }

    size_t sourceSampleCount = 0;
    size_t outputSampleCount = 0;
    if (!getPixelCount(width, height, 4, sourceSampleCount) ||
        bytes.size() != sourceSampleCount * 2 ||
        !getPixelCount(width, height, outputChannels, outputSampleCount))
    {
        error = "PNG has invalid dimensions or data size: " + path;
        return false;
    }

    Image16 decoded;
    decoded.width = width;
    decoded.height = height;
    decoded.format = format;
    decoded.pixels.resize(outputSampleCount);
    const size_t imagePixelCount = sourceSampleCount / 4;
    for (size_t i = 0; i < imagePixelCount; ++i)
    {
        const size_t sourceIndex = i * 4;
        const auto readSample = [&bytes](size_t sampleIndex) {
            return static_cast<uint16_t>(
                (static_cast<uint16_t>(bytes[2 * sampleIndex]) << 8) |
                static_cast<uint16_t>(bytes[2 * sampleIndex + 1]));
        };

        const uint16_t red = readSample(sourceIndex);
        const uint16_t green = readSample(sourceIndex + 1);
        const uint16_t blue = readSample(sourceIndex + 2);

        if (format == Format::Gray)
        {
            const uint32_t luminance =
                77u * red + 150u * green + 29u * blue + 128u;
            decoded.pixels[i] = static_cast<uint16_t>(luminance >> 8);
        }
        else
        {
            const size_t outputIndex = i * 3;
            decoded.pixels[outputIndex] = red;
            decoded.pixels[outputIndex + 1] = green;
            decoded.pixels[outputIndex + 2] = blue;
        }
    }

    image = std::move(decoded);
    error.clear();
    return true;
}

bool ImgIO::savePng(const std::string& path, const Image16& image,
                    std::string& error)
{
    const unsigned channels = channelCount(image.format);
    size_t pixelCount = 0;
    if (channels == 0 ||
        !getPixelCount(image.width, image.height, channels, pixelCount) ||
        image.pixels.size() != pixelCount)
    {
        error = "Image has invalid dimensions, format, or pixel count: " + path;
        return false;
    }

    std::vector<unsigned char> bytes(pixelCount * 2);
    for (size_t i = 0; i < pixelCount; ++i)
    {
        bytes[2 * i] = static_cast<unsigned char>(image.pixels[i] >> 8);
        bytes[2 * i + 1] = static_cast<unsigned char>(image.pixels[i] & 0xFF);
    }

    const unsigned code = lodepng::encode(
        path, bytes, image.width, image.height, colorType(image.format), 16);
    if (code)
    {
        error = "Failed to encode PNG '" + path + "': " + lodepng_error_text(code);
        return false;
    }

    error.clear();
    return true;
}
