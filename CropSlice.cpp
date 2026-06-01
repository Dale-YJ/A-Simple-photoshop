#include "CropSlice.h"
#include <iostream>

Image CropSlice::crop(const Image& src, int x, int y, int w, int h) {
    // 参数有效性检查
    if (src.getwidth() == 0 || src.getheight() == 0) {
        std::cerr << "Error: Source image is empty" << std::endl;
        return Image();
    }

    // 检查裁剪区域是否在源图像范围内
    if (x < 0 || y < 0 || x + w > src.getwidth() || y + h > src.getheight()) {
        std::cerr << "Error: Crop region out of bounds" << std::endl;
        return Image();
    }

    if (w <= 0 || h <= 0) {
        std::cerr << "Error: Invalid crop dimensions" << std::endl;
        return Image();
    }

    // 创建目标图像，与源图像相同的位深
    // 根据bitcount判断图像类型（通过Image构造函数自动处理）
    Image result(w, h, src.getbitcount(),
        src.getbitcount() == 1 ? Image::Binary :
        (src.getbitcount() == 8 ? Image::Gray : Image::Color));

    // 复制裁剪区域的像素
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int srcX = x + i;
            int srcY = y + j;
            int pixelValue = src.getPixel(srcX, srcY);
            result.setPixel(i, j, pixelValue);
        }
    }

    std::cout << "Cropped image: " << w << "x" << h << std::endl;
    return result;
}

vector<Image> CropSlice::slice(const Image& src, int blockW, int blockH) {
    vector<Image> res;

    // 参数有效性检查
    if (src.getwidth() == 0 || src.getheight() == 0) {
        std::cerr << "Error: Source image is empty" << std::endl;
        return res;
    }

    if (blockW <= 0 || blockH <= 0) {
        std::cerr << "Error: Invalid block dimensions" << std::endl;
        return res;
    }

    // 计算可以分割的块数
    int width = src.getwidth();
    int height = src.getheight();

    int cols = width / blockW;
    int rows = height / blockH;

    // 处理剩余部分（不足一块的丢弃）
    int actualWidth = cols * blockW;
    int actualHeight = rows * blockH;

    if (actualWidth != width || actualHeight != height) {
        std::cout << "Warning: Image size not multiple of block size. "
            << "Only processing " << actualWidth << "x" << actualHeight
            << " region. Discarding edges." << std::endl;
    }

    // 分割图像
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int startX = col * blockW;
            int startY = row * blockH;

            // 使用crop函数裁剪出每一个子图
            Image subImage = crop(src, startX, startY, blockW, blockH);
            res.push_back(subImage);
        }
    }

    std::cout << "Sliced image into " << res.size() << " blocks ("
        << cols << " x " << rows << ")" << std::endl;
    return res;
}