#include "BMPIO.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>     // 提供 uint8_t, uint16_t, uint32_t, int32_t
#include <cstdlib>     // 提供 abs() 函数

// 如果上述头文件仍有问题，取消下面的注释
// #include <windows.h>

// BMP文件头结构 (14字节) - 使用 My 前缀避免与 windows.h 冲突
#pragma pack(push, 1)

struct MyBitmapFileHeader {
    uint16_t bfType;        // 文件类型，必须为 "BM" (0x4D42)
    uint32_t bfSize;        // 整个文件的大小（字节）
    uint16_t bfReserved1;   // 保留字段，必须为0
    uint16_t bfReserved2;   // 保留字段，必须为0
    uint32_t bfOffBits;     // 像素数据相对于文件头的偏移量（字节）
};

struct MyBitmapInfoHeader {
    uint32_t biSize;            // 信息头大小，固定为40字节
    int32_t  biWidth;           // 图像宽度（像素）
    int32_t  biHeight;          // 图像高度（像素）
    uint16_t biPlanes;          // 颜色平面数，必须为1
    uint16_t biBitCount;        // 每个像素的位数：1、8、24、32
    uint32_t biCompression;     // 压缩类型，0表示不压缩
    uint32_t biSizeImage;       // 像素数据大小（字节）
    int32_t  biXPelsPerMeter;   // 水平分辨率（像素/米）
    int32_t  biYPelsPerMeter;   // 垂直分辨率（像素/米）
    uint32_t biClrUsed;         // 实际使用的调色板颜色数
    uint32_t biClrImportant;    // 重要颜色数
};

struct MyRgbQuad {
    uint8_t rgbBlue;        // 蓝色分量
    uint8_t rgbGreen;       // 绿色分量
    uint8_t rgbRed;         // 红色分量
    uint8_t rgbReserved;    // 保留字段
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
                        pixelValue = (palette[bitValue].rgbRed > 127) ? 1 : 0;
                    }
                    else {
                        pixelValue = (palette[bitValue].rgbRed << 16) |
                            (palette[bitValue].rgbGreen << 8) |
                            palette[bitValue].rgbBlue;
                    }
                }
                else {
                    pixelValue = bitValue ? 1 : 0;
                }
            }
            else if (bitCount == 8) {
                uint8_t index = rowData[x];
                if (index < palette.size()) {
                    // 关键修复：对于灰度图像，直接使用灰度值
                    pixelValue = palette[index].rgbRed;
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
    fileHeader.bfOffBits = sizeof(MyBitmapFileHeader) + sizeof(MyBitmapInfoHeader)
        + paletteSize * sizeof(MyRgbQuad);
    fileHeader.bfSize = fileHeader.bfOffBits + dataSize;

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

    // 写入调色板
    if (bitCount == 1) {
        MyRgbQuad palette[2] = { 0 };
        palette[0].rgbBlue = 0;   palette[0].rgbGreen = 0;   palette[0].rgbRed = 0;
        palette[1].rgbBlue = 255; palette[1].rgbGreen = 255; palette[1].rgbRed = 255;
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

    // 准备像素数据
    std::vector<uint8_t> bmpData(dataSize, 0);

    for (int y = 0; y < height; y++) {
        int dstY = height - 1 - y;
        uint8_t* rowData = bmpData.data() + dstY * rowSize;

        for (int x = 0; x < width; x++) {
            uint32_t pixel = img.getPixel(x, y);

            if (bitCount == 1) {
                int byteIndex = x / 8;
                int bitOffset = 7 - (x % 8);
                int r = (pixel >> 16) & 0xFF;
                int g = (pixel >> 8) & 0xFF;
                int b = pixel & 0xFF;
                int luminance = (r * 299 + g * 587 + b * 114) / 1000;
                if (luminance > 127) {
                    rowData[byteIndex] |= (1 << bitOffset);
                }
            }
            else if (bitCount == 8) {
                // 关键修复：直接存储灰度值
                rowData[x] = static_cast<uint8_t>(pixel & 0xFF);
            }
            else if (bitCount == 24) {
                rowData[x * 3] = static_cast<uint8_t>(pixel & 0xFF);        // B
                rowData[x * 3 + 1] = static_cast<uint8_t>((pixel >> 8) & 0xFF); // G
                rowData[x * 3 + 2] = static_cast<uint8_t>((pixel >> 16) & 0xFF); // R
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