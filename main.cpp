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
#include <time.h>
#include <math.h>
#include "tools.h"
#include "vector2.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

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
	int frameIndex;	// 序列帧的序号

	bool catched;	// 被僵尸捕获
	int deadTime;	// 死亡计数器

	int timer;
	int x, y;
};

struct Plant map[3][9];

enum {SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT};

struct SunshineBall
{
	int x, y;		// 阳光球在飘落过程中的坐标位置（x不变）
	int frameIndex;	// 当前显示的图片帧序号
	int destY;		// 飘落的目标位置的y坐标
	bool used;		// 是否在使用
	int timer;

	float xoff;
	float yoff;

	float t;		// 贝塞尔曲线的时间点 0..1
	vector2 p1, p2, p3, p4;
	vector2 pCur;	// 当前时刻阳光球的位置
	float speed;
	int status;
};

struct SunshineBall balls[10];
IMAGE imgSunshineBall[29];
int sunshine;

struct Zombie
{
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;
	bool dead;
	bool eating;	// 正在吃植物
};
struct Zombie zms[10];
IMAGE imgZombie[22];
IMAGE imgZombieDead[20];
IMAGE imgZombieEat[21];

// 子弹的数据类型
struct bullet
{
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;	//是否发射爆炸
	int frameIndex;	//帧序号
};
struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];

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
	sunshine = 50;

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++)
	{
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	// 配置随机种子
	srand(time(NULL));

	// 创建游戏的图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	// 设置字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWidth = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;	// 抗锯齿效果
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);

	// 初始化僵尸数据
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i);
		loadimage(&imgZombie[i], name);
	}

	loadimage(&imgBulletNormal, "res/bullets/PeaNormal/PeaNormal_0.png");
	memset(bullets, 0, sizeof(bullets));

	// 初始化豌豆子弹的帧图片数组
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[3].getwidth() * k,
			imgBulletBlast[3].getheight() * k, true);
	}

	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZombieDead[i], name);
	}

	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZombieEat[i], name);
	}
}

void drawZM()
{
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used)
		{
			//IMAGE* img = &imgZombie[zms[i].frameIndex];
			//IMAGE* img = (zms[i].dead) ? imgZombieDead : imgZombie;
			IMAGE* img = NULL;
			if (zms[i].dead)
			{
				img = imgZombieDead;
			}
			else if (zms[i].eating)
			{
				img = imgZombieEat;
			}
			else
			{
				img = imgZombie;
			}

			img += zms[i].frameIndex;

			putimagePNG(
				zms[i].x, 
				zms[i].y - img->getheight(), 
				img);
		}
	}
}

void drawSunshines()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used)
		{
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}
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

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				int plantType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				putimagePNG(map[i][j].x, map[i][j].y, imgPlants[plantType][index]);
			}
		}
	}

	// 渲染拖动过程中的植物
	if (curPlant > 0)
	{
		IMAGE* img = imgPlants[curPlant - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	drawSunshines();

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	if (sunshine < 100)
	{
		outtextxy(280, 67, scoreText);	// 输出分数
	}
	else
	{
		outtextxy(276, 67, scoreText);	// 输出分数
	}

	drawZM();

	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++)
	{
		if (bullets[i].used)
		{
			if (bullets[i].blast)
			{
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}

	EndBatchDraw();	//结束双缓冲
}

void collectSunshine(ExMessage* msg)
{
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for (int i = 0; i < count; i++)
	{
		if (balls[i].used)
		{
			int x = balls[i].pCur.x;
			int y = balls[i].pCur.y;

			if (msg->x > x && msg->x < x + w &&
				msg->y > y && msg->y < y + h)
			{
				balls[i].used = false;
				balls[i].status = SUNSHINE_COLLECT;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);
				balls[i].p1 = balls[i].pCur;
				balls[i].p4 = vector2(262, 0);
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i].p4);
				float off = 8;
				balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
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
			else
			{
				collectSunshine(&msg);
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

					map[row][col].x = 256 + col * 81;
					map[row][col].y = 179 + row * 102 + 14;
				}
			}

			curPlant = 0;
			status = 0;
		}
	}

}

void createSunshine()
{
	static int count = 0;
	static int fre = 400;
	count++;
	if (count >= fre)
	{
		fre = 200 + rand() % 200;
		count = 0;

		// 从阳光池中取一个可以使用的
		int ballMax = sizeof(balls) / sizeof(balls[0]);

		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)
		{
			return;
		}

		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;

		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 + rand() % (900 - 260), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}

	// 向日葵生产阳光
}

void createZombie()
{
	static int zmFre = 200;
	static int count = 0;
	count++;
	if (count > zmFre)
	{
		count = 0;
		zmFre = rand() % 200 + 300;

		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
		if (i < zmMax)
		{
			memset(&zms[i], 0, sizeof(zms[i]));
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;	//0..2
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 1;
			zms[i].blood = 100;
			zms[i].dead = false;
		}
	}
}

