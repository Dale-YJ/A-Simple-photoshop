#include "ModeConvert.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

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

// ======================== 辅助函数：判断是否为2的幂次 ========================
static bool isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// ======================== 辅助函数：生成 n×n 均匀阈值矩阵 ========================
// 用于 Dither 方法，矩阵值为 0 到 n?-1 的均匀分布
static vector<vector<int>> generateUniformDitherMatrix(int n) {
    vector<vector<int>> matrix(n, vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = i * n + j;
        }
    }
    return matrix;
}

// ======================== 辅助函数：递归生成任意 2^k 大小的 Bayer 矩阵 ========================
// 递归公式：M(2n) = [ 4*M(n) + 0,   4*M(n) + 2 ]
//                    [ 4*M(n) + 3,   4*M(n) + 1 ]
static vector<vector<int>> generateBayerMatrix(int n) {
    vector<vector<int>> matrix(n, vector<int>(n));

    if (n == 1) {
        matrix[0][0] = 0;
        return matrix;
    }

    if (n == 2) {
        // 2x2 Bayer 矩阵（基础情况）
        matrix[0][0] = 0;  matrix[0][1] = 2;
        matrix[1][0] = 3;  matrix[1][1] = 1;
        return matrix;
    }

    // 递归生成 n/2 大小的 Bayer 矩阵
    int half = n / 2;
    vector<vector<int>> subMatrix = generateBayerMatrix(half);

    // 扩展为 n×n 矩阵
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            int val = subMatrix[i][j];
            matrix[i][j] = 4 * val;      // 左上
            matrix[i][j + half] = 4 * val + 2;  // 右上
            matrix[i + half][j] = 4 * val + 3;  // 左下
            matrix[i + half][j + half] = 4 * val + 1; // 右下
        }
    }

    return matrix;
}

// ======================== 方法2：Dither（抖动法） ========================
// 算法步骤（根据课件）：
// 1. 将图像像素值线性映射到 0 ~ size? 范围
// 2. 将图像放大为 size*W × size*H 的矩阵，每个 size×size 子块填充相同值
// 3. 用 size×size 的阈值矩阵 D 在放大图像上滑动，进行二值化：
//    B[i][j] = 1 if A[i][j] > D(x,y) else 0
Image ModeConvert::grayToBinaryD(const Image& img, int size) {
    // 检查是否为灰度图像
    if (img.getbitcount() != 8) {
        std::cerr << "Error: Not a grayscale image" << std::endl;
        return Image();
    }

    if (size <= 0) {
        std::cerr << "Error: Invalid dither matrix size" << std::endl;
        return Image();
    }

    int width = img.getwidth();
    int height = img.getheight();

    int maxMappedValue = size * size;

    // 生成均匀阈值矩阵 D (size × size)
    vector<vector<int>> ditherMatrix = generateUniformDitherMatrix(size);

    // 结果图像（放大 size 倍）
    Image result(width * size, height * size, 1, Image::Binary);

    // 步骤1: 将原始图像值映射到 0 ~ size? 范围
    vector<vector<int>> mappedData(height, vector<int>(width));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gray = img.getPixel(x, y);
            // 线性映射: [0,255] -> [0, maxMappedValue]
            int mapped = (gray * maxMappedValue) / 255;
            mappedData[y][x] = mapped;
        }
    }

    // 步骤2 & 3: 对每个原始像素，在其对应的 size×size 子块中进行二值化
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int mappedValue = mappedData[y][x];

            // 当前像素对应的 size×size 子块在结果图像中的位置
            int blockStartX = x * size;
            int blockStartY = y * size;

            // 在子块中根据阈值矩阵进行二值化
            for (int dy = 0; dy < size; dy++) {
                for (int dx = 0; dx < size; dx++) {
                    int threshold = ditherMatrix[dy][dx];
                    // 二值化: 映射值 > 阈值则为白色(1)，否则为黑色(0)
                    int binaryValue = (mappedValue > threshold) ? 1 : 0;
                    result.setPixel(blockStartX + dx, blockStartY + dy, binaryValue);
                }
            }
        }
    }

    std::cout << "Dither method completed. Matrix size = " << size
        << "x" << size << ", Output image size = "
        << width * size << "x" << height * size << std::endl;
    return result;
}

// ======================== 方法3：Ordered Dither（有序抖动法） ========================
// 使用 Bayer 矩阵作为阈值矩阵，支持任意 2 的幂次大小的矩阵
// 算法步骤：
// 1. 将图像像素值与 Bayer 矩阵对应位置的阈值比较
// 2. 像素值 > 阈值 则输出白(1)，否则输出黑(0)
Image ModeConvert::grayToBinaryOD(const Image& img, int size) {
    // 检查是否为灰度图像
    if (img.getbitcount() != 8) {
        std::cerr << "Error: Not a grayscale image" << std::endl;
        return Image();
    }

    int matrixSize = size;

    // 检查是否为 2 的幂次
    if (!isPowerOfTwo(matrixSize)) {
        // 如果不是 2 的幂次，自动向上取整到最近的 2 的幂次
        int originalSize = matrixSize;
        matrixSize = 1;
        while (matrixSize < originalSize) {
            matrixSize *= 2;
        }
        std::cout << "Warning: Matrix size " << originalSize
            << " is not a power of 2. Using size " << matrixSize
            << " instead." << std::endl;
    }

    // 矩阵大小最小为 1
    if (matrixSize < 1) matrixSize = 1;

    // 性能提示：矩阵太大时给出警告
    if (matrixSize > 64) {
        std::cout << "Warning: Matrix size " << matrixSize
            << " is large, may affect performance." << std::endl;
    }

    int width = img.getwidth();
    int height = img.getheight();

    // 生成 Bayer 阈值矩阵（支持任意 2 的幂次大小）
    vector<vector<int>> bayerMatrix = generateBayerMatrix(matrixSize);
    int maxThreshold = matrixSize * matrixSize - 1;

    // 结果图像（与原始图像相同尺寸）
    Image result(width, height, 1, Image::Binary);

    // 遍历每个像素
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gray = img.getPixel(x, y);

            // 获取 Bayer 矩阵中的阈值（根据位置取模）
            int threshold = bayerMatrix[y % matrixSize][x % matrixSize];

            // 将阈值归一化到 0-255 范围
            int normalizedThreshold = (threshold * 255) / maxThreshold;

            // 二值化：灰度值 > 阈值则为白色(1)，否则为黑色(0)
            int binaryValue = (gray > normalizedThreshold) ? 1 : 0;
            result.setPixel(x, y, binaryValue);
        }
    }

    std::cout << "Ordered Dither method completed. Bayer matrix size = "
        << matrixSize << "x" << matrixSize << std::endl;
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