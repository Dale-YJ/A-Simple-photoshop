/*
	该文件是图像增强模块的头文件，
	要求：
	1.实现直方图均衡
	2.基于指数变换的增强
	3.基于对数变换的增强  

	具体要实现的是一个类，函数接口我已经给出，
	如有需要可以自行添加其他辅助函数，但必须完成给出的接口！！！
	具体函数实现，应该在对应的Enhancement.cpp中实现！！！
*/
#pragma once
#include "Image.h"

class Enhancement {

	//把一副图的直方图以向量形式返回
	vector<int>histogram(const Image& img);

	//若当前待处理图像为gray图像， 对其做直方图均衡
	//若当前待处理图像为真彩图像，通常先对其进行彩色变换（如RGB—HSI，  RGB--YCbCr），
	//并对亮度分量做直方图均衡，然后结合色度分量做彩色反变换得到均衡结果，
	Image histogramEqualization(const Image& img);

	
	//对灰度图像或彩色图像（同上，一般只需对亮度分量做变换）
	// 进行指数变换(允许用户设定指数值) ，
	Image expTransform(const Image& img, double gamma = 1.0);

	//对灰度图像或彩色图像（同上，一般只需对亮度分量做变换）进行对数变换，
	Image logTransform(const Image& img, double c = 1.0);

};

