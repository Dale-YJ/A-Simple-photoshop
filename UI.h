#pragma once
#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <functional>
#include <cwchar>    

#include "Image.h"
#include "BMPIO.h"
#include "CropSlice.h"
#include "EnDecoding.h"
#include "ModeConvert.h"
#include "Enhancement.h"
using namespace std;


//打开一个文件对话框，返回选择bmp文件的路径
string OpenFileDialog() {
    char filePath[MAX_PATH] = { 0 };

    OPENFILENAMEA ofn;   // 使用 ANSI 版本，避免字符集问题
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);

    ofn.lpstrFilter = "BMP Files (*.bmp)\0*.bmp\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "选择 BMP 文件";

    if (GetOpenFileNameA(&ofn)) {
        return string(filePath);  // 返回 string 类型路径
    }
    return "";  // 用户点击了“取消”
}
//打开文件对话框，但是这个用于选择文件夹

string OpenFolderDialog() {
    char folderPath[MAX_PATH] = { 0 };
    BROWSEINFOA  bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.lpszTitle = "选择文件夹";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != nullptr) {
        SHGetPathFromIDListA(pidl, folderPath);
        CoTaskMemFree(pidl);  // 释放内存
        return string(folderPath);  // 返回 string 类型路径
    }
    return "";  // 用户点击了“取消”
}


// 定义Button类，表示一个按钮
class Button
{
protected:
    int x; // 按钮左上角x坐标
    int y; // 按钮左上角y坐标
    int width; // 按钮宽度
    int height; // 按钮高度
    float scale; // 缩放比例，用于实现鼠标悬停效果
    bool isMouseOver; // 表示鼠标是否在按钮上方
    wstring text; // 按钮文本
    function<void*(void*,int)> onClick; // 点击按钮触发的函数
    
public:

    Button(int _x, int _y, int _width, int _height, const wstring& _text, const function<void*(void*,int)>& _onClick=[](void* img,int index){return nullptr; })
		: x(_x), y(_y), width(_width), height(_height), text(_text), scale(1.0f), isMouseOver(false), onClick(_onClick)
    {}

    // 检查鼠标是否在按钮上方
    bool checkMouseOver(int mouseX, int mouseY)
    {
        isMouseOver = (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height);

        if (isMouseOver) {
            scale = 0.9f; // 鼠标悬停时缩放按钮
        }
        else {
            scale = 1.0f; // 恢复按钮原始大小
        }
        return isMouseOver;

    }


    int getX() {
        return x;
    }
    int getY() {
        return y;
    }
    int getHeight() {
        return height;
    }

    int getWidth() {
        return width;
    }

    void setOnclick(function<void* (void*,int)> _onClick) {

        onClick = _onClick;
    }

    // 检查鼠标点击是否在按钮内，并执行函数
    void* checkClick(int mouseX, int mouseY,void*img=nullptr,int index=0)
    {
        if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height)
        {
            void* res=onClick(img,index); // 执行按钮点击时的函数
            //isMouseOver = false;
            //scale = 1.0f;
            return res;
        }
        return nullptr;
    }

    // 绘制按钮
    void draw()
    {
        int scaledWidth = width * scale; // 缩放后的按钮宽度
        int scaledHeight = height * scale; // 缩放后的按钮高度
        int scaledX = x + (width - scaledWidth) / 2; // 缩放后的按钮x坐标
        int scaledY = y + (height - scaledHeight) / 2; // 缩放后的按钮y坐标

        if (isMouseOver)
        {
            setlinecolor(RGB(0, 120, 215)); // 鼠标悬停时按钮边框颜色
            setfillcolor(RGB(229, 241, 251)); // 鼠标悬停时按钮填充颜色
        }
        else
        {
            setlinecolor(RGB(173, 173, 173)); // 按钮边框颜色
            setfillcolor(RGB(225, 225, 225)); // 按钮填充颜色
        }

        fillrectangle(scaledX, scaledY, scaledX + scaledWidth, scaledY + scaledHeight); // 绘制按钮
        settextcolor(BLACK); // 设置文本颜色为黑色
        setbkmode(TRANSPARENT); // 设置文本背景透明
        settextstyle(20 * scale, 0, _T("微软雅黑")); // 设置文本大小和字体
        //居中显示按钮文本
        int textX = scaledX + (scaledWidth - textwidth(text.c_str())) / 2; // 计算文本在按钮中央的x坐标
        int textY = scaledY + (scaledHeight - textheight(_T("微软雅黑"))) / 2; // 计算文本在按钮中央的y坐标
        outtextxy(textX, textY, text.c_str()); // 在按钮上绘制文本
    }
};


//定义一个TextureButton类
class TextureButton : public Button {
private:
    IMAGE* texture; // 按钮纹理，可以是一个图片
public:
    TextureButton(int _x, int _y, int _width, int _height, IMAGE* texture, const wstring& _text=L"", const function<void* (void*,int)>& _onClick = [](void* img,int index) {return nullptr; })
		: Button(_x, _y, _width, _height, _text, _onClick), texture(texture)
	{
	}
    

    // 检查鼠标点击是否在按钮内，并执行函数
    bool checkClick(int mouseX, int mouseY, void* img = nullptr,int index=0)
    {
        if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height)
        {
            onClick(img,index); // 执行按钮点击时的函数
            //isMouseOver = false;
            //scale = 1.0f;
            return true;
        }
        return false;
    }

    // 绘制按钮
    void draw()
    {
        int scaledWidth = width * scale; // 缩放后的按钮宽度
        int scaledHeight = height * scale; // 缩放后的按钮高度
        int scaledX = x + (width - scaledWidth) / 2; // 缩放后的按钮x坐标
        int scaledY = y + (height - scaledHeight) / 2; // 缩放后的按钮y坐标

        if (isMouseOver)
        {
            setlinecolor(RGB(0, 120, 215)); // 鼠标悬停时按钮边框颜色
            setfillcolor(RGB(229, 241, 251)); // 鼠标悬停时按钮填充颜色
        }
        else
        {
            setlinecolor(RGB(173, 173, 173)); // 按钮边框颜色
            setfillcolor(RGB(225, 225, 225)); // 按钮填充颜色
        }

        fillrectangle(scaledX, scaledY, scaledX + scaledWidth, scaledY + scaledHeight); // 绘制按钮

        //方法1：使用双三次插值缩放图像，质量差
        //IMAGE* res = Image::resizeImageBicubic(texture, scaledWidth - 10, scaledHeight - 10); // 使用双三次插值缩放图像
        //putimage(scaledX + 5, scaledY + 5, res); // 在指定位置绘制图像

        //方法2：使用简单缩放方法缩放图像，质量好
        IMAGE res2 = Image::resize(texture, scaledWidth - 10, scaledHeight - 10); // 使用简单缩放方法缩放图像 
        putimage(scaledX + 5, scaledY + 5, &res2); // 在指定位置绘制图像



    }
};


//定义一个tab类(选项卡)，表示一个选项列表，可以用于显示多个选项并让用户选择其中一个
class Tab {
private:
    int count;// 选项数量
    bool isExpanded = false; // 是否展开选项列表
    vector<wstring> options; // 存储选项文本内容的向量
    int selectedOption = 0; // 当前选中的选项文本
    //选项列表关闭时显示选中的选项文本，选项列表展开时显示所有选项文本

    int x; // 左上角x坐标
    int y; // 左上角y坐标
    int width; // 每一个选项的宽度
    int height; // 每一个选项的高度

    bool isMouseOver = false; // 表示鼠标是否在按钮上方
    int hoveredOptionIndex = -1; // 当前鼠标悬停的选项索引

public:

    int getX() {
        return x;
    }

    int getY() {
        return y;
    }
    int getHeight() {
        return height;
    }
    int getWidth() {
        return width;
    }

    Tab(int choiceCount, const vector<wstring>& options, int _x, int _y, int _width, int _height)
        :count(choiceCount), options(options), x(_x), y(_y), width(_width), height(_height)
    {
    }

