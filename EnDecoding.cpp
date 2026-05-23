#include "EnDecoding.h"

//无损预测编码,order是阶次,coefficient是预测系数
Image EnDecoding::losslessPredictiveEnCoding(const Image& img, int order, vector<int> coefficients) {
	//todo
	Image res;
	return res;
}
//无损预测解码
Image EnDecoding::losslessPredictiveDeCoding(const Image& img, int order, vector<int> coefficients) {
	//todo
	Image res;
	return res;
}

//均匀量化
Image EnDecoding::uniformQuantization(const Image& img, int bits) {
	//todo
	Image res;
	return res;
}
//IGS量化
Image EnDecoding::IGSQuantization(const Image& img, int bits) {
	//todo
	Image res;
	return res;
}

//DCT变换编码
//size是分块的大小
Image EnDecoding::dct(const Image& img, int size, float keepRatio) {
	//todo
	Image res;
	return res;
}

// DCT 反变换编码
Image EnDecoding::iverseDct(const Image& img, int size, float keepRatio) {
	//todo
	Image res;
	return res;
}