/*
	该头文件是图像编码模块的头文件，
*/
#pragma once
#include "Image.h"

class EnDecoding {
public:
	//无损预测编码,coefficient是预测系数
	static Image losslessPredictiveEnCoding(const Image& img,const vector<double>& coefficients);
	//无损预测解码
	static Image losslessPredictiveDeCoding(const Image& img,const vector<double>& coefficients);

	//均匀量化
	static Image uniformQuantization(const Image& img, int bitsPerPixel);
	
	//IGS量化
	static Image IGSQuantization(const Image& img);


	//DCT变换编码
	//size是分块的大小
	static Image dct(const Image& img, int size, float keepRatio = 0.5f);
	
	// DCT 反变换编码
	static Image iverseDct(const Image& img, int size, float keepRatio = 0.5f);

};