    bool checkMouseOver(int mouseX, int mouseY)
    {
        //如果选项卡没有展开，检查鼠标是否在选项卡上方
        if (!isExpanded) {
            isMouseOver = (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height);
        }
        //如果选项卡展开，检查鼠标在哪一个选项上方
        else {
            isMouseOver = (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height * (count + 1));
            if (isMouseOver) {
                mouseY -= y; //将鼠标y坐标转换为相对于选项列表的坐标
                int optionHeight = height; //每个选项的高度
                hoveredOptionIndex = mouseY / optionHeight; //计算鼠标悬停的选项索引
            }
            else {
                hoveredOptionIndex = -1; //没有悬停在任何选项上
            }
        }
        return isMouseOver;
    }

    //返回当前选中的选项索引
    int returnSelectedOption() {
        return selectedOption;
    }

    // 检查鼠标点击是否在标签内
    bool checkClick(int mouseX, int mouseY)
    {
        if (isMouseOver) {
            if (!isExpanded) {
                isExpanded = true; //点击选项卡时展开选项列表
            }
            else {
                //如果点击的是选项卡，则关闭选项列表但不改变选中的选项
                if (hoveredOptionIndex == 0) {
                    isExpanded = false;
                }
                else if (hoveredOptionIndex > 0 && hoveredOptionIndex <= options.size()) {
                    selectedOption = hoveredOptionIndex - 1; //根据鼠标悬停的选项索引设置选中的选项
                    isExpanded = false; //点击选项时关闭选项列表
                }

            }
            return true;
        }
        return false;
    }



    // 显示选项并等待用户选择
    void draw()
    {
        // 在这里实现显示选项的逻辑，例如绘制选项列表并处理用户输入
        // 可以使用按钮或其他交互方式来让用户选择

        //如果选项列表展开，显示所有选项文本；如果选项列表关闭，显示选中的选项文本

        //如果鼠标悬停在当前选择的选项卡上方，改变选项卡的颜色以提供视觉反馈
        if (isMouseOver && hoveredOptionIndex == 0)
        {
            setlinecolor(RGB(0, 120, 215)); // 鼠标悬停时边框颜色
            setfillcolor(RGB(229, 241, 251)); // 鼠标悬停时填充颜色
        }
        else
        {
            setlinecolor(RGB(173, 173, 173)); // 按钮边框颜色
            setfillcolor(RGB(225, 225, 225)); // 按钮填充颜色
        }

        fillrectangle(x, y, x + width, y + height); // 绘制选择的选项

        settextcolor(BLACK); // 设置文本颜色为黑色
        setbkmode(TRANSPARENT); // 设置文本背景透明
        settextstyle(20, 0, _T("微软雅黑")); // 设置文本大小和字体
        //居中显示按钮文本
        int textX = x + (width - textwidth(options[selectedOption].c_str())) / 2; // 计算文本在按钮中央的x坐标
        int textY = y + (height - textheight(_T("微软雅黑"))) / 2; // 计算文本在按钮中央的y坐标
        outtextxy(textX, textY, options[selectedOption].c_str()); // 在按钮上绘制文本

        //如果选项列表展开，显示所有选项文本
        if (isExpanded) {
            for (int i = 0; i < options.size(); i++) {
                if (isMouseOver && hoveredOptionIndex - 1 == i)
                {
                    setlinecolor(RGB(0, 120, 215)); // 鼠标悬停时边框颜色
                    setfillcolor(RGB(229, 241, 251)); // 鼠标悬停时填充颜色
                }
                else
                {
                    setlinecolor(RGB(173, 173, 173)); // 按钮边框颜色
                    setfillcolor(RGB(225, 225, 225)); // 按钮填充颜色
                }
                int optionX = x; //每个选项的x坐标
                int optionY = y + height * (i + 1); //每个选项的y坐标

                fillrectangle(optionX, optionY, optionX + width, optionY + height); // 绘制选项

                settextcolor(BLACK); // 设置文本颜色为黑色
                setbkmode(TRANSPARENT); // 设置文本背景透明
                settextstyle(20, 0, _T("微软雅黑")); // 设置文本大小和字体
                //居中显示按钮文本
                int textX = optionX + (width - textwidth(options[i].c_str())) / 2; // 计算文本在按钮中央的x坐标
                int textY = optionY + (height - textheight(_T("微软雅黑"))) / 2; // 计算文本在按钮中央的y坐标
                outtextxy(textX, textY, options[i].c_str()); // 在按钮上绘制文本
            }


        }







    }


};




//按钮对应的一堆函数
class Functions {
public:
    Functions() {

        onClick.push_back(func1);
        onClick.push_back(func2);
        onClick.push_back(func3);
        onClick.push_back(func4);
        onClick.push_back(func5);
        onClick.push_back(func6);
        onClick.push_back(func7);
        onClick.push_back(func8);
        onClick.push_back(func9);
        onClick.push_back(func10);
        onClick.push_back(func11);
        onClick.push_back(func12);
        onClick.push_back(func13);
        onClick.push_back(func14);
        onClick.push_back(func15);
        onClick.push_back(func16);

	}
    vector<wstring>name = {L"bmp文件读入",L"bmp文件输出",L"裁减", L"切割", L"gray to binary",
        L"color to gray", L"直方图均衡", L"指数变换增强", L"对数变换增强", L"无损预测编码", L"无损预测解码",
        L"均匀量化",L"IGS", L"DCT变换编码",L"反DCT变换",L"均方根误差" };
    
