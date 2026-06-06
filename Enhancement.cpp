#include "Enhancement.h"

//采用自适应直方图均衡化，以下列出步骤：
// 1. RGB图 → HSV图，只对V分量进行处理
// 2. 将原图分为若干方块，在方块内进行直方图均衡化，同时进行对比度限制
// 3.

//RGB[0..255] → HSV: H[0..360), S[0..1], V[0..255]
static void rgbToHsv(int r, int g, int b, double& h, double& s, double& v) {
	double rd = r / 255.0;//归一化
	double gd = g / 255.0;
	double bd = b / 255.0;

	double cmax = max(max(rd, gd), bd);
	double cmin = min(min(rd, gd), bd);
	double delta = cmax - cmin;

	v = cmax * 255.0;

	if (delta < 1e-10) {//R = B = G，灰度像素特别处理
		h = 0.0;
		s = 0.0;
		return;
	}

	s = delta / cmax;

	if (fabs(cmax - rd) < 1e-10) {//cmax = RGB，三种情况分类计算
		h = 60.0 * fmod((gd - bd) / delta + 6.0, 6.0);
	}
	else if (fabs(cmax - gd) < 1e-10) {
		h = 60.0 * ((bd - rd) / delta + 2.0);
	}
	else {
		h = 60.0 * ((rd - gd) / delta + 4.0);
	}
}

//HSV → RGB[0..255]
static void hsvToRgb(double h, double s, double v, int& r, int& g, int& b) {
	double vd = v / 255.0;
	double c = vd * s;
	double x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
	double m = vd - c;

	double rd, gd, bd;
	if (h < 60.0) { rd = c; gd = x; bd = 0.0; }//按h分类计算
	else if (h < 120.0) { rd = x; gd = c; bd = 0.0; }
	else if (h < 180.0) { rd = 0.0; gd = c; bd = x; }
	else if (h < 240.0) { rd = 0.0; gd = x; bd = c; }
	else if (h < 300.0) { rd = x; gd = 0.0; bd = c; }
	else { rd = c; gd = 0.0; bd = x; }

	r = (int)round((rd + m) * 255.0);//四舍五入，减少精度丢失
	g = (int)round((gd + m) * 255.0);
	b = (int)round((bd + m) * 255.0);

	//限制范围[0, 255]
	r = max(0, min(255, r));
	g = max(0, min(255, g));
	b = max(0, min(255, b));
}
// ============================================================
// 第 2 步: 计算单个瓦片的 CLAHE 映射表
// ============================================================
// hist:       该瓦片 256-bin 直方图
// tilePixels: 瓦片内总像素数
// clipLimit:  对比度限制参数 (e.g. 3.0)
// 返回:       256 元素查找表 map[oldVal] -> newVal
// ============================================================
static vector<int> computeTileMap(const vector<int>& hist,
	int tilePixels, double clipLimit) {
	// 计算实际裁剪阈值: clipLimit * tilePixels / 256
	double actualClip = clipLimit * tilePixels / 256.0;
	if (actualClip < 1.0) actualClip = 1.0;   // 每个 bin 至少保留 1 个像素

	// ----- 2a. 对比度裁剪 -----
	vector<int> clipped(256);
	double excess = 0.0;
	for (int i = 0; i < 256; i++) {
		if (hist[i] > actualClip) {
			excess += hist[i] - actualClip;   // 累积被裁掉的像素
			clipped[i] = (int)actualClip;
		}
		else {
			clipped[i] = hist[i];
		}
	}

	// ----- 2b. 重新分配多余像素 -----
	// 将 excess 均匀加回所有 256 bins
	double redist = excess / 256.0;
	double total = 0.0;
	for (int i = 0; i < 256; i++) {
		clipped[i] = (int)round(clipped[i] + redist);
		total += clipped[i];
	}

	// 处理空瓦片（total == 0）: 返回恒等映射
	if (total < 1.0) {
		vector<int> identity(256);
		for (int i = 0; i < 256; i++) identity[i] = i;
		return identity;
	}

	// ----- 2c. CDF -> 映射表 -----
	vector<int> map(256);
	double cdf = 0.0;
	for (int i = 0; i < 256; i++) {
		cdf += clipped[i];
		map[i] = (int)round(cdf * 255.0 / total);
		if (map[i] < 0)   map[i] = 0;
		if (map[i] > 255) map[i] = 255;
	}

	return map;
}

vector<int> Enhancement::histogram(const Image& img) {
	//todo
	vector<int> res;
	return res;
}


