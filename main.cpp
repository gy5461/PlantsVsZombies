/*
* 开发日志
* 1. 创建新项目（空项目模板）使用vs的任意版本
* 2. 导入素材
* 3. 实现最开始游戏的场景
* 
* 
*/

#include <stdio.h>
#include <graphics.h>	//easyx图形库的头文件，需要安装easyx图形库

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

IMAGE imgBg;	//表示背景图片

void gameInit()
{
	// 加载有背景的图片
	// 把字符集改成多字节字符集
	loadimage(&imgBg, "res/bg.jpg");

	// 创建游戏的图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT);
}

void updateWindow()
{
	putimage(0, 0, &imgBg);
}

int main(void)
{
	gameInit();

	updateWindow();

	system("pause");
	return 0;
}