    vector<function<void*(void*,int)>> onClick; // 点击按钮触发的函数族
    // 辅助函数：OTSU自动阈值计算
    static int calculateOtsuThreshold(const Image& img);
	//bmp文件读入,index是当前图像在图像列表中的索引，
    // 函数可以根据这个索引对图像列表进行修改
    static void* func1(void*image=nullptr,int index=-1) {

        Image* img = new Image();
        string path = OpenFileDialog();
        if (path == "") {
            MessageBox(NULL, L"没有选择文件", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        BMPIO::read(path, *img);
        return img;

    }

    //bmp文件输出
    static void* func2(void*img, int index = 0) {

        vector<Image*>* images = static_cast<vector<Image*>*>(img);

        cout << "images size: " << images->size() << endl;
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以保存", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        // 打开文件夹选择对话框，获取用户选择的文件夹路径
        string folderPath = OpenFolderDialog();

        if (folderPath == "") {
            MessageBox(NULL, L"没有选择文件夹", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        bool suc = true;
        for (int i = 0; i < images->size(); i++) {
            string filepath = folderPath;
            filepath += "\\output" + to_string(i) + ".bmp";
            suc &= BMPIO::write(filepath, *(*images)[i]);

        }

        // 显示一个消息框
        if (suc) {
            MessageBox(NULL, L"保存成功", L"提示", MB_OK | MB_ICONINFORMATION);
        }
        else {
            MessageBox(NULL, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
        }

        return nullptr;
    }
    
    // ======================== func3：图像裁剪 ========================
 // 功能：从原图中裁减一个小图，允许用户设置裁剪参数（左上角坐标、宽度、高度）
    static void* func3(void* image = nullptr, int index = 0) {

        vector<Image*>* images = static_cast<vector<Image*>*>(image);

        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* srcImg = (*images)[index];
        int srcWidth = srcImg->getwidth();
        int srcHeight = srcImg->getheight();

        // 显示当前图像尺寸
        wchar_t infoBuf[256];
        swprintf_s(infoBuf, 256, L"当前图像尺寸：%d x %d\n请设置裁剪参数", srcWidth, srcHeight);
        MessageBox(NULL, infoBuf, L"图像信息", MB_OK | MB_ICONINFORMATION);

        // 输入裁剪参数
        wchar_t params[100];
        wchar_t prompt[256];
        swprintf_s(prompt, 256, L"请输入裁剪参数\n格式：x,y,width,height\n图像尺寸：%d x %d\n示例：0,0,100,100",
            srcWidth, srcHeight);

        InputBox(params, 100, prompt);

        // 解析参数
        int x = 0, y = 0, w = 0, h = 0;
        wchar_t* context = nullptr;
        wchar_t* token = wcstok_s(params, L",", &context);

        if (token != nullptr) {
            x = _wtoi(token);
            token = wcstok_s(nullptr, L",", &context);
            if (token != nullptr) {
                y = _wtoi(token);
                token = wcstok_s(nullptr, L",", &context);
                if (token != nullptr) {
                    w = _wtoi(token);
                    token = wcstok_s(nullptr, L",", &context);
                    if (token != nullptr) {
                        h = _wtoi(token);
                    }
                }
            }
        }

        // 参数有效性检查
        if (w <= 0 || h <= 0) {
            MessageBox(NULL, L"错误：裁剪宽度和高度必须大于0", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (x < 0 || y < 0) {
            MessageBox(NULL, L"错误：裁剪起始坐标不能为负数", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (x + w > srcWidth || y + h > srcHeight) {
            wchar_t errorBuf[512];
            swprintf_s(errorBuf, 512, L"错误：裁剪区域超出图像范围！\n图像范围：[0, %d] x [0, %d]\n裁剪区域：[%d, %d] x [%d, %d]",
                srcWidth, srcHeight, x, x + w, y, y + h);
            MessageBox(NULL, errorBuf, L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        // 执行裁剪
        Image* result = new Image(CropSlice::crop(*srcImg, x, y, w, h));

        // 显示结果并询问是否保存
        wchar_t resultBuf[256];
        swprintf_s(resultBuf, 256, L"裁剪成功！\n裁剪区域：(%d, %d) 尺寸：%d x %d\n是否保存到硬盘？",
            x, y, w, h);

        int saveChoice = MessageBox(NULL, resultBuf, L"提示", MB_YESNO | MB_ICONQUESTION);

        if (saveChoice == IDYES) {
            string folderPath = OpenFolderDialog();
            if (folderPath != "") {
                string filepath = folderPath + "\\crop_result.bmp";
                if (BMPIO::write(filepath, *result)) {
                    MessageBox(NULL, L"裁剪图像已保存到硬盘", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    MessageBox(NULL, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
                }
            }
        }

        return result;
    }

    // ======================== func4：图像切割 ========================
  // 功能：将原图切割为多个小图，允许用户设置小图尺寸（宽度和高度）
    static void* func4(void* image = nullptr, int index = 0) {

        vector<Image*>* images = static_cast<vector<Image*>*>(image);

        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* srcImg = (*images)[index];
        int srcWidth = srcImg->getwidth();
        int srcHeight = srcImg->getheight();

        // 显示当前图像尺寸
        wchar_t infoBuf[256];
        swprintf_s(infoBuf, 256, L"当前图像尺寸：%d x %d\n请输入切割块尺寸", srcWidth, srcHeight);
        MessageBox(NULL, infoBuf, L"图像信息", MB_OK | MB_ICONINFORMATION);

        // 输入切割参数（块宽度和块高度）
        wchar_t params[100];
        wchar_t prompt[256];
        swprintf_s(prompt, 256, L"请输入切割块尺寸\n格式：blockWidth,blockHeight\n图像尺寸：%d x %d\n示例：100,100",
            srcWidth, srcHeight);

        InputBox(params, 100, prompt);

        // 解析参数
        int blockW = 0, blockH = 0;
        wchar_t* context = nullptr;
        wchar_t* token = wcstok_s(params, L",", &context);

        if (token != nullptr) {
            blockW = _wtoi(token);
            token = wcstok_s(nullptr, L",", &context);
            if (token != nullptr) {
                blockH = _wtoi(token);
            }
        }

        // 参数有效性检查
        if (blockW <= 0 || blockH <= 0) {
            MessageBox(NULL, L"错误：切割块宽度和高度必须大于0", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (blockW > srcWidth || blockH > srcHeight) {
            MessageBox(NULL, L"错误：切割块尺寸不能大于原图像尺寸", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        // 计算可以切割的块数
        int cols = srcWidth / blockW;   // 水平方向块数
        int rows = srcHeight / blockH;  // 垂直方向块数

        if (cols == 0 || rows == 0) {
            MessageBox(NULL, L"错误：切割块尺寸太大，无法切割出完整块", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        int totalBlocks = cols * rows;
        int actualWidth = cols * blockW;
        int actualHeight = rows * blockH;

        // 显示切割信息并确认
        wchar_t confirmBuf[512];
        swprintf_s(confirmBuf, 512,
            L"切割信息：\n"
            L"原图尺寸：%d x %d\n"
            L"块尺寸：%d x %d\n"
            L"切割数量：%d 行 x %d 列 = %d 块\n"
            L"实际切割区域：%d x %d\n"
            L"（边缘不足一块的部分将被丢弃）\n\n"
            L"是否继续？",
            srcWidth, srcHeight, blockW, blockH, rows, cols, totalBlocks, actualWidth, actualHeight);

        int confirm = MessageBox(NULL, confirmBuf, L"确认切割", MB_YESNO | MB_ICONQUESTION);
        if (confirm != IDYES) {
            return nullptr;
        }

        // 执行切割
        vector<Image> subImages = CropSlice::slice(*srcImg, blockW, blockH);

        if (subImages.empty()) {
            MessageBox(NULL, L"切割失败，未生成任何子图", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        // 将切割得到的子图添加到新的图像列表中
        vector<Image*>* resultImages = new vector<Image*>();
        for (auto& subImg : subImages) {
            resultImages->push_back(new Image(subImg));
        }

        // 询问是否保存到硬盘
        wchar_t saveBuf[256];
        swprintf_s(saveBuf, 256, L"切割成功！共获得 %d 张子图。\n是否将所有子图保存到硬盘？", (int)subImages.size());
        int saveChoice = MessageBox(NULL, saveBuf, L"提示", MB_YESNO | MB_ICONQUESTION);

        if (saveChoice == IDYES) {
            string folderPath = OpenFolderDialog();
            if (folderPath != "") {
                int successCount = 0;
                for (size_t i = 0; i < subImages.size(); i++) {
                    string filepath = folderPath + "\\subimage_" + to_string(i) + ".bmp";
                    if (BMPIO::write(filepath, subImages[i])) {
                        successCount++;
                    }
                }

                wchar_t resultBuf[256];
                swprintf_s(resultBuf, 256, L"已保存 %d / %d 张子图到硬盘", successCount, (int)subImages.size());
                MessageBox(NULL, resultBuf, L"提示", MB_OK | MB_ICONINFORMATION);
            }
        }

        // 将切割结果作为新的图像列表返回，方便用户逐个查看
        return resultImages;
    }
    // ======================== func5：灰度转二值（三种方法） ========================
  // 功能：将灰度图像转换为黑白图像
  // 方法1：单阈值法 - 允许用户设置阈值
  // 方法2：Floyd-Steinberg误差扩散抖动法（dither）
  // 方法3：有序抖动法(Ordered Dither) - 允许用户设置任意矩阵大小
    static void* func5(void* image = nullptr, int index = 0) {

        vector<Image*>* images = static_cast<vector<Image*>*>(image);

        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* srcImg = (*images)[index];

        // 检查是否为灰度图像
        if (srcImg->getType() != Image::Gray && srcImg->getbitcount() != 8) {
            MessageBox(NULL,
                L"错误：当前图像不是灰度图像！\n"
                L"gray to binary 只能处理8-bit灰度图像。\n"
                L"请先使用\"color to gray\"功能将图像转换为灰度图像。",
                L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        // 选择二值化方法
        int methodChoice = MessageBox(NULL,
            L"╔══════════════════════════════════════════════════════╗\n"
            L"║          灰度转二值图像 - 请选择方法                ║\n"
            L"╠══════════════════════════════════════════════════════╣\n"
            L"║  是   - 方法1：单阈值法                              ║\n"
            L"║  否   - 方法2：Dither抖动法                          ║\n"
            L"║  取消 - 方法3：有序抖动法(Ordered Dither)           ║\n"
            L"╚══════════════════════════════════════════════════════╝",
            L"选择二值化方法", MB_YESNOCANCEL | MB_ICONQUESTION);

        ModeConvert converter;
        Image* result = nullptr;

        // ==================== 方法1：单阈值法 ====================
        if (methodChoice == IDYES) {

            // 输入阈值
            wchar_t s[20];
            InputBox(s, 20, L"请输入阈值（0-255）\n\n像素值 > 阈值 设为白色(1)\n像素值 <= 阈值 设为黑色(0)", L"128");
            int threshold = _wtoi(s);

            // 阈值范围检查
            if (threshold < 0) threshold = 0;
            if (threshold > 255) threshold = 255;

            // 执行单阈值法二值化
            result = new Image(converter.grayToBinaryT(*srcImg, threshold));

            // 显示结果
            wchar_t msgBuf[256];
            swprintf_s(msgBuf, 256, L"单阈值法完成！\n阈值：%d\n\n图像已转换为黑白图像", threshold);
            MessageBox(NULL, msgBuf, L"处理完成", MB_OK | MB_ICONINFORMATION);
        }
        // ==================== 方法2：Dither抖动法 ====================
        else if (methodChoice == IDNO) {

            // 显示算法说明
            MessageBox(NULL,
                L"Dither抖动法（均匀阈值抖动）\n\n"
                L"算法原理：\n"
                L"1. 将像素值线性映射到 0 ~ n² 范围\n"
                L"2. 将每个像素扩展为 n×n 的子块\n"
                L"3. 用 n×n 均匀阈值矩阵进行二值化\n\n"
                L"特点：\n"
                L"矩阵越大，输出图像越大（放大n倍）\n"
                L"能够模拟出灰度层次感\n\n"
                L"点击确定继续...",
                L"算法说明", MB_OK);

            // 输入矩阵大小
            wchar_t sizeStr[20];
            InputBox(sizeStr, 20, L"请输入Dither矩阵大小 n（正整数）\n\n"
                L"矩阵为 n×n 均匀矩阵（值为 0 到 n²-1）\n"
                L"输出图像尺寸将放大为原图的 n 倍\n"
                L"示例：2、3、4、5...", L"4");

            int matrixSize = _wtoi(sizeStr);

            // 参数有效性检查
            if (matrixSize <= 0) {
                MessageBox(NULL, L"无效的矩阵大小，将使用默认值 4", L"提示", MB_OK);
                matrixSize = 4;
            }

            // 限制最大矩阵大小，防止内存溢出
            if (matrixSize > 32) {
                wchar_t confirmMsg[256];
                swprintf_s(confirmMsg, 256,
                    L"矩阵大小较大，输出图像尺寸将放大 %d 倍，可能消耗较多内存。\n是否继续？",
                    matrixSize);
                int confirm = MessageBox(NULL, confirmMsg, L"确认", MB_YESNO | MB_ICONQUESTION);
                if (confirm != IDYES) {
                    return nullptr;
                }
            }

            // 执行Dither抖动法
            result = new Image(converter.grayToBinaryD(*srcImg, matrixSize));

            wchar_t msgBuf[512];
            swprintf_s(msgBuf, 512, L"Dither抖动法完成！\n\n"
                L"矩阵大小：%d x %d\n"
                L"输出图像尺寸：%d x %d\n\n"
                L"输出图像已被放大，请使用滚动条或窗口调整查看完整图像。",
                matrixSize, matrixSize,
                result->getwidth(), result->getheight());
            MessageBox(NULL, msgBuf, L"处理完成", MB_OK | MB_ICONINFORMATION);
        }
        // ==================== 方法3：有序抖动法 (Ordered Dither) ====================
        else if (methodChoice == IDCANCEL) {

            // 显示算法说明
            MessageBox(NULL,
                L"有序抖动法(Ordered Dither) - Bayer矩阵\n\n"
                L"算法原理：\n"
                L"使用Bayer矩阵作为阈值矩阵，对每个像素进行比较：\n"
                L"if(像素值 > 归一化阈值) 设为白色 else 设为黑色\n\n"
                L"Bayer矩阵特点：\n"
                L"矩阵大小必须是2的幂次（1,2,4,8,16,32...）\n"
                L"矩阵越大，图像层次感越好\n"
                L"输出图像尺寸与原图相同\n\n"
                L"点击确定继续...",
                L"算法说明", MB_OK);

            // 输入矩阵大小
            wchar_t sizeStr[20];
            InputBox(sizeStr, 20, L"请输入Bayer矩阵大小 n（正整数）\n\n"
                L"矩阵为 n×n Bayer矩阵\n"
                L"推荐大小：2、4、8、16、32\n"
                L"如果不是2的幂次，将自动向上取整\n"
                L"示例：2、4、8、16", L"8");

            int matrixSize = _wtoi(sizeStr);

            // 参数有效性检查
            if (matrixSize <= 0) {
                MessageBox(NULL, L"无效的矩阵大小，将使用默认值 8", L"提示", MB_OK);
                matrixSize = 8;
            }

            // 检查是否为2的幂次
            bool isPowerOfTwo = (matrixSize > 0 && (matrixSize & (matrixSize - 1)) == 0);
            int originalSize = matrixSize;

            if (!isPowerOfTwo) {
                int actualSize = 1;
                while (actualSize < originalSize) {
                    actualSize *= 2;
                }

                wchar_t warnBuf[512];
                swprintf_s(warnBuf, 512,
                    L"注意：%d 不是2的幂次，将自动使用 %d x %d Bayer矩阵。\n\n是否继续？",
                    originalSize, actualSize, actualSize);
                int confirm = MessageBox(NULL, warnBuf, L"提示", MB_YESNO | MB_ICONQUESTION);
                if (confirm != IDYES) {
                    return nullptr;
                }
            }

            // 限制最大矩阵大小，防止性能问题
            if (matrixSize > 64) {
                wchar_t confirmMsg[256];
                swprintf_s(confirmMsg, 256,
                    L"矩阵大小较大（%d x %d），可能影响性能。\n是否继续？",
                    matrixSize, matrixSize);
                int confirm = MessageBox(NULL, confirmMsg, L"确认", MB_YESNO | MB_ICONQUESTION);
                if (confirm != IDYES) {
                    return nullptr;
                }
            }

            // 执行有序抖动法
            result = new Image(converter.grayToBinaryOD(*srcImg, matrixSize));

            // 计算实际使用的矩阵大小（用于显示）
            int usedSize = matrixSize;
            if (!isPowerOfTwo) {
                usedSize = 1;
                while (usedSize < matrixSize) {
                    usedSize *= 2;
                }
            }

            wchar_t msgBuf[512];
            swprintf_s(msgBuf, 512, L"有序抖动法完成！\n\n"
                L"用户输入矩阵大小：%d x %d\n"
                L"实际使用Bayer矩阵大小：%d x %d\n"
                L"输出图像尺寸：%d x %d\n\n"
                L"矩阵越大，图像层次感越好。",
                originalSize, originalSize, usedSize, usedSize,
                result->getwidth(), result->getheight());
            MessageBox(NULL, msgBuf, L"处理完成", MB_OK | MB_ICONINFORMATION);
        }
        else {
            return nullptr;  // 用户取消
        }

        // 询问是否保存结果到硬盘
        if (result != nullptr) {
            int saveChoice = MessageBox(NULL,
                L"处理完成！\n是否将二值图像保存到硬盘？",
                L"提示", MB_YESNO | MB_ICONQUESTION);

            if (saveChoice == IDYES) {
                string folderPath = OpenFolderDialog();
                if (folderPath != "") {
                    // 根据方法生成不同的文件名
                    string filename;
                    if (methodChoice == IDYES) {
                        filename = "binary_threshold.bmp";
                    }
                    else if (methodChoice == IDNO) {
                        filename = "binary_dither.bmp";
                    }
                    else {
                        filename = "binary_ordered_dither.bmp";
                    }
                    string filepath = folderPath + "\\" + filename;
                    if (BMPIO::write(filepath, *result)) {
                        MessageBox(NULL, L"二值图像已保存到硬盘", L"提示", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(NULL, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
                    }
                }
            }
        }

        return result;
    }
    
    // ======================== func6：彩色转灰度 ========================
    // 功能：将真彩图像（24-bit或32-bit）转换为灰度图像（8-bit）
    // 算法：使用亮度公式 Y = 0.299R + 0.587G + 0.114B
    static void* func6(void* image = nullptr, int index = 0) {

        vector<Image*>* images = static_cast<vector<Image*>*>(image);

        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* srcImg = (*images)[index];
        int bitCount = srcImg->getbitcount();

        // 检查是否为彩色图像（24-bit或32-bit）
        if (bitCount != 24 && bitCount != 32) {
            // 如果是灰度图像，提示无需转换
            if (bitCount == 8) {
                MessageBox(NULL,
                    L"当前图像已经是灰度图像（8-bit），无需转换。\n\n"
                    L"如需再次转换，请选择彩色图像。",
                    L"提示", MB_OK | MB_ICONINFORMATION);
                return nullptr;
            }
            // 如果是二值图像
            else if (bitCount == 1) {
                MessageBox(NULL,
                    L"当前图像是二值图像（1-bit），无法转换为灰度图像。\n\n"
                    L"请选择24-bit或32-bit彩色图像。",
                    L"错误", MB_OK | MB_ICONERROR);
                return nullptr;
            }
            else {
                MessageBox(NULL,
                    L"错误：当前图像不是彩色图像！\n\n"
                    L"color to gray 只能处理24-bit或32-bit真彩图像。",
                    L"错误", MB_OK | MB_ICONERROR);
                return nullptr;
            }
        }

        // 显示算法说明
        MessageBox(NULL,
            L"彩色转灰度算法说明\n\n"
            L"转换公式（亮度公式）：\n"
            L"Y = 0.299 * R + 0.587 * G + 0.114 * B\n\n"
            L"其中：\n"
            L"Y 为输出灰度值（0-255）\n"
            L"R、G、B 为彩色图像的三个颜色分量\n\n"
            L"点击确定开始处理...",
            L"算法说明", MB_OK);

        // 执行彩色转灰度
        ModeConvert converter;
        Image* result = new Image(converter.colorToGray(*srcImg));

        // 显示结果
        wchar_t msgBuf[256];
        swprintf_s(msgBuf, 256,
            L"彩色转灰度完成！\n\n"
            L"原图像：%d-bit 彩色图像\n"
            L"结果：8-bit 灰度图像\n"
            L"尺寸：%d x %d",
            bitCount, result->getwidth(), result->getheight());
        MessageBox(NULL, msgBuf, L"处理完成", MB_OK | MB_ICONINFORMATION);

        // 询问是否保存到硬盘
        int saveChoice = MessageBox(NULL,
            L"是否将灰度图像保存到硬盘？",
            L"提示", MB_YESNO | MB_ICONQUESTION);

        if (saveChoice == IDYES) {
            string folderPath = OpenFolderDialog();
            if (folderPath != "") {
                string filepath = folderPath + "\\gray_image.bmp";
                if (BMPIO::write(filepath, *result)) {
                    MessageBox(NULL, L"灰度图像已保存到硬盘", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    MessageBox(NULL, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
                }
            }
        }

        return result;
    }
    static void* func7(void* image = nullptr, int index = 0) {
        // 获取图像列表
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* srcImg = (*images)[index];

        // 显示算法说明
        MessageBox(NULL,
            L"自适应直方图均衡 (CLAHE)\n\n"
            L"算法原理：\n"
            L"  1. 将图像分为 8×8 子块\n"
            L"  2. 每块内做直方图均衡 + 对比度限制\n"
            L"  3. 双线性插值消除块边界\n\n"
            L"对灰度图直接处理\n"
            L"对彩色图：转HSV，处理V通道后转回RGB\n\n"
            L"点击确定开始处理...",
            L"算法说明", MB_OK);

        // 执行直方图均衡
        Enhancement enhancer;
        Image* result = new Image(enhancer.histogramEqualization(*srcImg));

        // 显示完成提示
        wchar_t msgBuf[256];
        swprintf_s(msgBuf, 256,
            L"直方图均衡完成！\n\n"
            L"原图: %d x %d, %d-bit\n"
            L"结果: %d x %d, %d-bit",
            srcImg->getwidth(), srcImg->getheight(), srcImg->getbitcount(),
            result->getwidth(), result->getheight(), result->getbitcount());
        MessageBox(NULL, msgBuf, L"处理完成", MB_OK | MB_ICONINFORMATION);

        // 询问是否保存到硬盘
        int saveChoice = MessageBox(NULL,
            L"是否将结果图像保存到硬盘？",
            L"提示", MB_YESNO | MB_ICONQUESTION);

        if (saveChoice == IDYES) {
            string folderPath = OpenFolderDialog();
            if (folderPath != "") {
                string filepath = folderPath + "\\histogram_equalized.bmp";
                if (BMPIO::write(filepath, *result)) {
                    MessageBox(NULL, L"直方图均衡图像已保存到硬盘", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    MessageBox(NULL, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
                }
            }
        }

        return result;
    }
    static void* func8(void* image = nullptr, int index = 0) {
        // 获取图像列表
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* srcImg = (*images)[index];

        // 输入 gamma 值
        wchar_t s[50];
        InputBox(s, 50,
            L"请输入指数 (gamma) 值：\n\n"
            L"公式: output = 255 * (input/255)^gamma\n\n"
            L"gamma > 1  → 压暗图像，增强亮部对比\n"
            L"gamma < 1  → 提亮图像，增强暗部细节\n");

        double gamma = _wtof(s);
        if (gamma <= 0.0) {
            MessageBox(NULL, L"gamma 必须大于 0", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        // 显示算法说明
        MessageBox(NULL,
            L"指数(幂律)变换增强\n\n"
            L"公式: s = 255 * (r/255)^γ\n\n"
            L"r = 输入像素值 [0, 255]\n"
            L"s = 输出像素值 [0, 255]\n"
            L"γ = 指数参数\n\n"
            L"点击确定开始处理...",
            L"算法说明", MB_OK);

        // 执行指数变换
        Enhancement enhancer;
        Image* result = new Image(enhancer.expTransform(*srcImg, gamma));

        // 显示完成提示
        wchar_t msgBuf[256];
        swprintf_s(msgBuf, 256,
            L"指数变换完成！\n"
            L"gamma = %.2f\n\n"
            L"原图: %d x %d, %d-bit\n"
            L"结果: %d x %d, %d-bit",
            gamma,
            srcImg->getwidth(), srcImg->getheight(), srcImg->getbitcount(),
            result->getwidth(), result->getheight(), result->getbitcount());
        MessageBox(NULL, msgBuf, L"处理完成", MB_OK | MB_ICONINFORMATION);

        // 询问是否保存到硬盘
        int saveChoice = MessageBox(NULL,
            L"是否将结果图像保存到硬盘？",
            L"提示", MB_YESNO | MB_ICONQUESTION);

        if (saveChoice == IDYES) {
            string folderPath = OpenFolderDialog();
            if (folderPath != "") {
                string filepath = folderPath + "\\exp_transformed.bmp";
                if (BMPIO::write(filepath, *result)) {
                    MessageBox(NULL, L"指数变换图像已保存到硬盘", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    MessageBox(NULL, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
                }
            }
        }

        return result;
    }
    static void* func9(void* image = nullptr, int index = 0) {
        // 获取图像列表
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* srcImg = (*images)[index];

        // 输入常数 c
        wchar_t s[50];
        InputBox(s, 50,
            L"请输入常数 c 值：\n\n"
            L"公式: output = c * log(1 + input) / log(256) * 255\n"
            L"c ＞ 1 → 整体增亮，拉伸暗区细节，压缩亮区动态范围\n"
            L"适合处理整体偏暗的图像\n\n");

        double c = _wtof(s);
        if (c <= 0.0) {
            MessageBox(NULL, L"c 必须大于 0", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        // 显示算法说明
        MessageBox(NULL,
            L"对数变换增强\n\n"
            L"公式: s = c * log(1 + r) / log(256) * 255\n\n"
            L"r = 输入像素值 [0, 255]\n"
            L"s = 输出像素值 [0, 255]\n"
            L"c = 缩放常数\n\n"
            L"点击确定开始处理...",
            L"算法说明", MB_OK);

        // 执行对数变换
        Enhancement enhancer;
        Image* result = new Image(enhancer.logTransform(*srcImg, c));

        // 显示完成提示
        wchar_t msgBuf[256];
        swprintf_s(msgBuf, 256,
            L"对数变换完成！\n"
            L"c = %.2f\n\n"
            L"原图: %d x %d, %d-bit\n"
            L"结果: %d x %d, %d-bit",
            c,
            srcImg->getwidth(), srcImg->getheight(), srcImg->getbitcount(),
            result->getwidth(), result->getheight(), result->getbitcount());
        MessageBox(NULL, msgBuf, L"处理完成", MB_OK | MB_ICONINFORMATION);

        // 询问是否保存到硬盘
        int saveChoice = MessageBox(NULL,
            L"是否将结果图像保存到硬盘？",
            L"提示", MB_YESNO | MB_ICONQUESTION);

        if (saveChoice == IDYES) {
            string folderPath = OpenFolderDialog();
            if (folderPath != "") {
                string filepath = folderPath + "\\log_transformed.bmp";
                if (BMPIO::write(filepath, *result)) {
                    MessageBox(NULL, L"对数变换图像已保存到硬盘", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    MessageBox(NULL, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
                }
            }
        }

        return result;
    }

	//无损预测编码
    static void* func10(void* image = nullptr, int index = 0) {
        //整个图像序列
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        
        if(index<0 || index>=images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
		}

        //待处理的图像
        Image* img = (*images)[index];

        if (img->getType() != Image::Gray) {
            MessageBox(NULL, L"图像格式无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        wchar_t sParams[50];
        InputBox(sParams, 50, L"请输入预测阶数和系数\n（格式：阶数,系数1,系数2,系数3...）\n例如：2,0.5,0.5\n只支持1-3阶");
		vector<double> coefficients(3,0.0); //预测系数

        wchar_t* context = nullptr;
        wchar_t* token = wcstok_s(sParams, L",", &context); // 
        if (token != nullptr) {
            wchar_t* endptr;
            long order = wcstol(token, &endptr, 10);
            // 检查阶数是否有效（1-3）
            if (order >= 1 && order <= 3 && *endptr == L'\0') {
                for (long i = 0; i < order; ++i) {
                    token = wcstok_s(nullptr, L",", &context);
                    if (token == nullptr) {
                        // 系数数量不足
                        MessageBox(NULL, L"系数数量不足", L"错误", MB_OK | MB_ICONERROR);
                        return nullptr;
                    }
                    double coeff = std::wcstod(token, &endptr);

                    if (*endptr == L'\0') {
						coefficients[i] = coeff;
                    }
                    else {
                        // 系数格式错误，可在此处添加错误处理
                        MessageBox(NULL, L"系数格式错误", L"错误", MB_OK | MB_ICONERROR);
                        return nullptr;
                     
                    }
                }
            }
            else {
                // 阶数无效
                MessageBox(NULL, L"阶数无效", L"错误", MB_OK | MB_ICONERROR);
                return nullptr;

            }
        }
        else {
            // 输入为空
            MessageBox(NULL, L"无输入", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        SpecialImage* res =new SpecialImage(EnDecoding::losslessPredictiveEnCoding(*img, coefficients)) ;

        return res;
    }
    //无损预测解码
    static void* func11(void* image = nullptr, int index = 0) {
        //整个图像序列
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        //待处理的图像
        Image* img = (*images)[index];
        if (img->getType() != Image::Special) {
            MessageBox(NULL, L"图像格式无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
		SpecialImage* specialImg = static_cast<SpecialImage*>(img);

        wchar_t sParams[50];
        InputBox(sParams, 50, L"请输入预测阶数和系数\n（格式：阶数,系数1,系数2,系数3...）\n例如：2,0.5,0.5\n只支持1-3阶");
        vector<double> coefficients(3, 0.0); //预测系数

        wchar_t* context = nullptr;
        wchar_t* token = wcstok_s(sParams, L",", &context); // 
        if (token != nullptr) {
            wchar_t* endptr;
            long order = wcstol(token, &endptr, 10);
            // 检查阶数是否有效（1-3）
            if (order >= 1 && order <= 3 && *endptr == L'\0') {
                for (long i = 0; i < order; ++i) {
                    token = wcstok_s(nullptr, L",", &context);
                    if (token == nullptr) {
                        // 系数数量不足
                        MessageBox(NULL, L"系数数量不足", L"错误", MB_OK | MB_ICONERROR);
                        return nullptr;
                    }
                    double coeff = std::wcstod(token, &endptr);

                    if (*endptr == L'\0') {
                        coefficients[i] = coeff;
                    }
                    else {
                        // 系数格式错误，可在此处添加错误处理
                        MessageBox(NULL, L"系数格式错误", L"错误", MB_OK | MB_ICONERROR);
                        return nullptr;

                    }
                }
            }
            else {
                // 阶数无效
                MessageBox(NULL, L"阶数无效", L"错误", MB_OK | MB_ICONERROR);
                return nullptr;
            }
        }
        else {
            // 输入为空
            MessageBox(NULL, L"无输入", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        Image* res = new Image(EnDecoding::losslessPredictiveDeCoding(*specialImg, coefficients));

        return res;
    }
   
    //均匀量化（finished）
    static void* func12(void* image = nullptr, int index = 0) {
        //整个图像序列
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        //待处理的图像
        Image* img = (*images)[index];
        if (img->getType() != Image::Gray) {
            MessageBox(NULL, L"图像格式无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        wchar_t s[50];
        InputBox(s, 50, L"请输入量化后bit/pixel \n只支持1-8阶");
        int bitperpixel = _wtoi(s);
        if (bitperpixel <= 0 || bitperpixel >= 9) {
            MessageBox(NULL, L"无效的参数", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        
        Image* res = new Image(EnDecoding::uniformQuantization(*img, bitperpixel));

        double error = Image::rootMeanSquareError(res, img);
        cout << "均方根误差：" << error << endl;
        return res;
    }
    //IGS（finished）
    static void* func13(void* image = nullptr, int index = 0) {
        //整个图像序列
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        //待处理的图像
        Image* img = (*images)[index];
        if (img->getType() != Image::Gray) {
            MessageBox(NULL, L"图像格式无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* res = new Image(EnDecoding::IGSQuantization(*img));

        double error = Image::rootMeanSquareError(res, img);
        cout << "均方根误差：" << error << endl;
        return res;
    }
    
    //DCT变换
    static void* func14(void* image = nullptr, int index = 0) {
        //整个图像序列
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        //待处理的图像
        Image* img = (*images)[index];
        if (img->getType() != Image::Gray) {
            MessageBox(NULL, L"图像格式无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        wchar_t s[50];
        InputBox(s, 50, L"请输入分块大小\n");
        int size = _wtoi(s);
        if (size <= 0) {
            MessageBox(NULL, L"无效的参数", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

		double keepRatio = 1.0;
        wchar_t sParams[50] = { 0 };
        InputBox(sParams, 50, L"请输入保留比例\n即每块中保留的系数比例\n不输入或输入错误默认为1.0");
        // 检查用户是否输入了内容（非空字符串）
        if (sParams[0] != L'\0') {  // 若第一个字符不是结束符，说明有输入
            wchar_t* endPtr = nullptr;  // 用于接收转换结束的位置
            double tempRatio = wcstod(sParams, &endPtr);  // 宽字符转 double

            // 验证输入是否有效（排除空输入或非数字）
            if (endPtr != sParams && tempRatio >= 0.0 && tempRatio <= 1.0) {  // 若转换成功（至少解析到一个数字）
                keepRatio = tempRatio;
            }
            else {
                keepRatio = 1.0; // 若输入无效（如非数字），可保持默认值 1.0
            }
           

        }
        SpecialImage* dimage = new SpecialImage(EnDecoding::dct(*img, size, keepRatio));
        
		cout << "DCT变换完成，保留比例：" << keepRatio << endl;
		cout << "生成图像大小:  " << dimage->getWidth() << "x" << dimage->getHeight() << endl;
        
        return dimage;
    }
    //反DCT变换
    static void* func15(void* image = nullptr, int index = 0) {
        //整个图像序列
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        if (images->size() == 0) {
            MessageBox(NULL, L"没有图像可以处理", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        if (index < 0 || index >= images->size()) {
            MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }
        //待处理的图像
        Image* img = static_cast<Image*>((*images)[index]);

        if (img->getType() != Image::Special) {
            MessageBox(NULL, L"图像格式无效", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

		SpecialImage* simage = static_cast<SpecialImage*>(img);

        wchar_t s[50];
        InputBox(s, 50, L"请输入分块大小\n");
        int size = _wtoi(s);
        if (size <= 0) {
            MessageBox(NULL, L"无效的参数", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        Image* res = new Image(EnDecoding::iverseDct(*simage, size));

        cout << "DCT反变换完成 " << endl;
        cout << "还原图像大小:  " << res->getwidth() << "x" << res->getheight() << endl;
        return res;
    }
   
	//均方根误差计算
    static void* func16(void* image = nullptr, int index = 0) {
        //整个图像序列
        vector<Image*>* images = static_cast<vector<Image*>*>(image);
        wchar_t s[50];          
        int index1 = 0, index2 = 0;  

        InputBox(s, 50, L"请输入需要对比的两张图片的索引（仅支持灰度图像，用空格分隔，如：1 2）\n");

        if (swscanf_s(s, L"%d %d", &index1, &index2) == 2) {
            
            if(index1 <= 0 || index1 > images->size() || index2 <= 0 || index2 > images->size()) {
                MessageBox(NULL, L"图像索引无效", L"错误", MB_OK | MB_ICONERROR);
                return nullptr;
			}

            Image* img1 = (*images)[index1-1];
            Image* img2 = (*images)[index2-1];

            if (img1->getType() != Image::Gray||img2->getType()!=Image::Gray) {
                MessageBox(NULL, L"图片类型不支持", L"错误", MB_OK | MB_ICONERROR);
                return nullptr;
            }

            double error = Image::rootMeanSquareError(img1, img2);
            cout << "均方根误差：" << error << endl;

        }
        else {
            MessageBox(NULL, L"无效的索引", L"错误", MB_OK | MB_ICONERROR);
            return nullptr;
        }

        return nullptr;
    }

};


// 定义Widget类，表示一个简单的图形用户界面
class Widget
{
private:
    int width; // 宽度
    int height; // 高度
	int moduleIndex = 0; // 当前选中的模块索引
	vector<vector<Button*>> modules; // 存储不同选项卡对应的按钮，及不同功能模块对应的按钮
    vector<Button*> buttons; // 存储当前显示页面上的按钮
	vector<Tab*> tabs; // 存储页面上的选项卡
    
	vector<TextureButton*> tButtons; // 存储页面上的纹理按钮
   
	Button*  Dbutton; // 删除当前展示的图片的按钮
    vector<Image*>images;//要展示的图片序列
	int imageIndex = 0;//当前展示的图片索引

    // 给某一个模块上添加一个按钮
    void addButton(Button* button, int module)
    {
        modules[module].push_back(button);
    }
    // 在页面上添加一个选项卡
    void addTab(Tab* tab)
    {
        tabs.push_back(tab);
	}
 
    // 处理鼠标点击事件
    void mouseClick(int mouseX, int mouseY)
    {
		
        for(TextureButton* tButton : tButtons)
        {
            if (tButton->checkClick(mouseX, mouseY)) {
                return;
                // 如果点击了一个选择按钮，停止检查其他选择按钮
            }
        }

        for(Button* button : buttons)
        {               
            Image* res = (Image*)(button->checkClick(mouseX, mouseY,&images,imageIndex));
            if (res!=nullptr)
            {
                images.push_back(res);
                imageIndex = images.size() - 1;
                // 更新当前展示的图片索引为最新添加的图片
                return;
            }
		}
        Dbutton->checkClick(mouseX, mouseY, &images); // 检查删除按钮是否被点击

        for (Tab* tab : tabs)
        {
            if (tab->checkClick(mouseX, mouseY))
            {
                return; // 如果点击了一个选项卡，停止检查其他选项卡
            }

        }

    }

    // 处理鼠标移动事件
    void mouseMove(int mouseX, int mouseY)
    {
        Dbutton->checkMouseOver(mouseX, mouseY); // 检查删除按钮是否悬停  
        for (TextureButton* tButton : tButtons)
        {
            if (tButton->checkMouseOver(mouseX, mouseY)) {
                return;
                // 如果悬停在一个选择按钮上，停止检查其他选择按钮
            }
        }
        for (Button* button : buttons)
        {
            if (button->checkMouseOver(mouseX, mouseY))
            {
                return;
                // 如果悬停在一个按钮，停止检查其他
            }
        }
        for (Tab* tab : tabs)
        {
            if (tab->checkMouseOver(mouseX, mouseY))
            {
                return; // 如果点击了一个选项卡，停止检查其他选项卡
            }

        }
    }

    // 绘制当前页面的按钮、选择按钮和选项卡,以及图像
    void draw()
    {
        // 绘制图像
        if (!images.empty()) {
            
            IMAGE img;
            if (images[imageIndex]->getType() == Image::Special) {
                SpecialImage* simage = static_cast<SpecialImage*>(images[imageIndex]);

                Image tmp = EnDecoding::linearTransformTo0_255(*simage);
                img = tmp.convertToEasyXImage();
            }
            else {
                img = (images[imageIndex])->convertToEasyXImage();// 显示最新的图像
            }
           
            int x = buttons[0]->getX() + buttons[0]->getWidth() + 20; 
            // 图像显示在按钮右侧，留出20像素的间距
            int y = height / 7;
            // 设置最大显示尺寸（可以根据需要调整）
            int maxDisplayWidth = tabs[0]->getX() - x - 20;  // 右侧与选项卡留20像素边距
            int maxDisplayHeight = height - height * 2 / 7; // 底部留20像素边距

            // 绘制图像背景（可选，用于区分图像区域）
            setfillcolor(RGB(240, 240, 240));  // 浅灰色背景
            solidrectangle(x, y, x + maxDisplayWidth, y + maxDisplayHeight);

            // 绘制图像边框
            setlinecolor(LIGHTGRAY);
            rectangle(x, y, x + maxDisplayWidth, y + maxDisplayHeight);

            // 获取图像原始尺寸
            int imgWidth = img.getwidth();
            int imgHeight = img.getheight();

            // 计算等比例缩放因子
            double scaleX = (double)maxDisplayWidth / imgWidth;
            double scaleY = (double)maxDisplayHeight / imgHeight;
            double scale = min(scaleX, scaleY);  // 选择较小的缩放因子，确保完整显示

            // 如果图像已经小于显示区域，则不缩放
            if (scale >= 1.0) {
                scale = 1.0;
            }

            // 计算缩放后的尺寸
            int scaledWidth = (int)(imgWidth * scale);
            int scaledHeight = (int)(imgHeight * scale);


            //IMAGE* res=Image::resizeImageBicubic(&img, scaledWidth, scaledHeight); // 使用双三次插值缩放图像
            IMAGE res = Image::resize(&img, scaledWidth, scaledHeight); // 使用简单缩放方法缩放图像

            // 计算居中显示的位置（在按钮右侧区域内）
            int centeredX = x + (maxDisplayWidth - scaledWidth) / 2;
            int centeredY = y + (maxDisplayHeight - scaledHeight) / 2;
            putimage(centeredX, centeredY, &res); // 在指定位置绘制图像


            // 提示信息：当前展示的图像索引和总图像数量
            settextcolor(BLACK);
            settextstyle(15, 0, _T("宋体"));

            //文本右侧对齐显示
            wstring text = L"当前图像: " + to_wstring(imageIndex + 1) + L"/" + to_wstring(images.size());
            int textX = x + maxDisplayWidth - textwidth(text.c_str());
            // 右侧对齐，留10像素边距
            int textY = y + maxDisplayHeight - textheight(text.c_str()); // 图像下方，留10像素边距

            outtextxy(textX, textY, text.c_str());



        }
        // 绘制按钮和选项卡
        Dbutton->draw(); // 绘制删除图像的按钮
        for (TextureButton* tButton : tButtons)
        {
            tButton->draw(); // 绘制当前页面上的所有选择按钮
        }

        for (Button* button : buttons)
        {
            button->draw(); // 绘制当前页面上的所有按钮
        }
        for (Tab* tab : tabs)
        {
            tab->draw(); // 显示当前页面上的所有选项卡
        }
    }
      
    
    // 绘制主菜单
    void drawMainMenu() {

        cleardevice();

        // 标题
        //居中显示文本
        LOGFONT f;
        gettextstyle(&f);						// 获取当前字体设置
        f.lfHeight = 48;						// 设置字体高度为 48
        _tcscpy_s(f.lfFaceName, _T("黑体"));      // 设置字体为“黑体”
        f.lfQuality = ANTIALIASED_QUALITY;		// 设置输出效果为抗锯齿  
        settextstyle(&f);						// 设置字体样式


        int textX = (width - textwidth(L"图像处理实验 - 主菜单")) / 2; // 计算文本在按钮中央的x坐标

        int textY = height / 20;
        settextcolor(BLACK);
        outtextxy(textX, textY, L"图像处理实验 - 主菜单");

        draw();

        // 提示信息
        settextcolor(BLACK);
        settextstyle(20, 0, _T("宋体"));
        textX = (width - textwidth(L"点击右侧选择功能模块，左侧选择具体功能")) / 2;
        textY = height * 9 / 10;
        outtextxy(textX, textY, L"点击右侧选择功能模块，左侧选择具体功能");

    }


    // 初始化不同模块对应的按钮
    void initModuleButtons() {

		int buttoncounts[] = { 2, 2, 2, 3, 7 }; //每个模块的按钮数量
        Functions f; int k = 0;
        for (int j = 0; j < modules.size(); j++)
        {
            // --- 按钮布局逻辑 ---
            int buttonCount = buttoncounts[j];
            int buttonWidth = width / 10;
            int buttonHeight = buttonWidth / 2;
            int buttonSpacing = 20; // 按钮之间的垂直间距

            // 计算按钮总高度（所有按钮 + 间距）
            int totalButtonsHeight = buttonCount * buttonHeight + (buttonCount - 1) * buttonSpacing;

            // 让按钮区域在垂直方向上居中
            int startY = (height - totalButtonsHeight) / 2;

            // 让按钮在水平方向上处于窗口左边 1/8 处（留出空间给Tab）
            int startX = width / 8 - buttonWidth / 2; // 放在窗口左边 1/8 处

            for (int i = 0; i < buttonCount; i++)
            {
                Button* button = new Button(startX, startY + i * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight, f.name[k], f.onClick[k]);
              
                ++k;
                addButton(button, j);
               
            }
        }
    }

public:
    Widget(int width, int height)
        :width(width), height(height)
    {
        modules.resize(5);
        Dbutton = nullptr;

    }

    ~Widget() {
        delete Dbutton;
        for (TextureButton* tButton : tButtons) {
            delete tButton;
        }
        for (auto vec : modules) {
            for (auto b : vec) {
                delete b;
            }
        }
        for (auto t : tabs) {
            delete t;
        }
        for (auto i : images) {
            delete i;
        }
    }

    // 初始化控件，创建图形环境，设置页面和按钮
    void init()
    {
        initgraph(width, height, EX_SHOWCONSOLE);
        setbkcolor(WHITE);
        initModuleButtons();

        //////////////初始化选项卡//////////////////////////////////////

        int initwidth = width / 10; // 选项卡宽度
        int initheight = initwidth / 5; // 选项卡高度
        // 让选项卡在水平方向上处于窗口右边 1/8 处
        // 放在窗口右边 1/8 处
        int initx = (width * 7.0) / 8.0 - initwidth / 2.0; // 选项卡初始x坐标 
        int inity = (height - initheight) / 2; // 选项卡y坐标     
        Tab* tab = new Tab(5, { L"文件处理模块", L"裁剪切割模块", L"模式转换模块", L"图像增强模块" ,L"图像编码模块" }, initx, inity, initwidth, initheight);
        addTab(tab);
        buttons = modules[moduleIndex];

        /////////////////图像删除按钮////////////////////////////////////
        
        Dbutton= new Button(initx, inity + initheight * 6, initwidth, initheight, L"删除图像", [this](void* img,int index) {

            if (!images.empty()) {
                if (0 <= imageIndex && imageIndex < images.size()) {
                    images.erase(images.begin() + imageIndex);
                    if (imageIndex >= images.size()) {
                        imageIndex = images.size() - 1; // 更新当前展示的图片索引为最后一张图像
                    }

                }
            }
            return nullptr;
            });

        ///////////////////////图像选择按钮////////////////////////////////

        IMAGE temp;
        loadimage(&temp, L"next.png"); // 加载图像以获取其尺寸
        float scale = (float)((initwidth * 1.0) / temp.getwidth()); // 计算缩放比例

        IMAGE* next = new IMAGE();
        loadimage(next, L"next.png", temp.getwidth() * scale, temp.getheight() * scale);

        loadimage(&temp, L"prev.png"); // 加载图像以获取其尺寸
        scale = (float)((initwidth * 1.0) / temp.getwidth()); // 计算缩放比例

        IMAGE* prev = new IMAGE();
        loadimage(prev, L"prev.png", temp.getwidth() * scale, temp.getheight() * scale);

        inity = height * 3.0 / 4.0 - initheight / 2.0;

        TextureButton* nextButton = new TextureButton(initx, inity, initwidth, next->getheight(), next);
        inity = height / 4.0 - initheight / 2.0;
        TextureButton* prevButton = new TextureButton(initx, inity, initwidth, prev->getheight(), prev);

        tButtons.push_back(nextButton);
        tButtons.push_back(prevButton);


        nextButton->setOnclick([this](void* img,int index) {
            if (!images.empty()) {
                imageIndex = (imageIndex + 1) % images.size(); // 显示下一张图像
            }
            return nullptr;
            });

        prevButton->setOnclick([this](void* img,int index) {
            if (!images.empty()) {
                imageIndex = (imageIndex - 1 + images.size()) % images.size(); // 显示上一张图像
            }
            return nullptr;
            });



    }

    // 运行，进入消息循环
    void run()
    {
        ExMessage msg;
        BeginBatchDraw(); // 开始批量绘制

        while (true)
        {
            moduleIndex = tabs[0]->returnSelectedOption(); // 获取当前选中的选项卡索引，根据索引切换显示的按钮
            buttons = modules[moduleIndex];
            while (peekmessage(&msg)) // 检查是否有消息
            {
                int mouseX = msg.x; // 获取鼠标x坐标
                int mouseY = msg.y; // 获取鼠标y坐标

                switch (msg.message)
                {
                case WM_LBUTTONDOWN: // 鼠标左键按下事件
                    mouseClick(mouseX, mouseY); // 处理鼠标点击事件

                    break;
                case WM_MOUSEMOVE: // 鼠标移动事件
                    mouseMove(mouseX, mouseY); // 处理鼠标移动事件

                    break;
                }
            }

            flushmessage(); // 刷新消息队列，处理完所有消息后继续执行下面的代码

            drawMainMenu(); // 绘制主菜单

            FlushBatchDraw(); // 将缓冲区内容显示在屏幕上
            Sleep(1000 / 60);
        }

        EndBatchDraw(); // 结束批量绘制
    }

    // 关闭
    void close()
    {
        closegraph(); // 关闭图形环境
    }
};
