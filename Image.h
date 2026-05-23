/*
    该头文件定义了统一的图像数据结构
*/

#pragma once
#include<vector>
#include<string>
#include<Windows.h>



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
    int getPixel(int x, int y) const {
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

};

