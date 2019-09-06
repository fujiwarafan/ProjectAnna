#include"stdafx.h"
#include"DrawTable.h"

//这是程序主逻辑所在的位置
int main(int argv, char* argc[])
{
	//ofstream fcout;
	//fcout.open("communitionlog.txt");
	//fcout.close();

	initgraph(800, 600);   // 创建绘图窗口，大小为 640x480 像素
	//circle(640/2, 480/2, 50); // 画圆，圆心(200, 200)，半径 100
	setbkcolor(GREEN);
	cleardevice();
	TCHAR input[80];
	InputBox(
		input,
		50,
		_T("请输入轮局配置信息,格式:\nAIposi,dealer,turnid,turncount,turntime,\nroundid,roundcount,time,vulnerable,seed"),
		_T("Config"),
		_T("0,0,1,4,1200,1,4,20,1,123456"),
		0,
		0,
		true
	);
	int dealer = 0;
	int seed = 0;
	int aiposition = -1;
	int info[8] = { 1,1,6,1200,1,8,15,1 };
	swscanf_s(input,_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), &aiposition,&dealer,&info[1],&info[2],&info[3],&info[4],&info[5] ,&info[6] ,&info[7],&seed);
	
	//line(180, 40, 270*2, 40);
	//line(700, 40, 700, 130*2);
	//line(180, 460, 270*2, 460);
	//line(40, 40, 40, 130*2);





	GAME_STATUS game(dealer, seed,info[7], info[1], info[2]);//发牌方 种子 局况 当前轮 总轮
	Player ai(_T("./ai.exe")); //乘3
	/*希望在这里支持自由的ai数量配置*/
	CardsImg drawcards;
	TCHAR output[80];
	wsprintf(output,_T("ai1状态%d\n,ai2状态\n,ai3状态\n"), ai.StatuCheck());
	outtextxy(0, 0, output);
	//printf("创建完成");
	//游戏的配置信息
	int cards[52] = { 0 };
	int aicards[13] = { 0 };//乘3
	
	int avalibleCards[13];
	int avalibleBid[8][5];
	//int seed = 123;
	int process = 0;
	int leader = -1;
	ai.UploadInfo(info);//乘3
	int playFlag = 0;
	int bidFlag = 0;//叫牌错误次数
	int bidcheck = -1;

	int bidRecoder[100];//与game.bidRecoder内容相同，play阶段画图用
	int bidCount = 1;//同上

	//主要过程
	while (1)
	{
		process = game.WhatToDo();
		wsprintf(output,_T("\n游戏状态标志%2d\n"), process);
		outtextxy(0, 0, output);
		int num = 0;
		int result = -1;
		int score = 0;
		switch (process / 10)
		{
		case 0:
			cleardevice();
			wsprintf(output,_T("发牌\n"));
			outtextxy(0, 0, output);
			game.DEAL();
			game.GetCards(cards);

			for (int i = 0, j = 0; i < 52; i++)
			{
				if (cards[i] == aiposition)
				{
					aicards[j++] = i;
				}
				//改成发三遍
			}
			//绘制
			drawcards.DrawCards(cards);
			ai.UploadDeal(aiposition, aicards);
			//乘三
			break;

		case 1:
			wsprintf(output,_T("叫牌\n"));
			outtextxy(0, 0, output);
			

			for (int i = 1; i < 8; i++)
				for (int j = 1; j < 6; j++)
					avalibleBid[i - 1][j - 1] = i * 10 + j;

			avalibleBid[7][0] = 80;
			avalibleBid[7][1] = 90;
			avalibleBid[7][2] = 0;
			avalibleBid[7][3] = 0;
			avalibleBid[7][4] = 0;


			for (int i = 0; i < (game.nowBid / 10 % 10 - 1); i++)
				for (int j = 0; j < 5; j++)
					avalibleBid[i][j] = -1;//划掉不能叫的行
			for (int i = 0; i <= (game.nowBid % 10); i++)
				avalibleBid[(game.nowBid / 10 % 10 - 1)%8][i] = -1;//划掉不能叫的列
			
			drawcards.DrawBids(avalibleBid);

			wsprintf(output,_T("当前%d\n"), game.nowBid);
			outtextxy(0, 0, output);

			/*下面这些也要乘三*/
			if (process % 10 == aiposition && bidFlag>1)
				//如果ai之前已经发送了错误的叫牌命令 直接让其pass
			{
				result = 0;
			}
			else if (process % 10 == aiposition)
			{
				result = ai.InquireBid();
				wsprintf(output,_T("ai叫牌%d\n"), result);
				outtextxy(0, 14, output);
			}

			else
			{
				result = MakeBid(process % 10, avalibleBid[0]);	
			}

			bidcheck = game.BID(process % 10, result / 10, result % 10);
			if (bidcheck == 0)//如果合法 不合法是-1
			{
				//这里需要有一个循环遍历四家 打牌时候也得是这样
				if (process % 10 != aiposition)//改成不等于当前ai方位呗 出牌到时候也得是
				{
					ai.UploadBid(result + process % 10 * 100);
				}
				//调用绘制叫牌历史
				drawcards.DrawBided(game.bidCount, game.bidRecoder);

				if (game.bidCount != 0)
				{
					bidCount = game.bidCount;
					for (int i = 0; i < bidCount; i++)
					{
						bidRecoder[i] = game.bidRecoder[i];
					}
				}

			}
			else
			{
				bidFlag += 1;
			}
				
			break;
	
		case 2:
			wsprintf(output,_T("打牌\n"));
			outtextxy(0, 14, output);
			
			
			if (aiposition == game.dummy)//这个判断届时可以删除了
			{
				wsprintf(output, _T("该方位为明手\n现在可以关闭了"));
				outtextxy(0, 0, output);
			}
			if (playFlag % 4 == 0)//如果是一墩开始
			{
				Sleep(2000);
				leader = process % 10;
				cleardevice();			//用背景色清空屏幕
				game.GetCards(cards);
				drawcards.DrawCards(cards);
				drawcards.DrawTrick(game.trick);
				drawcards.DrawContract(bidCount, bidRecoder);
				//printf("======================================================\n");
			}
			if (playFlag == 0)//如果是刚开始打牌
			{
				//乘三
				ai.UploadContover(game.banker * 1000 + game.contover * 100 + game.trump * 10 + game.dbl);
				//在这里告诉AI订约
			}
			if (playFlag == 1)//如果是首攻完事
			{
				for (int i = 0, j = 0; i < 52; i++)
				{
					if (cards[i] == game.dummy)
					{
						aicards[j++] = i;
					}
				}
				//乘三
				ai.UploadDummy(game.dummy, aicards);
			}
			
			for (int i = 0; i < 13; i++)
				avalibleCards[i] = -1;
			
				for (int i = 0; i < 52; i++)
				{
					if (game.cardPosition[0][i] == process % 10 && game.cardPosition[1][i] == 1)
					{
						avalibleCards[num++] = i;
					}
				}
				drawcards.DrawBided(game.bidCount, game.bidRecoder);
				//开始询问出牌
				
				if (aiposition == process % 10)//ai出牌
				{
					wsprintf(output,_T("等待ai出牌...\n"));
					outtextxy(0, 14, output);
					result = ai.InquirePlay(process % 10);
					
				}
				else if ((aiposition == game.banker) && (process % 10 == game.dummy))//ai为明手出牌
				{
					wsprintf(output, _T("等待ai为明手出牌...\n"));
					outtextxy(0, 14, output);
					result = ai.InquirePlay(process % 10);
					//ai.UploadPlay(process % 10, result);
				}
				else
				{
					result = ChoseCardToPlay(process % 10, avalibleCards);
					
				}
				
				if (game.PLAY(process % 10, result / 4 + 2, result % 4) == 0)
				{
					//这里控制给ai转发出牌信息
					//if (process % 10 != aiposition || (game.banker == aiposition && process % 10== game.dummy))//警告
					if (process % 10 != aiposition )
					{
						ai.UploadPlay(process % 10, result);//合法则上传给AI出牌信息
					}

					cleardevice();			//用背景色清空屏幕
					game.GetCards(cards);
					drawcards.DrawTrick(game.trick);
					drawcards.DrawContract(bidCount, bidRecoder);
					drawcards.DrawCards(cards);
					//drawcards.DrawBided(game.bidCount, game.bidRecoder);
					//显示当前牌局
					drawcards.DrawPlayed(leader, game.cardRecorder + (playFlag / 4 * 4));
					playFlag += 1;
				}
				break;
		case 3:
			wsprintf(output,_T("结算\n"));
			outtextxy(0, 14, output);
			score = game.GetScore();
			wsprintf(output,_T("score:%d\n"), score);
			outtextxy(0, 14, output);
			MessageBox(NULL, TEXT("对局结束！"), TEXT("Info"), MB_OK);
			ai.UploadOverMsg(game.GetResult());
			//game.SetForNewGame();
			game.gameStatus = 5;
			break;
			
		}
	}
		
		delete &ai;
		delete &game;
		return 0;
	
}

