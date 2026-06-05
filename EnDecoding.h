/*
	该头文件是图像编码模块的头文件，
*/
#pragma once
#include "Image.h"

constexpr double M_PI = 3.1415926;
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
	static SpecialImage dct(const Image& img, int size, float keepRatio = 1.0f);
	
	// DCT 反变换编码
	static Image iverseDct(const SpecialImage& img, int size);


	// 辅助函数：计算一个大小为 NxM 的子块的DCT变换
	static void dctBlock(const vector<vector<double>>& block, vector<vector<double>>& dct, int N,int M) {
		
		for (int u = 0; u < N; ++u) {
			for (int v = 0; v < M; ++v) {
				double sum = 0.0;

				for (int x = 0; x < N; ++x) {
					for (int y = 0; y < M; ++y) {
						sum += block[x][y] *
							cos((2 * x + 1) * u * M_PI / (2.0 * N)) *
							cos((2 * y + 1) * v * M_PI / (2.0 * M));
					}
				}
				double cu = (u == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
				double cv = (v == 0) ? sqrt(1.0 / M) : sqrt(2.0 / M);
				dct[u][v] = sum * cu * cv;
			}
		}
	}

	// 辅助函数：计算一个大小为 NxM 的子块的DCT反变换
	static void idctBlock(const vector<vector<double>>& dct, vector<vector<double>>& block, int N, int M) {
		for (int x = 0; x < N; ++x) {
			
			for (int y = 0; y < M; ++y) {
				double sum = 0.0;

				for (int u = 0; u < N; ++u) {
					for (int v = 0; v < M; ++v) {

						double au = (u == 0) ? 1.0 : sqrt(2.0);
						double av = (v == 0) ? 1.0 : sqrt(2.0);

						sum += dct[u][v] * au * av *
							cos((2 * x + 1) * u * M_PI / (2 * N)) *
							cos((2 * y + 1) * v * M_PI / (2 * M));
					}
				}
				block[x][y] = sum / sqrt(M * N);
			}
		}



	}


	// 辅助函数：生成ZigZag扫描顺序（低频到高频）
	static vector<std::pair<int, int>> getZigZagOrder(int size) {
		vector<std::pair<int, int>> order(size * size);

		int idx = 0;
		//slash表示当前处理的斜线编号，从0到2*(size-1)
		for(int slash=0;slash<=2*(size-1);++slash) {
				if(slash%2==0) { //偶数斜线：左下->右上
					int i = min(slash, size - 1);//该斜线最左下方的元素的行坐标
					int j = slash - i;			//该斜线最左下方的元素的列坐标
					while(i>=0 && j<size) {
						order[idx++]={i,j};
						--i;
						++j;
					}
				}
				else { //奇数斜线：右上->左下
					int j=min(slash,size-1);
					int i=slash-j;
					while(j>=0 && i<size) {
						order[idx++]={i,j};
						++i;
						--j;
					}
				}
		}

		return order;
	}

	//辅助函数：把特殊的图像转换为0-255范围的正常图像，方便显示和存储

	static Image linearTransformTo0_255(SpecialImage& specialImage) {

		int w = specialImage.getWidth();
		int h = specialImage.getHeight();

		vector<vector<double>> tmp(w, vector<double>(h,0.0));

		for(int x=0;x<w;++x) {
			for(int y=0;y<h;++y) {
				tmp[x][y] = abs(specialImage.getPixel(x,y));
			}
		}

		// 查找输入数组的最小值和最大值
		double min_val = tmp[0][0];
		double max_val = tmp[0][0];
		for (const auto& col : tmp) {
			for (double val : col) {
				if (val < min_val) min_val = val;
				if (val > max_val) max_val = val;
			}
		}

		// 初始化结果数组
		Image resultImage(w,h, 8, Image::Gray);

		// 计算线性变换参数 y = a*(x - min_val)
		const double scale = 255.0 / (max_val - min_val); // 缩放因子 a

		for(int y=0;y<h;++y) {
			for(int x=0;x<w;++x) {
				double transformed = scale * (tmp[x][y] - min_val);
				resultImage.setPixel(x, y, static_cast<unsigned char>(round(transformed)));
			}
		}
		return resultImage;
	}


};
