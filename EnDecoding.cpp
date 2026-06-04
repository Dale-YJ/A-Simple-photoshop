#include "EnDecoding.h"
#include <cmath>

//所有的出错检验在外部function中进行，这里假设输入的参数都是合法的

//无损预测编码,coefficient是预测系数
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
			int trunkpredictedValue = static_cast<int>(predictedValue);//截断
			int error = img.getPixel(i, j) - trunkpredictedValue; // 计算残差
			errImg.setPixel(i, j, error); // 
		}
	}
	return errImg;
}
//无损预测解码
Image EnDecoding::losslessPredictiveDeCoding(const Image& img,const vector<double>& coefficients) {

	int w = img.getwidth();
	int h = img.getheight();

	Image res(w, h, img.getbitcount(), img.getType());
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {

			int left = (i > 0) ? res.getPixel(i - 1, j) : 128; // 左边像素值
			int up = (j > 0) ? res.getPixel(i, j - 1) : 128; // 上边像素值
			int leftUp = (i > 0 && j > 0) ? res.getPixel(i - 1, j - 1) : 128; // 左上像素值
			
			double predictedValue = left * coefficients[0] + up * coefficients[1] + leftUp * coefficients[2];
			int trunkpredictedValue = static_cast<int>(predictedValue);//截断

			//真实值等于预测值加上残差
			int trueValue = trunkpredictedValue + img.getPixel(i, j);
			res.setPixel(i, j, trueValue);
		}
	}

	return res;
}


//实现8 - bit灰度图像的均匀量化，
//均匀量化
Image EnDecoding::uniformQuantization(const Image& img, int bitsPerPixel) {
	
	//const int shift = 8 - bitsPerPixel;       // 右移位数（8位图像减去目标比特数）
	//const int step = 1 << shift;              // 量化步长（每个区间的宽度）
	//const int halfStep = step >> 1;           // 区间中点偏移量（步长的一半）
	
	//量化后的级数
	int level = 1 << bitsPerPixel;
	//量化间隔
	int delta = 256 / level;

	int w = img.getwidth();
	int h = img.getheight();
	Image res(w, h, img.getbitcount(), img.getType());

	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			int quantizedValue = 0;
			int originValue = img.getPixel(i, j);
			//q(s)=si+Δ/2
			quantizedValue = originValue / delta * delta + delta / 2;
			//quantizedValue = ((originValue >> shift) << shift) | halfStep;

			res.setPixel(i, j, quantizedValue);
		}
	}
	return res;
}


//IGS量化
// 实现8-bit灰度图像的压缩比为2(即bitsPerpixel为4)的均匀量化改进版IGS，
Image EnDecoding::IGSQuantization(const Image& img) {

	int w = img.getwidth();
	int h = img.getheight();
	Image res(w, h, img.getbitcount(), img.getType());

	//对水平方向的处理更好
	int sum = 0;
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			int originValue = img.getPixel(i, j);
			int quantizedValue = (originValue + sum) > 255 ? originValue : originValue + sum;
			sum = quantizedValue & 0xf;
			quantizedValue >>= 4;
			quantizedValue <<= 4;
			res.setPixel(i, j, quantizedValue);
		}
	}
	return res;
}




//DCT变换编码
//size是分块的大小
//实现8 - bit灰度图像的分块DCT变换，允许用户设置分块大小，并将变换结果以图像的方式在窗口显示或以BMP格式存储至硬盘
//对步骤1得到变换结果进行DCT反变换，将反变换结果（即恢复图像）在窗口显示或以BMP格式存储至硬盘，并与原始的图像进行对比

Image EnDecoding::dct(const Image& img, int size, float keepRatio) {
	
	Image res;
	return res;
}




// DCT 反变换编码
//将步骤1得到变换结果扔掉50 % 的数据（即将每块中50 % 的高频系数用0代替），
// 然后进行DCT反变换，
// 将反变换结果（即解压图像）在窗口显示或以BMP格式存储至硬盘，并与原始图像、步骤2的恢复图像进行对比
Image EnDecoding::iverseDct(const Image& img, int size, float keepRatio) {
	//todo
	Image res;
	return res;
}