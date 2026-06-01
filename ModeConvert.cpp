#include "ModeConvert.h"
#include <iostream>

// 单阈值法
Image ModeConvert::grayToBinaryT(const Image& img, int threshold) {
    if (img.getbitcount() != 8) {
        std::cerr << "Error: Not a grayscale image" << std::endl;
        return Image();
    }

    int width = img.getwidth();
    int height = img.getheight();
    Image result(width, height, 1, Image::Binary);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int grayValue = img.getPixel(x, y);
            result.setPixel(x, y, (grayValue > threshold) ? 1 : 0);
        }
    }

    std::cout << "Threshold method completed. Threshold = " << threshold << std::endl;
    return result;
}

// Dither抖动法
Image ModeConvert::grayToBinaryD(const Image& img, int size) {
    if (img.getbitcount() != 8) {
        std::cerr << "Error: Not a grayscale image" << std::endl;
        return Image();
    }

    int width = img.getwidth();
    int height = img.getheight();
    Image result(width, height, 1, Image::Binary);

    // 8x8 Bayer矩阵
    int bayer8[8][8] = {
        {0,32,8,40,2,34,10,42},
        {48,16,56,24,50,18,58,26},
        {12,44,4,36,14,46,6,38},
        {60,28,52,20,62,30,54,22},
        {3,35,11,43,1,33,9,41},
        {51,19,59,27,49,17,57,25},
        {15,47,7,39,13,45,5,37},
        {63,31,55,23,61,29,53,21}
    };

    int matrixSize = (size == 2 || size == 4 || size == 8) ? size : 8;
    int maxThreshold = matrixSize * matrixSize - 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gray = img.getPixel(x, y);
            int threshold = bayer8[y % matrixSize][x % matrixSize];
            int adjThreshold = (threshold * 255) / maxThreshold;
            result.setPixel(x, y, (gray > adjThreshold) ? 1 : 0);
        }
    }

    std::cout << "Dither method completed. Matrix size = " << matrixSize << std::endl;
    return result;
}

// Ordered Dither有序抖动法
Image ModeConvert::grayToBinaryOD(const Image& img, int size) {
    if (img.getbitcount() != 8) {
        std::cerr << "Error: Not a grayscale image" << std::endl;
        return Image();
    }

    int width = img.getwidth();
    int height = img.getheight();
    Image result(width, height, 1, Image::Binary);

    int bayer8[8][8] = {
        {0,32,8,40,2,34,10,42},
        {48,16,56,24,50,18,58,26},
        {12,44,4,36,14,46,6,38},
        {60,28,52,20,62,30,54,22},
        {3,35,11,43,1,33,9,41},
        {51,19,59,27,49,17,57,25},
        {15,47,7,39,13,45,5,37},
        {63,31,55,23,61,29,53,21}
    };

    int matrixSize = (size == 2 || size == 4 || size == 8) ? size : 8;
    int maxThreshold = matrixSize * matrixSize - 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gray = img.getPixel(x, y);
            int threshold = bayer8[y % matrixSize][x % matrixSize];
            float normThreshold = (float)threshold / maxThreshold;
            result.setPixel(x, y, (gray >= normThreshold * 255) ? 1 : 0);
        }
    }

    std::cout << "Ordered Dither method completed. Matrix size = " << matrixSize << std::endl;
    return result;
}

// 彩色转灰度
Image ModeConvert::colorToGray(const Image& img) {
    int bitcount = img.getbitcount();

    // 已经是灰度图
    if (bitcount == 8) {
        std::cout << "Already grayscale" << std::endl;
        Image result(img.getwidth(), img.getheight(), 8, Image::Gray);
        for (int y = 0; y < img.getheight(); y++) {
            for (int x = 0; x < img.getwidth(); x++) {
                result.setPixel(x, y, img.getPixel(x, y));
            }
        }
        return result;
    }

    // 二值转灰度
    if (bitcount == 1) {
        std::cout << "Converting binary to grayscale" << std::endl;
        Image result(img.getwidth(), img.getheight(), 8, Image::Gray);
        for (int y = 0; y < img.getheight(); y++) {
            for (int x = 0; x < img.getwidth(); x++) {
                result.setPixel(x, y, img.getPixel(x, y) * 255);
            }
        }
        return result;
    }

    // 彩色转灰度
    if (bitcount != 24 && bitcount != 32) {
        std::cerr << "Error: Unsupported format" << std::endl;
        return Image();
    }

    int width = img.getwidth();
    int height = img.getheight();
    Image result(width, height, 8, Image::Gray);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t pixel = img.getPixel(x, y);
            int r = Image::getRedComponent(pixel);
            int g = Image::getGreenComponent(pixel);
            int b = Image::getBlueComponent(pixel);
            int luminance = (int)(0.299 * r + 0.587 * g + 0.114 * b);
            // 手动限制范围，避免与Windows宏冲突
            if (luminance < 0) luminance = 0;
            if (luminance > 255) luminance = 255;
            result.setPixel(x, y, luminance);
        }
    }

    std::cout << "Color to Gray completed" << std::endl;
    return result;
}