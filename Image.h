/*
    该头文件定义了统一的图像数据结构
*/

#pragma once
#include<vector>
#include<string>
#include<Windows.h>
#include <graphics.h>
#include <conio.h>


using std::vector;
using std::string;


class Image {

public:
    //图像的类型
    enum Type {
        Binary,
        Gray,
        Color
    };

private:
    //图像的宽度
    int width = 0;

    //图像的高度
    int height = 0;
    
    //图像的位深
    int bitcount = 24;

    //图像的类型
    Type type = Color;

    //图像数据,以连续内存的形式存储
    vector<uint8_t> data;   //BGR 顺序存储

    //判断(x,y)是否是有效的坐标
    bool isValid(int x, int y) const{
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    //每个像素占用字节个数
    int bytesPerPixel() const {
        return (bitcount + 7) / 8;
    }

    // 计算(x,y)像素值在 data 中的起始下标
    size_t index(int x, int y) const {
        return static_cast<size_t>(y * width * bytesPerPixel() + x * bytesPerPixel());
    }


public:


    Image() = default;

    int getwidth() const{
        return width;
    }
    int getheight() const {
        return height;
    }
    int getbitcount()const {
        return bitcount;
    }
    Type getType() const {
        return type;
	}

    Image(int w, int h, int bc,Type t)
        : width(w), height(h), bitcount(bc),type(t) {
        if (width <= 0 || height <= 0)
            throw "Invalid image size.";

        if (bitcount != 1 && bitcount != 8 &&
            bitcount != 24 && bitcount != 32)
            throw "Unsupported bit count.";

        data.resize(static_cast<size_t>(width * height * bytesPerPixel()),0);

    }

    //获取像素值
    uint64_t getPixel(int x, int y) const {
        if (!isValid(x, y)) return -1;

        size_t i = index(x, y);
        int result = 0;

        switch (bitcount) {
        case 1: {
            int byteIndex = i / 8;
            int bitOffset = 7 - (i % 8);
            return (data[byteIndex] >> bitOffset) & 0x01;
        }
        case 8:
            return data[i];

        case 24:
            return (data[i + 2] << 16)|(data[i + 1] << 8)|data[i];
            //        R                    G                   B

        case 32:
            return (data[i + 3] << 24)
                | (data[i + 2] << 16)
                | (data[i + 1] << 8)
                | data[i];
        default:
            return -1;
        }
    }
    //设置像素值
    bool setPixel(int x, int y, uint64_t value) {
        if (!isValid(x, y)) return false;

        size_t i = index(x, y);

        switch (bitcount) {
        case 1: {
            int byteIndex = i / 8;
            int bitOffset = 7 - (i % 8);
            if (value)
                data[byteIndex] |= (1 << bitOffset);
            else
                data[byteIndex] &= ~(1 << bitOffset);
            break;
        }

        case 8:
            data[i] = static_cast<uint8_t>(value);
            break;

        case 24:
            data[i] = static_cast<uint8_t>(value & 0xFF);
            data[i + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
            data[i + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
            break;

        case 32:
            data[i] = static_cast<uint8_t>(value & 0xFF);
            data[i + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
            data[i + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
            data[i + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
            break;
        default:
            return false;
        }
        return true;
    }
    
    //获取一个24位像素点的红色分量
    static int getRedComponent(uint32_t value) {
        return value >> 16&0xff;
    }
    //获取一个24位像素点的绿色分量
    static int getGreenComponent(uint32_t value) {
        return value >> 8 & 0xff;
    }
    //获取一个24位像素点的蓝色分量
    static int getBlueComponent(uint32_t value) {
        return value &0xff;
    }



    // 双三次插值权重函数
    static double cubicInterpolation(double x) {
        double absX = fabs(x);
        if (absX <= 1.0) {
            return 1.5 * pow(absX, 3) - 2.5 * pow(absX, 2) + 1.0;
        }
        else if (absX < 2.0) {
            return -0.5 * pow(absX, 3) + 2.5 * pow(absX, 2) - 4.0 * absX + 2.0;
        }
        return 0.0;
    }

       
    
    static IMAGE resize(IMAGE* srcImg, int newWidth, int newHeight) {

		float xRatio = (newWidth*1.0/(srcImg->getwidth()*1.0) );
        float yRatio = (newHeight*1.0/(srcImg->getheight()*1.0));

		saveimage(L"temp.jpg", srcImg);

		IMAGE dstImg (newWidth, newHeight);
		

        loadimage(&dstImg, L"temp.jpg", newWidth, newHeight);

		return dstImg;

    }

    // 双三次插值缩放图像（高质量）
    static IMAGE* resizeImageBicubic(IMAGE* srcImg, int newWidth, int newHeight) {
        if (!srcImg || newWidth <= 0 || newHeight <= 0) return nullptr;

        IMAGE* dstImg = new IMAGE(newWidth, newHeight);
        DWORD* srcBuffer = GetImageBuffer(srcImg);
        DWORD* dstBuffer = GetImageBuffer(dstImg);

        int srcWidth = srcImg->getwidth();
        int srcHeight = srcImg->getheight();

        double xRatio = (double)(srcWidth - 1) / newWidth;
        double yRatio = (double)(srcHeight - 1) / newHeight;

        for (int y = 0; y < newHeight; y++) {
            for (int x = 0; x < newWidth; x++) {
                double srcX = x * xRatio;
                double srcY = y * yRatio;

                int xInt = (int)srcX;
                int yInt = (int)srcY;

                double r = 0.0, g = 0.0, b = 0.0;
                double weightSum = 0.0;

                // 双三次插值：考虑周围16个像素
                for (int m = -1; m <= 2; m++) {
                    for (int n = -1; n <= 2; n++) {
                        int px = min(max(xInt + m, 0), srcWidth - 1);
                        int py = min(max(yInt + n, 0), srcHeight - 1);

                        double dx = srcX - (xInt + m);
                        double dy = srcY - (yInt + n);

                        double weight = cubicInterpolation(dx) * cubicInterpolation(dy);
                        weightSum += weight;

                        DWORD pixel = srcBuffer[py * srcWidth + px];
                        r += weight * GetRValue(pixel);
                        g += weight * GetGValue(pixel);
                        b += weight * GetBValue(pixel);
                    }
                }

                // 归一化
                if (weightSum > 0) {
                    r /= weightSum;
                    g /= weightSum;
                    b /= weightSum;
                }

                dstBuffer[y * newWidth + x] = RGB((BYTE)r, (BYTE)g, (BYTE)b);
            }
        }

        return dstImg;
    }

    //计算两个图像的均方根误差
    static double rootMeanSquareError(Image* img1,Image* img2) {
        double res = 0.0;
        int w = img1->getwidth();
        int h = img1->getheight();
        int sum = 0;
        for (int j = 0; j < h; ++j) {
            for (int i = 0; i < w; ++i) {
                int value1 = img1->getPixel(i, j);
                int value2 = img2->getPixel(i, j);
                int error = value1 - value2;
                sum += (error * error);

            }
        }
        res = (sum * 1.0) / (w * h * 1.0);
        return sqrt(res);
    }


    // 将自定义 Image 转换为 EasyX 的 IMAGE
    IMAGE convertToEasyXImage() {
        IMAGE dstImg(width, height);

        // 获取dstImg缓冲区的指针
        DWORD* pMem = GetImageBuffer(&dstImg);

        // 直接对显示缓冲区赋值
        for (int i = 0; i < width * height; i++) {
            //对应该图像的x，y坐标
            int y = i / width;
            int x = i % width;

            switch (type) {
            case Binary:
            {
                bool value = getPixel(x, y);
                if (value) {
                    pMem[i] = WHITE;
                }
                else {
                    pMem[i] = BLACK;
                }

                break;
            }
                
            case Gray:
            {
                uint8_t value = getPixel(x, y);
                pMem[i] = RGB(value, value, value);
                break;

            }
               
            case Color:
            {
                uint32_t value = getPixel(x, y);
                pMem[i] = BGR(RGB(getRedComponent(value), getGreenComponent(value), getBlueComponent(value)));
                break;
            }
               
            }


        }
        return dstImg;

    }


};

