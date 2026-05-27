#include "BMPIO.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

// BMP文件头结构 (14字节) - 使用 My 前缀避免与 windows.h 冲突
#pragma pack(push, 1)

struct MyBitmapFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

// BMP信息头结构 (40字节)
struct MyBitmapInfoHeader {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

// 调色板结构 (4字节)
struct MyRgbQuad {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
};

#pragma pack(pop)

bool BMPIO::read(const std::string& path, Image& img) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << path << std::endl;
        return false;
    }

    MyBitmapFileHeader fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(MyBitmapFileHeader));

    if (file.gcount() != sizeof(MyBitmapFileHeader)) {
        std::cerr << "Error: Failed to read file header" << std::endl;
        return false;
    }

    if (fileHeader.bfType != 0x4D42) {
        std::cerr << "Error: Not a valid BMP file" << std::endl;
        return false;
    }

    MyBitmapInfoHeader infoHeader;
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(MyBitmapInfoHeader));

    if (file.gcount() != sizeof(MyBitmapInfoHeader)) {
        std::cerr << "Error: Failed to read info header" << std::endl;
        return false;
    }

    if (infoHeader.biCompression != 0) {
        std::cerr << "Error: Compressed BMP not supported" << std::endl;
        return false;
    }

    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    int bitCount = infoHeader.biBitCount;

    if (bitCount != 1 && bitCount != 8 && bitCount != 24 && bitCount != 32) {
        std::cerr << "Error: Unsupported bit count " << bitCount << std::endl;
        return false;
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Invalid image size" << std::endl;
        return false;
    }

    int bytesPerPixel = (bitCount + 7) / 8;
    int rowSize = ((width * bitCount + 31) / 32) * 4;
    int dataSize = rowSize * height;

    std::vector<MyRgbQuad> palette;
    if (bitCount <= 8) {
        int paletteSize = (infoHeader.biClrUsed == 0) ? (1 << bitCount) : infoHeader.biClrUsed;
        palette.resize(paletteSize);

        size_t paletteBytes = paletteSize * sizeof(MyRgbQuad);
        file.read(reinterpret_cast<char*>(palette.data()), paletteBytes);
    }

    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    if (file.fail()) {
        std::cerr << "Error: Failed to seek to pixel data" << std::endl;
        return false;
    }

    std::vector<uint8_t> bmpData(dataSize);
    file.read(reinterpret_cast<char*>(bmpData.data()), dataSize);
    file.close();

    Image::Type type;
    switch (bitCount) {
    case 1:  type = Image::Binary; break;
    case 8:  type = Image::Gray;   break;
    default: type = Image::Color;  break;
    }

    img = Image(width, height, bitCount, type);

    bool isBottomUp = (infoHeader.biHeight > 0);

    for (int y = 0; y < height; y++) {
        int srcY = isBottomUp ? (height - 1 - y) : y;
        const uint8_t* rowData = bmpData.data() + srcY * rowSize;

        for (int x = 0; x < width; x++) {
            uint32_t pixelValue = 0;

            if (bitCount == 1) {
                int byteIndex = x / 8;
                int bitOffset = 7 - (x % 8);
                int bitValue = (rowData[byteIndex] >> bitOffset) & 0x01;

                if (bitValue < static_cast<int>(palette.size())) {
                    if (palette[bitValue].rgbRed == palette[bitValue].rgbBlue &&
                        palette[bitValue].rgbRed == palette[bitValue].rgbGreen) {
                        pixelValue = palette[bitValue].rgbRed;
                    }
                    else {
                        pixelValue = (palette[bitValue].rgbRed << 16) |
                            (palette[bitValue].rgbGreen << 8) |
                            palette[bitValue].rgbBlue;
                    }
                }
                else {
                    pixelValue = bitValue * 255;
                }
            }
            else if (bitCount == 8) {
                uint8_t index = rowData[x];

                if (index < palette.size()) {
                    if (palette[index].rgbRed == palette[index].rgbBlue &&
                        palette[index].rgbRed == palette[index].rgbGreen) {
                        pixelValue = palette[index].rgbRed;
                    }
                    else {
                        pixelValue = (palette[index].rgbRed << 16) |
                            (palette[index].rgbGreen << 8) |
                            palette[index].rgbBlue;
                    }
                }
                else {
                    pixelValue = index;
                }
            }
            else if (bitCount == 24) {
                uint8_t b = rowData[x * 3];
                uint8_t g = rowData[x * 3 + 1];
                uint8_t r = rowData[x * 3 + 2];
                pixelValue = (r << 16) | (g << 8) | b;
            }
            else if (bitCount == 32) {
                uint8_t b = rowData[x * 4];
                uint8_t g = rowData[x * 4 + 1];
                uint8_t r = rowData[x * 4 + 2];
                uint8_t a = rowData[x * 4 + 3];
                pixelValue = (a << 24) | (r << 16) | (g << 8) | b;
            }

            img.setPixel(x, y, pixelValue);
        }
    }

    std::cout << "Successfully loaded: " << width << "x" << height << ", " << bitCount << "bit" << std::endl;
    return true;
}