Image Enhancement::histogramEqualization(const Image& img) {
	//todo
    const int w = img.getwidth();
    const int h = img.getheight();
    Image res(w, h, img.getbitcount(), img.gettype());

    //二值图不处理
    if (img.gettype() == 0) return img;

    //CLAHE参数
    const int    TILE_GRID_X = 8;//水平瓦片数
    const int    TILE_GRID_Y = 8;//垂直瓦片数
    const double CLIP_LIMIT = 3.0;//对比度限制

    //计算瓦片尺寸和网格
    int tileW = max(8, w / TILE_GRID_X);
    int tileH = max(8, h / TILE_GRID_Y);
    int gridX = w / tileW;
    int gridY = h / tileH;
    // 确保至少有 2×2 个瓦片做插值
    if (gridX < 2) { gridX = 2; tileW = w / gridX; }
    if (gridY < 2) { gridY = 2; tileH = h / gridY; }

    // ===== Step A: 为每个瓦片计算映射表 =====
    //
    //   tileMaps[ty * gridX + tx] = 该瓦片的 256 元素映射表
    //
    vector<vector<int>> tileMaps(gridY * gridX);

    for (int ty = 0; ty < gridY; ty++) {
        int yStart = ty * tileH;
        int yEnd = (ty == gridY - 1) ? h : (ty + 1) * tileH;

        for (int tx = 0; tx < gridX; tx++) {
            int xStart = tx * tileW;
            int xEnd = (tx == gridX - 1) ? w : (tx + 1) * tileW;

            // -- 统计该瓦片的 V 通道直方图 --
            vector<int> hist(256, 0);
            int tilePixels = 0;

            for (int y = yStart; y < yEnd; y++) {
                for (int x = xStart; x < xEnd; x++) {
                    int p = img.getPixel(x, y);
                    int vValue;

                    if (img.gettype() == 1) {
                        // 灰度图: 像素值即亮度
                        vValue = p;
                    }
                    else {
                        // 彩色图: V = max(R, G, B)
                        int r = Image::getRedComponent(p);
                        int g = Image::getGreenComponent(p);
                        int b = Image::getBlueComponent(p);
                        vValue = max(max(r, g), b);
                    }

                    hist[vValue]++;
                    tilePixels++;
                }
            }

            // -- 计算该瓦片的 CLAHE 映射 --
            tileMaps[ty * gridX + tx] = computeTileMap(hist, tilePixels, CLIP_LIMIT);
        }
    }

    // ===== Step B: 逐像素应用 CLAHE（双线性插值）=====
    for (int y = 0; y < h; y++) {
        // 当前像素在瓦片网格中的浮点坐标（相对于瓦片中心）
        double tfy = (double)y / tileH - 0.5;

        // 确定上下瓦片索引
        int tyT = max(0, min(gridY - 1, (int)floor(tfy)));
        int tyB = max(0, min(gridY - 1, tyT + 1));
        double wyB = max(0.0, min(1.0, tfy - tyT));
        double wyT = 1.0 - wyB;

        for (int x = 0; x < w; x++) {
            double tfx = (double)x / tileW - 0.5;

            // 确定左右瓦片索引
            int txL = max(0, min(gridX - 1, (int)floor(tfx)));
            int txR = max(0, min(gridX - 1, txL + 1));
            double wxR = max(0.0, min(1.0, tfx - txL));
            double wxL = 1.0 - wxR;

            int p = img.getPixel(x, y);

            if (img.gettype() == 1) {
                // ===== 灰度图: 直接插值映射值 =====
                int vTL = tileMaps[tyT * gridX + txL][p];
                int vTR = tileMaps[tyT * gridX + txR][p];
                int vBL = tileMaps[tyB * gridX + txL][p];
                int vBR = tileMaps[tyB * gridX + txR][p];

                int newVal = (int)round(
                    wxL * wyT * vTL + wxR * wyT * vTR +
                    wxL * wyB * vBL + wxR * wyB * vBR);
                newVal = max(0, min(255, newVal));
                res.setPixel(x, y, newVal);

            }
            else {
                // ===== 彩色图: RGB->HSV, 均衡 V, HSV->RGB =====
                int r = Image::getRedComponent(p);
                int g = Image::getGreenComponent(p);
                int b = Image::getBlueComponent(p);

                double hh, ss, vv;
                rgbToHsv(r, g, b, hh, ss, vv);

                int oldV = (int)round(vv);
                oldV = max(0, min(255, oldV));

                // 从 4 个相邻瓦片插值新 V 值
                int vTL = tileMaps[tyT * gridX + txL][oldV];
                int vTR = tileMaps[tyT * gridX + txR][oldV];
                int vBL = tileMaps[tyB * gridX + txL][oldV];
                int vBR = tileMaps[tyB * gridX + txR][oldV];

                double newV = wxL * wyT * vTL + wxR * wyT * vTR +
                    wxL * wyB * vBL + wxR * wyB * vBR;
                newV = max(0.0, min(255.0, newV));

                // 保持 H, S 不变，只用新的 V 重建 RGB
                int newR, newG, newB;
                hsvToRgb(hh, ss, newV, newR, newG, newB);

                unsigned int color24 =
                    ((unsigned int)newR << 16) |
                    ((unsigned int)newG << 8) |
                    (unsigned int)newB;
                res.setPixel(x, y, color24);
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