CardsImg::CardsImg(void)
{
	//图像加载
	for (int i = 0; i < 52; i++)
	{
		TCHAR imgname[50];
		wsprintf(imgname, _T("./cards/%d.bmp"), i);
		loadimage(&cards_img[i], imgname);
	}
	for (int i = 0; i <= 90; i++)
	{
		bid_img[i] = NULL;
	}
	for (int i = 0; i <= 7; i++)
	{
		for (int j = 0; j <= 5; j++)
		{
			TCHAR imgname[50];
			wsprintf(imgname, _T("./bids/%d.jpg"), i * 10 + j);
			loadimage(&bid_img[i * 10 + j], imgname);
		}
	}
	//存加倍，再加倍，和不能叫的图像
	loadimage(&bid_img[01], _T("./bids/-1.jpg"));
	loadimage(&bid_img[80], _T("./bids/80.jpg"));
	loadimage(&bid_img[90], _T("./bids/90.jpg"));
	return;
}

void CardsImg::DrawBids(int avaliableBid[8][5])
{
	for (int i = 1, n = 130; i <= 7; i++)
	{
		for (int j = 1, w = 0; j <= 5; j++)
		{
			if (avaliableBid[i - 1][j] == -1)
				putimage(290 + (w++) * 40, n, &bid_img[01]);
			else
				putimage(290 + (w++) * 40, n, &bid_img[i * 10 + j]);
			if (j == 5)
				n += 40;
		}
	}
	putimage(290, 410, &bid_img[0]);
	putimage(410, 410, &bid_img[80]);
	putimage(450, 410, &bid_img[90]);
	return;

}
void CardsImg::DrawCards(int cards[])
{


	for (int i = 0; i < 13; i++)
	{
		cards_N[i] = -1;
		cards_E[i] = -1;
		cards_S[i] = -1;
		cards_W[i] = -1;
	}
	//找出4家手牌
	for (int i = 0, q = 0, w = 0, e = 0, r = 0; i < 52; i++)
	{
		//if(cards[i]==-1)

		if (cards[i] == 0)
			cards_N[q++] = i;
		else if (cards[i] == 1)
			cards_E[w++] = i;
		else if (cards[i] == 2)
			cards_S[e++] = i;
		else if (cards[i] == 3)
			cards_W[r++] = i;
		else
		continue;
	}
	
	//把4家手牌对应图像加载到数组中
	/*
	for (int i = 0, q = 0, w = 0, e = 0, r = 0; i < 13; i++)
	{
	cards_N_img[i] = cards_img[cards_N[q++]];
	cards_E_img[i] = cards_img[cards_E[w++]];
	cards_S_img[i] = cards_img[cards_S[e++]];
	cards_W_img[i] = cards_img[cards_W[r++]];
	}
	//显示发牌结果
	for (int i = 0, n = 0, e = 0, w = 0; i < 13; i++)
	{
	putimage(180 + (w++) * 7, 40, &cards_N_img[i]);
	putimage(700, 40 + (w++) * 7, &cards_E_img[i]);
	putimage(180 + (w++) * 7, 460, &cards_S_img[i]);
	putimage(40, 40 + (w++) * 7, &cards_W_img[i]);
	}
	*/
		
	for (int cardsnum = 0, w = 0; cardsnum < 13; cardsnum++)
	{
		if (cards_N[cardsnum] == -1)
			break;
		else
		{
			int card = cards_N[cardsnum];
			putimage(180 + (w++) * 28, 40, &cards_img[card]);
		}
	}

	for (int cardsnum = 0, w = 0; cardsnum < 13; cardsnum++)
	{
		if (cards_E[cardsnum] == -1)
			break;
		else
		{
			int card = cards_E[cardsnum];
			putimage(700, 40 + (w++) * 28, &cards_img[card]);
		}
	}

	for (int cardsnum = 0, w = 0; cardsnum < 13; cardsnum++)
	{
		if (cards_S[cardsnum] == -1)
			break;
		else
		{
			int card = cards_S[cardsnum];
			putimage(180 + (w++) * 28, 460, &cards_img[card]);
		}
	}

	for (int cardsnum = 0, w = 0; cardsnum < 13; cardsnum++)
	{
		if (cards_W[cardsnum] == -1)
			break;
		else
		{
			int card = cards_W[cardsnum];
			putimage(40, 40 + (w++) * 28, &cards_img[card]);
		}
	}
	return;
}
void CardsImg::DrawPlayed(int leader, int cards[4])
{
	int player = leader;
	for(int i=0;i<4;i++)
	{
		if (cards[i] == -1)
			break;
		int card = cards[i];
		switch (player)
		{
		case 0:
			putimage(370, 200, &this->cards_img[card]);
			break;
		case 1:
			putimage(450, 260, &this->cards_img[card]);
			break;
		case 2:
			putimage(370, 300, &this->cards_img[card]);
			break;
		case 3:
			putimage(290, 260, &this->cards_img[card]);
			break;
		default:
			continue;
		}
		player = (player + 1 == 4) ? 0 : player + 1;
	}
	return;
}
void CardsImg::DrawBided(int  bidcount, int bidRecoder[100])
{
	for (int i = 0, q = 0, w = 0, e = 0, r = 0; i<bidcount; i++)
	{
		IMAGE pass;
		loadimage(&pass, _T("./bids/01.jpg"));
		if (bidRecoder[i] / 100 == 0)
		{
			if (bidRecoder[i] % 100 == 0)
				putimage(180 + (q++) * 40, 0, &pass);
			else
				putimage(180 + (q++) * 40, 0, &bid_img[bidRecoder[i] % 100]);
		}
		else
			if (bidRecoder[i] / 100 == 1)
			{
				if (bidRecoder[i] % 100 == 0)
					putimage(760, 40 + (w++) * 40, &pass);
				else
					putimage(760, 40 + (w++) * 40, &bid_img[bidRecoder[i] % 100]);
			}
			else
				if (bidRecoder[i] / 100 == 2)
				{
					if (bidRecoder[i] % 100 == 0)
						putimage(180 + (e++) * 40, 540, &pass);
					else
						putimage(180 + (e++) * 40, 540, &bid_img[bidRecoder[i] % 100]);
				}
				else
					if (bidRecoder[i] / 100 == 3)
					{
						if (bidRecoder[i] % 100 == 0)
							putimage(0, 40 + (r++) * 40, &pass);
						else
							putimage(0, 40 + (r++) * 40, &bid_img[bidRecoder[i] % 100]);
					}
	}
}
void CardsImg::DrawContract(int  bidcount, int bidRecoder[100])
{
	IMAGE pass;
	loadimage(&pass, _T("./bids/01.jpg"));
	if (bidRecoder[bidcount-3] / 100 == 0)
	{
		if (bidRecoder[bidcount - 3] % 100 == 0)
				putimage(180 , 0, &pass);
		else
			putimage(180 , 0, &bid_img[bidRecoder[bidcount - 3] % 100]);
	}
	else
		if (bidRecoder[bidcount - 3] / 100 == 1)
		{
			if (bidRecoder[bidcount - 3] % 100 == 0)
				putimage(760, 40 , &pass);
			else
				putimage(760, 40 , &bid_img[bidRecoder[bidcount - 3] % 100]);
		}
		else
			if (bidRecoder[bidcount - 3] / 100 == 2)
			{
				if (bidRecoder[bidcount - 3] % 100 == 0)
					putimage(180 , 540, &pass);
				else
					putimage(180 , 540, &bid_img[bidRecoder[bidcount - 3] % 100]);
			}
			else
				if (bidRecoder[bidcount - 3] / 100 == 3)
				{
					if (bidRecoder[bidcount - 3] % 100 == 0)
						putimage(0, 40 , &pass);
					else
						putimage(0, 40 , &bid_img[bidRecoder[bidcount - 3] % 100]);
				}
	
}
void CardsImg::DrawTrick(int trick[4])
{
	TCHAR NS_Trick[2];
	TCHAR WE_Trick[2];
	int s[2] = { trick[0] + trick[2] ,trick[1] + trick[3] };
	RECT r = { 540, 500, 650, 640 };
	drawtext(_T("NS方赢墩:"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	swprintf_s(NS_Trick, _T("%d"), s[0]);
	outtextxy(640, 560, NS_Trick);
	RECT x = { 540, 500, 900, 640 };
	drawtext(_T("WE方赢墩:"), &x, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	swprintf_s(WE_Trick, _T("%d"), s[1]);
	outtextxy(770, 560, WE_Trick);
	
}










//一大堆乱七八糟的测试代码
//////////////////////////pipe 功能测试
/*
int info[8] = { 1,1,6,1200,1,8,15,1 };
int deal[13] = {51,50,49,48,47,46,45,44,43,42,41,40,39};
int deald[13] = { 0,1,2,3,4,5,6,7,8,9,10,11,12 };
ai.UploadInfo(info);
ai.UploadDeal(0, deal);
ai.UploadBid(300);
int bid = ai.InquireBid();
//printf("这个诡异的数字是%d", bid);
ai.UploadContover(bid*10+0*1000);
ai.UploadPlay(1, 13);
ai.UploadDummy(2, deald);
int card=ai.InquirePlay(2);
ai.UploadPlay(2, card);
ai.UploadPlay(3, 17);
ai.InquirePlay(0);
ai.UploadOverMsg(13);
*/
////////////////////////////////
/*
cin.get();
return 0;
int seed = 123;
int process = 0;
int cards[52];
//game.GetCards(cards);


*/
///////////////////////主逻辑示范
/*
while(1)
{
int process = 0;
process = game.WhatToDo();
printf("%2d", process);

int num = 0;
int result = -1;
int score = 0;
switch (process / 10)
{
case 0:
printf("发牌\n");
game.DEAL(seed);
break;
case 1:
printf("叫牌\n");
for (int i = 1; i < 8; i++)
for (int j = 1; j < 6; j++)
avalibleBid[i - 1][j - 1] = i * 10 + j;
avalibleBid[7][0] = 80;
avalibleBid[7][1] = 90;
printf("当前%d\n", game.nowBid);
for (int i = 0; i < (game.nowBid / 10 % 10 - 1); i++)
for (int j = 0; j < 5; j++)
avalibleBid[i][j] = -1;//划掉不能叫的行
for (int i = 0; i < (game.nowBid % 10); i++)
avalibleBid[game.nowBid / 10 % 10 - 1][i] = -1;//划掉不能叫的列
result=MakeBid(process%10,avalibleBid[0]);
game.BID(process % 10, result / 10, result % 10);
//game.gameStatus = 2;
break;
case 2:
printf("打牌\n");

for (int i = 0; i < 13; i++)
avalibleCards[i] = -1;
for (int i = 0; i < 52; i++)
{
if (game.cardPosition[0][i] == process % 10 && game.cardPosition[1][i] == 1)
{
avalibleCards[num] = i;
num++;
}
}
result =ChoseCardToPlay(process % 10, avalibleCards);
game.PLAY(process % 10, result / 4 + 2, result % 4);
break;
case 3:
//printf("结算\n");
score = game.GetScore();
printf("score:%d", score);
break;
}

}

/*
char buffer[256];
ifstream configFile("./config.txt");
if (!configFile.is_open())
{
MessageBox(NULL, TEXT("Can't open config.txt!"), TEXT("Error"), MB_OK);
}
else
{
while (!configFile.eof())
{
configFile.getline(buffer, 100);
switch (buffer[1])
{
case 'V':
sscanf_s(buffer,"EVENTNAME=%s", game.eventName);
printf("%s\n", game.eventName);
break;
case 'S':
sscanf_s(buffer,"NSNAME=%s", game.NSName);
printf("%s\n", game.NSName);
break;
case 'W':
sscanf_s(buffer,"EWNAME=%s", game.NSName);
printf("%s\n", game.EWName);
break;
case 'I':
sscanf_s(buffer,"AIPOSITION=%d", &game.AIPosition);
printf("%d\n", game.AIPosition);
break;
case 'E':
sscanf_s(buffer,"SEED=%d", &game.seed);
printf("%d\n", game.seed);
break;
case 'B':
sscanf_s(buffer, "PBNFILE=%s", game.pbnName);
printf("%s\n", game.pbnName);
break;
default:
break;
}
//cout << buffer << endl;
}
}
*/
//cin.get();
//game.BID(1, 0, 0);
/*测试叫牌

int a, b, c;
while (1)
{
if(game.gameStatus!=1)
printf("庄家是%d\n", game.banker);
if (game.gameStatus == 2)
printf("定约是%d\n", game.nowBid);
printf("现在是%d阶段\n", game.gameStatus);
printf("PASS计数%d\n", game.passCount);
printf("轮到%d了\n", game.nowPlayerPosition);
printf("方位 阶数 花色\n");
scanf_s("%d%d%d", &a, &b, &c);
game.BID(a, b, c);
}
*/
/*测试发牌
game.DEAL(123123);
*/
//game.PLAY(0, 0, 0);

/*测试打牌
game.trump = 5;
game.contover = 1;
game.gameStatus = 2;
game.cardPosition[0][48] = 0;
//假设打1nt
int a, b, c;
while (1)
{
printf("现在是%d阶段\n", game.gameStatus);
printf("轮到%d了\n", game.nowPlayerPosition);
printf("方位 牌点 花色\n");
scanf_s("%d%d%d", &a, &b, &c);
game.PLAY(a, b, c);
}
*/
//测试分数
/*
game.gameStatus = 3;
game.banker = 1;
game.dummy = 2;
game.trump = 3;
game.contover = 1;
game.trick[game.banker] = 3;
game.trick[game.dummy] = 3;
game.GetScore();
printf("score:%d", game.score);
printf("game status %d", game.gameStatus);
*/