bool BMPIO::write(const std::string& path, const Image& img) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file " << path << std::endl;
        return false;
    }

    int width = img.getwidth();
    int height = img.getheight();
    int bitCount = img.getbitcount();

    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Invalid image size" << std::endl;
        return false;
    }

    int bytesPerPixel = (bitCount + 7) / 8;
    int rowSize = ((width * bitCount + 31) / 32) * 4;
    int dataSize = rowSize * height;

    int paletteSize = 0;
    if (bitCount == 1) {
        paletteSize = 2;
    }
    else if (bitCount == 8) {
        paletteSize = 256;
    }

    MyBitmapFileHeader fileHeader = { 0 };
    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits = sizeof(MyBitmapFileHeader) + sizeof(MyBitmapInfoHeader) + paletteSize * sizeof(MyRgbQuad);
    fileHeader.bfSize = fileHeader.bfOffBits + dataSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;

    MyBitmapInfoHeader infoHeader = { 0 };
    infoHeader.biSize = sizeof(MyBitmapInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = static_cast<uint16_t>(bitCount);
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = dataSize;
    infoHeader.biXPelsPerMeter = 3780;
    infoHeader.biYPelsPerMeter = 3780;
    infoHeader.biClrUsed = (bitCount <= 8) ? paletteSize : 0;
    infoHeader.biClrImportant = 0;

    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(MyBitmapFileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(MyBitmapInfoHeader));

    if (file.fail()) {
        std::cerr << "Error: Failed to write headers" << std::endl;
        return false;
    }

    if (bitCount == 1) {
        MyRgbQuad palette[2] = { 0 };
        palette[0].rgbBlue = 0;
        palette[0].rgbGreen = 0;
        palette[0].rgbRed = 0;
        palette[0].rgbReserved = 0;

        palette[1].rgbBlue = 255;
        palette[1].rgbGreen = 255;
        palette[1].rgbRed = 255;
        palette[1].rgbReserved = 0;

        file.write(reinterpret_cast<const char*>(palette), sizeof(palette));
    }
    else if (bitCount == 8) {
        std::vector<MyRgbQuad> palette(256);
        for (int i = 0; i < 256; i++) {
            palette[i].rgbBlue = static_cast<uint8_t>(i);
            palette[i].rgbGreen = static_cast<uint8_t>(i);
            palette[i].rgbRed = static_cast<uint8_t>(i);
            palette[i].rgbReserved = 0;
        }
        file.write(reinterpret_cast<const char*>(palette.data()), palette.size() * sizeof(MyRgbQuad));
    }

    std::vector<uint8_t> bmpData(dataSize, 0);

    for (int y = 0; y < height; y++) {
        int dstY = height - 1 - y;
        uint8_t* rowData = bmpData.data() + dstY * rowSize;

        for (int x = 0; x < width; x++) {
            uint32_t pixel = img.getPixel(x, y);

            if (bitCount == 1) {
                int byteIndex = x / 8;
                int bitOffset = 7 - (x % 8);

                uint8_t value;
                int r = (pixel >> 16) & 0xFF;
                int g = (pixel >> 8) & 0xFF;
                int b = pixel & 0xFF;
                int luminance = (r * 299 + g * 587 + b * 114) / 1000;
                value = (luminance > 127) ? 1 : 0;

                if (value) {
                    rowData[byteIndex] |= (1 << bitOffset);
                }
            }
            else if (bitCount == 8) {
                int r = (pixel >> 16) & 0xFF;
                int g = (pixel >> 8) & 0xFF;
                int b = pixel & 0xFF;
                int luminance = (r * 299 + g * 587 + b * 114) / 1000;
                rowData[x] = static_cast<uint8_t>(luminance);
            }
            else if (bitCount == 24) {
                rowData[x * 3] = static_cast<uint8_t>(pixel & 0xFF);
                rowData[x * 3 + 1] = static_cast<uint8_t>((pixel >> 8) & 0xFF);
                rowData[x * 3 + 2] = static_cast<uint8_t>((pixel >> 16) & 0xFF);
            }
            else if (bitCount == 32) {
                rowData[x * 4] = static_cast<uint8_t>(pixel & 0xFF);
                rowData[x * 4 + 1] = static_cast<uint8_t>((pixel >> 8) & 0xFF);
                rowData[x * 4 + 2] = static_cast<uint8_t>((pixel >> 16) & 0xFF);
                rowData[x * 4 + 3] = static_cast<uint8_t>((pixel >> 24) & 0xFF);
            }
        }
    }

    file.write(reinterpret_cast<const char*>(bmpData.data()), dataSize);

    if (file.fail()) {
        std::cerr << "Error: Failed to write pixel data" << std::endl;
        return false;
    }

    file.close();

    std::cout << "Successfully saved: " << path << std::endl;
    return true;
}