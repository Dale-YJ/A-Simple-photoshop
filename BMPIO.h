/*
	该头文件是文件处理模块的头文件，
	要求：
	1.实现bmp文件读入功能
	2.bmp文件输出功能

	具体要实现的是一个类，函数接口我已经给出，
	如有需要可以自行添加其他辅助函数，但必须完成给出的两个接口！！！
	具体函数实现，应该在对应的BMPIO.cpp中实现！！！
*/

#pragma once
#include "Image.h"

class BMPIO {
public:

	//从指定的文件路径中读入bmp图片，并把他转化为Image类型的数据结构
	//Image类型定义见Image.h
	static bool read(const std::string& path, Image& img);
	//把img的表示的图片以bmp格式保存到path路径
	static bool write(const std::string& path, const Image& img);

};