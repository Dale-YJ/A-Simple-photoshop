#include "EnDecoding.h"
#include <cmath>


//所有的出错检验在外部function中进行，这里假设输入的参数都是合法的

//无损预测编码,coefficient是预测系数
SpecialImage EnDecoding::losslessPredictiveEnCoding(const Image& img,const vector<double>& coefficients) {

	int w = img.getwidth();
	int h = img.getheight();
	//待返回的残差图像，使用与输入图像相同的尺寸和类型
	SpecialImage errImg(w, h);

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
Image EnDecoding::losslessPredictiveDeCoding(const SpecialImage& img,const vector<double>& coefficients) {

	int w = img.getWidth();
	int h = img.getHeight();

	Image res(w, h, 8, Image::Gray);
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
//size是分块的大小,keepRatio是要保留的数据比例，比如0.5，表示要保留50%的数据

SpecialImage EnDecoding::dct(const Image& img, int size, float keepRatio) {
	
	int W = img.getwidth();
	int H = img.getheight();
	SpecialImage result(W, H);

	// 获取ZigZag顺序（低频→高频）
	vector<std::pair<int, int>> zigzag = getZigZagOrder(size);
	const int totalCoeffs = size * size;
	const int keepNum = static_cast<int>(round(totalCoeffs * keepRatio)); // 保留的系数数量

	// 遍历所有分块
	for (int by = 0; by < H; by += size) {
		for (int bx = 0; bx < W; bx += size) {
			// 提取当前块
			vector<vector<double>> block(size, vector<double>(size));//待处理的子块
			for (int i = 0; i < size; ++i) {
				for (int j = 0; j < size; ++j) {
					//i行,j列
					int x = bx + j;//原图像中的x坐标
					int y = by + i;

					if (x < W && y < H) {
						block[i][j] = static_cast<double>(img.getPixel(x, y));
					}
					else {
						//不足size的部分补零

						block[i][j] = 0.0; // 补零
					}
				}
			}

			// 执行DCT变换
			vector<vector<double>> dctCoeff(size, std::vector<double>(size));
			dctBlock(block, dctCoeff, size, size);

			// 按ZigZag顺序保留系数（前keepNum个为低频，其余置0）
			for (int k = keepNum; k < totalCoeffs; ++k) {
				const int r = zigzag[k].first;
				const int c = zigzag[k].second;
				dctCoeff[r][c] = 0.0;
			}

			for (int i = 0; i < size; ++i) {
				for (int j = 0; j < size; ++j) {
					int x = bx + j;
					int y = by + i;
					if (x < W && y < H) {
						result.setPixel(x, y, dctCoeff[i][j]);
					}
				}
			}
		}
	}
	return result;
}

Image EnDecoding::iverseDct(const SpecialImage& img, int size) {

	int W = img.getWidth();
	int H = img.getHeight();

	Image result(W, H, 8, Image::Gray);

	// 遍历所有分块
	for (int by = 0; by < H; by += size) {
		for (int bx = 0; bx < W; bx += size) {
			// 从图像中恢复DCT系数
			vector<vector<double>> dctCoeff(size, vector<double>(size));
			for (int i = 0; i < size; ++i) {
				for (int j = 0; j < size; ++j) {
					int x = bx + j;
					int y = by + i;
					if (x < W && y < H) {
						dctCoeff[i][j] = img.getPixel(x, y);
					}
					else {
						//不足size的部分补零
						dctCoeff[i][j] = 0.0; // 补零
					}

				
				}
			}
			// 执行IDCT反变换
			vector<vector<double>> block(size, vector<double>(size));
			idctBlock(dctCoeff, block, size, size);

			for (int i = 0; i < size; ++i) {
				for (int j = 0; j < size; ++j) {
					int x = bx + j;
					int y = by + i;
					if (x < result.getwidth() && y < result.getheight()) {
						double pixel = block[i][j];
						pixel = max(0.0, min(255.0, pixel));
						result.setPixel(x, y, static_cast<int>(pixel));
					}
				}
			}
		}
	}
	return result;

}



//
//SpecialImage EnDecoding::dct(const Image& img, int size, float keepRatio) {
//	int W = img.getwidth();
//	int H = img.getheight();
//
//	// 计算分块后的尺寸（向上取整，不足补零）
//	int newW = ((W + size - 1) / size) * size;
//	int newH = ((H + size - 1) / size) * size;
//	SpecialImage result(newW, newH);
//
//	// 获取ZigZag顺序（低频→高频）
//	vector<std::pair<int, int>> zigzag = getZigZagOrder(size);
//	const int totalCoeffs = size * size;
//	const int keepNum = static_cast<int>(round(totalCoeffs * keepRatio)); // 保留的系数数量
//
//	// 遍历所有分块
//	for (int by = 0; by < newH; by += size) {
//		for (int bx = 0; bx < newW; bx += size) {
//			// 提取当前块
//			vector<vector<double>> block(size,vector<double>(size));//待处理的子块
//			for (int i = 0; i < size; ++i) {
//				for (int j = 0; j < size; ++j) {
//					//i行,j列
//					int x = bx + j;//原图像中的x坐标
//					int y = by + i;
//					
//					if (x < W && y < H) {
//						block[i][j] = static_cast<double>(img.getPixel(x, y));
//					}
//					else {
//						//不足size的部分补零
//						block[i][j] = 0.0; // 补零
//					}
//				}
//			}
//
//			// 执行DCT变换
//			vector<vector<double>> dctCoeff(size, std::vector<double>(size));
//			dctBlock(block, dctCoeff, size, size);
//
//			// 按ZigZag顺序保留系数（前keepNum个为低频，其余置0）
//			for (int k = keepNum; k < totalCoeffs; ++k) {
//				const int r = zigzag[k].first;
//				const int c = zigzag[k].second;
//				dctCoeff[r][c] = 0.0;
//			}
//
//			for (int i = 0; i < size; ++i) {
//				for (int j = 0; j < size; ++j) {
//					int x = bx + j;
//					int y = by + i;
//					result.setPixel(x, y, dctCoeff[i][j]);
//				}
//			}
//		}
//	}
//	return result;
//}


// DCT 反变换编码
//Image EnDecoding::iverseDct(const SpecialImage& img, int size) {
//	
//	int W = img.getWidth();
//	int H = img.getHeight();
//	
//	Image result(W, H, 8,Image::Gray);
//
//	// 遍历所有分块
//	for (int by = 0; by < H; by += size) {
//		for (int bx = 0; bx < W; bx += size) {
//			// 从图像中恢复DCT系数
//			vector<vector<double>> dctCoeff(size, vector<double>(size));
//			for (int i = 0; i < size; ++i) {
//				for (int j = 0; j < size; ++j) {
//					int x = bx + j;
//					int y = by + i;
//					dctCoeff[i][j] = img.getPixel(x, y);
//				}
//			}
//			// 执行IDCT反变换
//			vector<vector<double>> block(size, vector<double>(size));
//			idctBlock(dctCoeff, block, size, size);
//
//			for (int i = 0; i < size; ++i) {
//				for (int j = 0; j < size; ++j) {
//					int x = bx + j;
//					int y = by + i;
//					if (x < result.getwidth() && y < result.getheight()) {
//						double pixel = block[i][j];
//						pixel = max(0.0, min(255.0, pixel));
//						result.setPixel(x, y, static_cast<int>(pixel));
//					}
//				}
//			}
//		}
//	}
//	return result;
//
//}