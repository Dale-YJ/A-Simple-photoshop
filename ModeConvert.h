/*
	该头文件是模式转换模块的头文件，
	要求：
	1.gray to binary ----单阈值法/ dither / ordered dither ）

	2.color to gray

	具体要实现的是一个类，函数接口我已经给出，
	如有需要可以自行添加其他辅助函数，但必须完成给出的接口！！！
	具体函数实现，应该在对应的ModeConvert.cpp中实现！！！
*/

#pragma once
#include "Image.h"

class ModeConvert {
public:

	
	//用单阈值法，把灰度图像转换为二进制图像
	//threshold为给定的阈值
	Image grayToBinaryT(const Image& img, int threshold);
	
	//用dither把灰度图像转换为二进制图像，
	// size为dither矩阵的大小
	Image grayToBinaryD(const Image& img, int size);

	//用ordered dither把灰度图像转换为二进制图像
	Image grayToBinaryOD(const Image& img, int size);

	//若当前待处理图像为真彩图像，计算该图像对应的亮度分量，
	//转换为灰度图像
	Image colorToGray(const Image& img);
	
};