void updateSunshine()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used)
		{
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if (balls[i].timer == 0)
			{
				balls[i].y += 2;
			}

			if (balls[i].y >= balls[i].destY)
			{
				balls[i].timer++;
				if (balls[i].timer > 100)
				{
					balls[i].used = false;
				}
			}
		}
		else if (balls[i].xoff)
		{
			// 设置阳光球的偏移量
			float destY = 0;
			float destX = 262;
			float angle = atan((balls[i].y - destY) / (balls[i].x - destX));
			balls[i].xoff = 4 * cos(angle);
			balls[i].yoff = 4 * sin(angle);

			balls[i].x -= balls[i].xoff;
			balls[i].y -= balls[i].yoff;
			if (balls[i].y < 0 || balls[i].x < 262)
			{
				balls[i].xoff = 0;
				balls[i].yoff = 0;
				sunshine += 25;
			}
		}
	}
}

void updateZombie()
{
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	static int count = 0;
	count++;
	if (count > 2)
	{
		count = 0;

		// 更新僵尸的位置
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)
			{
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 170)
				{
					//printf("GAME OVER\n");
					//MessageBox(NULL, "over", "over", 0);	//待优化
					//exit(0);	//待优化
				}
			}
		}
	}

	static int count2 = 0;
	count2++;
	if (count2 > 4)
	{
		count2 = 0;
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)
			{
				if (zms[i].dead)
				{
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20)
					{
						zms[i].used = false;
					}
				}
				else if (zms[i].eating)
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}
}

void shoot()
{
	int lines[3] = { 0 };
	int zmCnt = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int danagerX = WIN_WIDTH - imgZombie[0].getwidth();
	for (int i = 0; i < zmCnt; i++)
	{
		if (zms[i].used && zms[i].x < danagerX)
		{
			lines[zms[i].row] = 1;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == PEASHOOTER + 1 && lines[i])
			{
				static int count = 0;
				count++;
				if (count > 20)
				{
					count = 0;

					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax)
					{
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 6;

						bullets[k].blast = false;
						bullets[k].frameIndex = 0;

						int plantX = 256 + j * 81;
						int plantY = 179 + i * 102 + 14;
						bullets[k].x = plantX + imgPlants[map[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = plantY + 5;
					}
				}
			}
		}
	}
}

void updateBullets()
{
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++)
	{
		if (bullets[i].used)
		{
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH)
			{
				bullets[i].used = false;
			}
			
			// 待实现子弹的碰撞检测
			if (bullets[i].blast)
			{
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4)
				{
					bullets[i].used = false;
				}
			}
		}
	}
}

void checkBullet2Zm()
{
	int bulletCnt = sizeof(bullets) / sizeof(bullets[0]);
	int zombieNum = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bulletCnt; i++)
	{
		if (bullets[i].used == false || bullets[i].blast)
		{
			continue;
		}

		for (int k = 0; k < zombieNum; k++)
		{
			if (zms[k].used == false || zms[k].dead)
			{
				continue;
			}

			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (bullets[i].row == zms[k].row && x > x1 && x < x2)
			{
				bullets[i].blast = true;
				zms[k].blood -= 10;
				bullets[i].speed = 0;

				if (zms[k].blood <= 0)
				{
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
				}

				break;
			}
		}
	}
}

void checkZm2Plant()
{
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zCount; i++)
	{
		if (zms[i].dead || !zms[i].used)
		{
			continue;
		}

		int row = zms[i].row;
		for (int k = 0; k < 9; k++)
		{
			if (map[row][k].type == 0)
			{
				continue;
			}

			//    x1  x2
			//    [    ]
			//      x3
			int plantX = 256 + k * 81;
			int x1 = plantX + 10;
			int x2 = plantX + 60;
			int x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2)
			{
				if (map[row][k].catched)
				{
					map[row][k].deadTime++;
					if (map[row][k].deadTime > 100)
					{
						map[row][k].deadTime = 0;
						map[row][k].type = 0;
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;
					}
				}
				else
				{
					map[row][k].catched = true;
					map[row][k].deadTime = 0;
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;
				}
			}
		}
	}
}

void collisionCheck()
{
	checkBullet2Zm();	// 子弹对僵尸的碰撞检测
	checkZm2Plant();	// 僵尸对植物的碰撞检测
}

void updateGame()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				map[i][j].frameIndex++;
				int plantType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgPlants[plantType][index] == NULL)
				{
					map[i][j].frameIndex = 0;
				}
			}
		}
	}

	createSunshine();	// 创建阳光
	updateSunshine();	// 更新阳光状态

	createZombie();	// 创建僵尸
	updateZombie();	// 更新僵尸状态

	shoot();	// 发射豌豆子弹
	updateBullets();	// 更新豌豆子弹

	collisionCheck();	//实现豌豆子弹和僵尸的碰撞检测
}

void startUI()
{
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;

	while (1)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);

		ExMessage msg;
		if (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN &&
				msg.x > 474 && msg.x < 474 + 300 &&
				msg.y > 75  && msg.y < 75 + 140)
			{
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP && flag)
			{
				return;
			}
		}

		EndBatchDraw();
	}
}

int main(void)
{
	gameInit();

	startUI();

	int timer = 0;
	bool flag = true;
	while (1)
	{
		userClick();
		timer += getDelay();
		if (timer > 20)
		{
			flag = true;
			timer = 0;
		}

		if (flag)
		{
			flag = false;
			updateWindow();
			updateGame();
		}
	}

	system("pause");
	return 0;
}