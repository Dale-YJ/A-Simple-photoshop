/*
	该头文件是图像编码模块的头文件，
*/
#pragma once
#include "Image.h"

class EnDecoding {
public:
	//无损预测编码,order是阶次,coefficient是预测系数
	Image losslessPredictiveEnCoding(const Image& img,int order,vector<int> coefficients);
	//无损预测解码
	Image losslessPredictiveDeCoding(const Image& img, int order, vector<int> coefficients);

	//均匀量化
	Image uniformQuantization(const Image& img, int bits);
	//IGS量化
	Image IGSQuantization(const Image& img, int bits);

	//DCT变换编码
	//size是分块的大小
	Image dct(const Image& img, int size, float keepRatio = 0.5f);
	
	// DCT 反变换编码
	Image iverseDct(const Image& img, int size, float keepRatio = 0.5f);

};
