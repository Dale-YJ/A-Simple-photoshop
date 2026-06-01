#include "ModeConvert.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

// ======================== 单阈值法 ========================
Image ModeConvert::grayToBinaryT(const Image& img, int threshold) {
    // 检查是否为灰度图像
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

// ======================== 【修改1】误差扩散抖动法 (Floyd-Steinberg) ========================
// 原代码：使用 Bayer 矩阵的有序抖动
// 修改后：改为 Floyd-Steinberg 误差扩散抖动算法
Image ModeConvert::grayToBinaryD(const Image& img, int size) {
    // 参数 size 在误差扩散中不使用，保留是为了接口兼容
    if (img.getbitcount() != 8) {
        std::cerr << "Error: Not a grayscale image" << std::endl;
        return Image();
    }

    int width = img.getwidth();
    int height = img.getheight();
    Image result(width, height, 1, Image::Binary);

    // 创建浮点数组用于误差扩散（避免整数精度损失）
    vector<vector<double>> grayData(height, vector<double>(width));

    // 将图像数据复制到浮点数组中
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            grayData[y][x] = img.getPixel(x, y);
        }
    }

    // Floyd-Steinberg 误差扩散算法
    // 误差扩散权重矩阵：
    //     X   7/16
    // 3/16 5/16 1/16
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double oldPixel = grayData[y][x];
            double newPixel = (oldPixel > 127) ? 255.0 : 0.0;
            double error = oldPixel - newPixel;

            // 设置结果像素（二值：0或1）
            result.setPixel(x, y, (newPixel == 255.0) ? 1 : 0);

            // 将误差扩散到相邻的未处理像素
            // 向右扩散 7/16
            if (x + 1 < width) {
                grayData[y][x + 1] += error * 7.0 / 16.0;
            }
            // 向左下扩散 3/16
            if (y + 1 < height && x - 1 >= 0) {
                grayData[y + 1][x - 1] += error * 3.0 / 16.0;
            }
            // 向下扩散 5/16
            if (y + 1 < height) {
                grayData[y + 1][x] += error * 5.0 / 16.0;
            }
            // 向右下扩散 1/16
            if (y + 1 < height && x + 1 < width) {
                grayData[y + 1][x + 1] += error * 1.0 / 16.0;
            }
        }
    }

    std::cout << "Floyd-Steinberg Dither method completed." << std::endl;
    return result;
}

// ======================== 有序抖动法 (Ordered Dithering) ========================
// 使用 Bayer 矩阵的有序抖动（保持原实现，这是正确的）
Image ModeConvert::grayToBinaryOD(const Image& img, int size) {
    if (img.getbitcount() != 8) {
        std::cerr << "Error: Not a grayscale image" << std::endl;
        return Image();
    }

    int width = img.getwidth();
    int height = img.getheight();
    Image result(width, height, 1, Image::Binary);

    // 8x8 Bayer 矩阵（有序抖动的标准阈值矩阵）
    int bayer8[8][8] = {
        {0, 32, 8, 40, 2, 34, 10, 42},
        {48, 16, 56, 24, 50, 18, 58, 26},
        {12, 44, 4, 36, 14, 46, 6, 38},
        {60, 28, 52, 20, 62, 30, 54, 22},
        {3, 35, 11, 43, 1, 33, 9, 41},
        {51, 19, 59, 27, 49, 17, 57, 25},
        {15, 47, 7, 39, 13, 45, 5, 37},
        {63, 31, 55, 23, 61, 29, 53, 21}
    };

    // 确定矩阵大小（只支持 2、4、8，默认使用 8）
    int matrixSize = (size == 2 || size == 4 || size == 8) ? size : 8;
    int maxThreshold = matrixSize * matrixSize - 1;  // 最大阈值

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gray = img.getPixel(x, y);
            // 根据位置取模获取阈值
            int threshold = bayer8[y % matrixSize][x % matrixSize];
            // 将阈值归一化到 0-255 范围
            int adjThreshold = (threshold * 255) / maxThreshold;
            result.setPixel(x, y, (gray > adjThreshold) ? 1 : 0);
        }
    }

    std::cout << "Ordered Dither method completed. Matrix size = " << matrixSize << std::endl;
    return result;
}

// ======================== 彩色转灰度 ========================
Image ModeConvert::colorToGray(const Image& img) {
    int bitcount = img.getbitcount();

    // 情况1：已经是灰度图，直接复制返回
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

    // 情况2：二值图转灰度
    if (bitcount == 1) {
        std::cout << "Converting binary to grayscale" << std::endl;
        Image result(img.getwidth(), img.getheight(), 8, Image::Gray);
        for (int y = 0; y < img.getheight(); y++) {
            for (int x = 0; x < img.getwidth(); x++) {
                // 二值图：0 转为 0，1 转为 255
                result.setPixel(x, y, img.getPixel(x, y) * 255);
            }
        }
        return result;
    }

    // 情况3：彩色图（24位或32位）转灰度
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
            // 【修改2】使用整数运算的亮度公式，避免浮点精度问题
            // 亮度公式：Y = 0.299R + 0.587G + 0.114B
            int luminance = (r * 299 + g * 587 + b * 114) / 1000;
            // 限制范围（防止溢出）
            if (luminance < 0) luminance = 0;
            if (luminance > 255) luminance = 255;
            result.setPixel(x, y, luminance);
        }
    }

    std::cout << "Color to Gray completed" << std::endl;
    return result;
}