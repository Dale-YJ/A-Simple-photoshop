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


int main()
{
    Widget widget(1200, 700);
    widget.init();
    widget.run();
    widget.close();
    return 0;
}

