#include<iostream>
#include"Image.h"

#include"UI.h"






int main()
{
    Widget widget(1000, 600);
    widget.init();
    widget.run();
    widget.close();
    return 0;
}

