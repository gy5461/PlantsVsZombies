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
IMAGE *imgPlants[PLANT_COUNT][20];

int curX, curY;	//当前选中的植物，在移动过程中的位置
int curPlant; // 0: 没有选中，1: 选中第一种

struct Plant
{
	int type;		// 0: 没有选中，1: 选中第一种
	int frameIndex;	//序列帧的序号
};

struct Plant map[3][9];

bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		return false;
	}

	fclose(fp);
	return true;
}

void gameInit()
{
	// 加载有背景的图片
	// 把字符集改成多字节字符集
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	memset(imgPlants, 0, sizeof(imgPlants));
	memset(map, 0, sizeof(map));
	
	//初始化植物卡牌
	char name[64];
	for (int i = 0; i < PLANT_COUNT; i++)
	{
		// 生成植物卡牌的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			// 先判断这个文件是否存在
			if (fileExist(name))
			{
				imgPlants[i][j] = new IMAGE;
				loadimage(imgPlants[i][j], name);
			}
			else
			{
				break;
			}
		}
	}

	curPlant = 0;
	// 创建游戏的图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);
}

void updateWindow()
{
	BeginBatchDraw();	//开始缓冲

	putimage(0, 0, &imgBg);
	putimagePNG(250, 0, &imgBar);

	for (int i = 0; i < PLANT_COUNT; i++)
	{
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}

	// 渲染拖动过程中的植物
	if (curPlant > 0)
	{
		IMAGE* img = imgPlants[curPlant - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			int x = 256 + j * 81;
			int y = 179 + i * 102 + 14;
			int plantType = map[i][j].type - 1;
			int index = map[i][j].frameIndex;
			if (plantType >= 0)
			{
				putimagePNG(x, y, imgPlants[plantType][index]);
			}
		}
	}

	EndBatchDraw();	//结束双缓冲
}

void userClick()
{
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg))
	{
		if (msg.message == WM_LBUTTONDOWN)
		{
			if (msg.x > 338 && msg.x < 338 + 65 * PLANT_COUNT && msg.y >6 && msg.y < 96)
			{
				int index = (msg.x - 338) / 65;
				status = 1;
				curPlant = index + 1;
				curX = msg.x;
				curY = msg.y;
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)
		{
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP)
		{
			if (msg.x > 256 && msg.y > 179 && msg.y < 489)
			{
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;

				if (map[row][col].type == 0)
				{
					map[row][col].type = curPlant;
					map[row][col].frameIndex = 0;
				}
			}

			curPlant = 0;
			status = 0;
		}
	}

}

int main(void)
{
	gameInit();

	while (1)
	{
		userClick();

		updateWindow();
	}

	system("pause");
	return 0;
}