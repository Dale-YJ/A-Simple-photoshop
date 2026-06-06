#include<iostream>
#include"Image.h"
#include"UI.h"




//
//int main() {
//    initgraph(800, 600,EX_SHOWCONSOLE);
//    Image img;
//    string path = OpenFileDialog();
//    BMPIO::read(path, img);
//    cout << img.getType();
//    IMAGE t = img.convertToEasyXImage();
//    putimage(0, 0, &t);
//    _getch();
//    closegraph();
//}

//
//int main()
//{
//	// 初始化绘图窗口
//	initgraph(640, 480);
//
//	// 定义字符串缓冲区，并接收用户输入
//	wchar_t s[10];
//	InputBox(s, 10, L"请输入半径");
//
//	// 将用户输入转换为数字
//	int r = _wtoi(s);
//
//	// 画圆
//	circle(320, 240, r);
//
//	// 按任意键退出
//	_getch();
//	closegraph();
//
//	return 0;
//}

//void testPredictiveDeCoding() {
//    initgraph(1200, 700, EX_SHOWCONSOLE);
//    Image* img = new Image();
//    string path = OpenFileDialog();
//    if (path == "") {
//        MessageBox(NULL, L"没有选择文件", L"错误", MB_OK | MB_ICONERROR);
//    }
//    BMPIO::read(path, *img);
//    vector<double>coeff;
//    coeff.push_back(0.95);
//    coeff.push_back(0);
//    coeff.push_back(0);
//    Image err = EnDecoding::losslessPredictiveEnCoding(*img, coeff);
//    Image restore =EnDecoding::losslessPredictiveDeCoding(err, coeff);
//    
//    for (int j = 0; j < img->getheight(); ++j) {
//        for (int i = 0; i < img->getwidth(); ++i) {
//            int a = img->getPixel(i, j);
//            int b= restore.getPixel(i, j);
//            if (a != b) {
//                cout << "fail" << endl;
//                return;
//            }
//
//        }
//    }
//    cout << "suc" << endl;
//   
//    _getch();
//    //closegraph();
//
//}


//void testDCT() {
//    int N = 4;
//    int M = 4;
//    vector<vector<double>> block(N, vector<double>(M, 0.0));
//    block[0][0] = 5;
//    block[0][1] = 3;
//    block[0][2] = 0;
//    block[0][3] = 2;
//
//    block[1][0] = 1;
//    block[1][1] = 7;
//    block[1][2] = 8;
//    block[1][3] = 3;
//
//    block[2][0] = 4;
//    block[2][1] = 2;
//    block[2][2] = 2;
//    block[2][3] = 2;
//
//    block[3][0] = 8;
//    block[3][1] = 5;
//    block[3][2] = 2;
//    block[3][3] = 1;
//
//
//
//    vector<vector<double>> dct(N, vector<double>(M, 0.0));
//    
//    EnDecoding::dctBlock(block, dct,N,M);
//
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < M; ++j) {
//            cout << dct[i][j] << " ";
//        }
//        cout << endl;
//    }
//    cout << endl << endl;
//
//    vector<vector<double>> newblock(N, vector<double>(M, 0.0));
//    EnDecoding::idctBlock(dct, newblock, N, M);
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < M; ++j) {
//            cout << newblock[i][j] << "    ";
//        }
//        cout << endl;
//    }
//
//
//}

void test() {
    Image* img = new Image();
    string path = OpenFileDialog();
    if (path == "") {
        MessageBox(NULL, L"没有选择文件", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    BMPIO::read(path, *img);
    SpecialImage dctImg = EnDecoding::dct(*img,8,1.0f);

    Image idctImg = EnDecoding::iverseDct(dctImg, 8);
    cout << idctImg.getheight() << endl;
    
}

int main()
{
    //testPredictiveCoding();
    //testDCT();
    //test();
    
    Widget widget(1200, 700);
    widget.init();
    widget.run();
    widget.close();
    return 0;
}

