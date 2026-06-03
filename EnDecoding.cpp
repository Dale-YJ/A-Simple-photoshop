#include "EnDecoding.h"
#include <cmath>
//无损预测编码,order是阶次,coefficient是预测系数
//实现8-bit灰度图像的线性预测编码，允许用户选择预测器阶次并设置预测系数，
// 并将预测结果以图像的方式在窗口显示或以BMP格式存储至硬盘
//对步骤1得到预测结果进行解码（采用与步骤1中完全相同的预测器），
// 将解码结果（即恢复图像）在窗口显示或以BMP格式存储至硬盘


//所有的出错检验在外部function中进行，这里假设输入的参数都是合法的
//默认使用三阶线性预测器

Image EnDecoding::losslessPredictiveEnCoding(const Image& img,const vector<double>& coefficients) {

	int w = img.getwidth();
	int h = img.getheight();
	//待返回的残差图像，使用与输入图像相同的尺寸和类型
	Image errImg(w, h,img.getbitcount(),img.getType());

	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			int left = (i > 0) ? img.getPixel(i - 1, j) : 128; // 左边像素值
			int up = (j > 0) ? img.getPixel(i, j - 1) : 128; // 上边像素值
			int leftUp = (i > 0 && j > 0) ? img.getPixel(i - 1, j - 1) : 128; // 左上像素值
		
			double predictedValue = left * coefficients[0] + up * coefficients[1] + leftUp * coefficients[2];	
			
			double error = img.getPixel(i, j) - predictedValue; // 计算残差
			// 将残差值存储在errImg中，注意要进行适当的偏移和截断以确保像素值在合法范围内
			errImg.setPixel(i, j, static_cast<uint64_t>(std::round(error + 128))); // 假设残差值范围为[-128, 127]，偏移128后存储
		}
	}
	return errImg;
}
//无损预测解码
Image EnDecoding::losslessPredictiveDeCoding(const Image& img, int order,const vector<double>& coefficients) {
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