#include "Enhancement.h"

const double PI = 3.14159265358979323846;
inline double deg2rad(int deg) {
	return deg * PI / 180.0;
}

vector<int> Enhancement::histogram(const Image& img) {
	//todo
	vector<int> res;
	return res;
}


Image Enhancement::histogramEqualization(const Image& img) {
	//todo

	int w = img.getwidth();
	int h = img.getheight();

	int bc = img.getbitcount();
	printf("Image object address: %p\n", &img);
	printf("w = %d (0x%X)\n", w, w);
	printf("h = %d (0x%X)\n", h, h);
	printf("bc = %d (0x%X)\n", bc, bc);
	printf("type = %d (0x%X)\n", img.gettype(), img.gettype());

	Image res(w, h, img.getbitcount(), img.gettype());
	vector<int> pixels(256, 0);  //统计所有灰度/亮度的像素个数

	if (img.gettype() == 0) return img;  //黑白图像，直接返回
	else if (img.gettype() == 1) {  //灰度图像，直接对像素进行处理
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int p = img.getPixel(i, j);
				pixels[p]++;
			}
		}
	}
	else if (img.gettype() == 2){  //真彩色图像，用RGB计算出亮度，对亮度进行处理
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int p = img.getPixel(i, j);
				int r = img.getRedComponent(p);
				int b = img.getBlueComponent(p);
				int g = img.getGreenComponent(p);
				int light = (r + b + g) / 3;  //亮度计算
				pixels[light]++;
			}
		}
	}

	int total = w * h;
	vector<int> new_pixels(256, 0);
	new_pixels[0] = round(pixels[0] / total * 255.0);
	for (int i = 1; i < 256; i++) {
		pixels[i] += pixels[i - 1];  //计算CDF，即灰度/亮度<=x的像素数
		new_pixels[i] = round(pixels[i] / total * 255.0);  //计算新灰度/亮度
	}

	if (img.gettype() == 1) {
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int old_light = img.getPixel(i, j);
				res.setPixel(i, j, new_pixels[old_light]);
			}
		}
	}
	else {
		for (int x = 0; x < h; x++) {
			for (int y = 0; y < w; y++) {
				int p = img.getPixel(x, y);
				int r = img.getRedComponent(p);
				int b = img.getBlueComponent(p);
				int g = img.getGreenComponent(p);
				double theta = cos(0.5 * (2 * r - g - b) / sqrt(pow((r - g), 2) + (r - b) * (g - b)));
				double hue = b <= g ? theta : (deg2rad(360) - theta);
				double s = 1 - (3 * min(r, g, b) / (r + b + g));
				double old_light = (r + g + b) / 3;
				int i = new_pixels[old_light];
				if (hue <= deg2rad(120)) {
					b = i * (1 - s);
					r = i * (1 + s * cos(hue) / cos(deg2rad(60) - hue));
					g = 3 * i - (r + b);
					r = max(0, min(255, (int)r));
					g = max(0, min(255, (int)g));
					b = max(0, min(255, (int)b));
					unsigned int color24 = ((unsigned int)b << 16) |((unsigned int)g << 8) |(unsigned int)r;
					res.setPixel(x, y, color24);
				}
				else if (hue <= deg2rad(240) && hue > deg2rad(120)) {
					hue = hue - deg2rad(120);
					r = i * (1 - s);
					g = i * (1 + s * cos(hue) / cos(deg2rad(60) - hue));
					b = 3 * i - (r + g);
					r = max(0, min(255, (int)r));
					g = max(0, min(255, (int)g));
					b = max(0, min(255, (int)b));
					unsigned int color24 = ((unsigned int)b << 16) | ((unsigned int)g << 8) | (unsigned int)r;
					res.setPixel(x, y, color24);
				}
				else {
					hue=hue- deg2rad(240);
					g = i * (1 - s);
					b = i * (1 + s * cos(hue) / cos(deg2rad(60) - hue));
					r = 3 * i - (g + b);
					r = max(0, min(255, (int)r));
					g = max(0, min(255, (int)g));
					b = max(0, min(255, (int)b));
					unsigned int color24 = ((unsigned int)b << 16) | ((unsigned int)g << 8) | (unsigned int)r;
					res.setPixel(x, y, color24);
				}
			}
		}
	}

	return res;
}


//对灰度图像或彩色图像（同上，一般只需对亮度分量做变换）
// 进行指数变换(允许用户设定指数值) ，
Image Enhancement::expTransform(const Image& img, double gamma) {
	//todo
	Image res;
	return res;
}

//对灰度图像或彩色图像（同上，一般只需对亮度分量做变换）进行对数变换，
Image Enhancement::logTransform(const Image& img, double c) {
	//todo
	Image res;
	return res;
}