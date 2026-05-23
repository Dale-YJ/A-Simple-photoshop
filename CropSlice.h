/*
	该头文件是文件裁减切割模块的头文件，
	要求：
	1.从原图中裁减一个小图
	2.将原图切割为多个小图

	具体要实现的是一个类，函数接口我已经给出，
	如有需要可以自行添加其他辅助函数，但必须完成给出的两个接口！！！
	具体函数实现，应该在对应的CropSlice.cpp中实现！！！
*/
#pragma once
#include "Image.h"

class CropSlice {

public:

	//把src图片裁剪，(x,y)是裁剪的起始位置，w,h是裁剪图片的宽和高，返回子图
	static Image crop(const Image& src, int x, int y, int w, int h);

	//把src图片分割，子图片的宽和高是参数，把得到的子图片以向量形式返回
	static vector<Image> slice(const Image& src, int blockW, int blockH);

};