/*
* 开发日志
* 1. 创建新项目（空项目模板）使用vs的任意版本
* 2. 导入素材
* 3. 实现最开始游戏的场景
* 4. 实现游戏顶部的工具栏
* 5. 实现工具栏中的植物卡牌
* 
*/

#include <stdio.h>
#include <graphics.h>	//easyx图形库的头文件，需要安装easyx图形库
#include "tools.h"

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum
{
	PEASHOOTER,
	SUNFLOWER,
	PLANT_COUNT
};

IMAGE imgBg;	//表示背景图片
IMAGE imgBar;	//表示工具栏图片
IMAGE imgCards[PLANT_COUNT];

void gameInit()
{
	// 加载有背景的图片
	// 把字符集改成多字节字符集
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");
	
	//初始化植物卡牌
	char name[64];
	for (int i = 0; i < PLANT_COUNT; i++)
	{
		// 生成植物卡牌的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);
	}

	// 创建游戏的图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT);
}

void updateWindow()
{
	putimage(0, 0, &imgBg);
	putimagePNG(250, 0, &imgBar);

	for (int i = 0; i < PLANT_COUNT; i++)
	{
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
}

int main(void)
{
	gameInit();

	updateWindow();

	system("pause");
	return 0;
}