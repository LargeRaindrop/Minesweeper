#define random(m,n) rand()%(n-m+1)+m
#define VICTORY (numOfExposedGrid+numOfMine==width*height)

#include<iostream>
#include<windows.h>
#include<conio.h>
#include<cstdlib>
#include<ctime>
#include<queue>
//#include<cctype>
using namespace std;

const int MAX_WIDTH=30,MAX_HEIGHT=24;

HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE),
	   hIn=GetStdHandle(STD_INPUT_HANDLE);
INPUT_RECORD eventMsg;
DWORD Pointer;
int width,height,numOfMine,numOfLeftMine,numOfExposedGrid,offset[4]={0,-1,0,1};
bool gameOver,gameStart,bothPress;
struct grid
{
	bool isMine,isClicked;
	int numOfMineNearby,status;//status: 0-primitive 1-uncovered 2-markedAsMine 3-questionMark
}map[MAX_HEIGHT+1][MAX_WIDTH+1];

void getMouseInfo(COORD &site,int &status);
void homepage();
void read(int &x);
bool charInString(string x);
bool startGame();
void initialize();
void printMap();
void resetConsoleMode();
void generateMap(COORD clickSite);
void clickDiffusion(COORD clickSite);

int main()
{
	system("title 扫雷1.0 by TinyRaindrop");
	while(true)
	{
		homepage();
		while(startGame());
	}
	return 0;
}
void getMouseInfo(COORD &site,int &status)
{
	//site: 00-blankSite 01-← 10-☆ default-mapSite 
	//status: 0-loose 1-left 2-right 3-both
	COORD stdCoord;
	while(true)
	{
		ReadConsoleInput(hIn,&eventMsg,1,&Pointer);
		if (eventMsg.EventType==MOUSE_EVENT)
		{
			stdCoord=eventMsg.Event.MouseEvent.dwMousePosition;
			if (stdCoord.Y==0)
			{
				if (stdCoord.X==2*width-4||stdCoord.X==2*width-3)
				{
					site.X=0;
					site.Y=1;
				}
				else if (stdCoord.X==2*width-2||stdCoord.X==2*width-1)
				{
					site.X=1;
					site.Y=0;
				}
				else
				{
					site.X=0;
					site.Y=0;
				}
			}
			else if (stdCoord.X>=0&&stdCoord.X<2*width&&stdCoord.Y>=4&&stdCoord.Y<=3+height)
			{
				site.X=stdCoord.X/2+1;
				site.Y=stdCoord.Y-3;
			}
			else
			{
				site.X=0;
				site.Y=0;
			}
			switch(eventMsg.Event.MouseEvent.dwButtonState)
			{
				case 0: 
					status=0; 
					bothPress=false;
					break;
				case 1:
					status=1; 
					break;
				case 2: 
					status=2; 
					break;
				case 3: 
					status=3;
					bothPress=true;
					break;
				default: 
					status=0; 
					break;
			}
			if (!(bothPress&&status!=3)) break;
		}
	}
//	cout<<"here";
	return;
}
void homepage()
{
	int cmd,maxNumOfMine;
	char input;
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED);
	system("cls");
	resetConsoleMode();
//	system("color 07");
	cout<<"欢迎来到扫雷1.0！\n"
		<<"作者：TinyRaindrop\n"
		<<endl
		<<"请输入数字选择难度：\n";
	SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_INTENSITY);
	cout<<"[1] 简单\n";
	SetConsoleTextAttribute(hOut,BACKGROUND_BLUE|BACKGROUND_INTENSITY);
	cout<<"[2] 一般\n";
	SetConsoleTextAttribute(hOut,BACKGROUND_RED|BACKGROUND_INTENSITY);
	cout<<"[3] 困难\n";
	SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_RED|BACKGROUND_INTENSITY);
	cout<<"[4] 自定义\n";
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED);
	do
	{
		input=getch();
		cmd=input-'0';
	}while(cmd<1||cmd>4);
	switch(cmd)
	{
		case 1:
			width=9;
			height=9;
			numOfMine=10;
			break;
		case 2:
			width=16;
			height=16;
			numOfMine=40;
			break;
		case 3:
			width=30;
			height=16;
			numOfMine=99;
			break;
		case 4:
			cout<<"请输入宽度、高度和雷数:\n"
				<<"宽度（9~30）：";
			read(width);
			while(width<9||width>30)
			{
				cout<<"请输入位于9~30的整数！\n";
				read(width);
			}
			cout<<"高度（9~24）：";
			read(height);
			while(height<9||height>24)
			{
				cout<<"请输入位于9~24的整数！\n";
				read(height);
			}
			maxNumOfMine=(width-1)*(height-1);
			cout<<"雷数（10~"<<maxNumOfMine<<"）：";
			read(numOfMine);
			while(numOfMine<10||numOfMine>maxNumOfMine)
			{
				cout<<"请输入位于10~"<<maxNumOfMine<<"的整数！\n";
				read(numOfMine);
			}
			break;
	}
	return;
}
void read(int &x)
{
	string input;
	getline(cin,input);
	while(charInString(input))
	{
		cout<<"输入中含有非法字符，请重新输入！\n";
		getline(cin,input);
	}
	x=0;
	for(int i=0;i<input.size();i++) x=(x*10)+input[i]-'0';
	return;
}
bool charInString(string x)
{
	for(int i=0;i<x.size();i++)
		if (!isdigit(x[i]))
			return true;
	return false;
}
bool startGame()
{
//	int cnt=0;
	COORD dstSite,mouseSite,preMouseSite={0,0};//上一次无操作 
	int mouseStatus,preMouseStatus=0;
	bool rePrintMap;
	initialize();
	while(true)
	{
		printMap();
		rePrintMap=false;
		while(true)
		{
			getMouseInfo(mouseSite,mouseStatus);
			/*
			SetConsoleTextAttribute(hOut,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
			dstSite.X=0; dstSite.Y=4+height;
			SetConsoleCursorPosition(hOut,dstSite);
			cout<<"Now: "<<mouseStatus<<" at "<<mouseSite.X<<" , "<<mouseSite.Y<<"     \n"
				<<"Pre: "<<preMouseStatus<<" at "<<preMouseSite.X<<" , "<<preMouseSite.Y<<"    \n";
			*/
			//消除上次操作 
			if (preMouseSite.X==0&&preMouseSite.Y==1)
			{
//				cout<<++cnt;
				dstSite.X=2*width-4; dstSite.Y=0;
				SetConsoleCursorPosition(hOut,dstSite);
				SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE|
								 			 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
				cout<<"←";
			}
			else if (preMouseSite.X==1&&preMouseSite.Y==0)
			{
				dstSite.X=2*width-2; dstSite.Y=0;
				SetConsoleCursorPosition(hOut,dstSite);
				SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE|
						 					 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
				cout<<"☆";
			}
			else if (!gameOver)
			{
				switch(preMouseStatus)
				{
					case 1:
						if (preMouseSite.X!=0&&preMouseSite.Y!=0&&map[preMouseSite.Y][preMouseSite.X].status==0)
						{
							dstSite.X=(preMouseSite.X-1)*2; dstSite.Y=preMouseSite.Y+3;
							SetConsoleCursorPosition(hOut,dstSite);
							SetConsoleTextAttribute(hOut,BACKGROUND_BLUE);
							cout<<"  ";
						}
						break;
					case 3:
						if (preMouseSite.X!=0&&preMouseSite.Y!=0)
						{
							for(int i=1;i<=3;i++)
							{
								COORD checkSite;
								checkSite.Y=preMouseSite.Y+offset[i];
								for(int j=1;j<=3;j++)
								{
									checkSite.X=preMouseSite.X+offset[j];
									if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height&&map[checkSite.Y][checkSite.X].status==0)
									{
										dstSite.X=(checkSite.X-1)*2; dstSite.Y=checkSite.Y+3;
										SetConsoleCursorPosition(hOut,dstSite);
										SetConsoleTextAttribute(hOut,BACKGROUND_BLUE);
										cout<<"  ";
									}
								}
							}
						}
						break;
				}
			}
			//本次操作 
			switch(mouseStatus)
			{
				case 0:
					if (preMouseStatus==1)
					{
						if (mouseSite.X==0&&mouseSite.Y==1) return false;
						else if (mouseSite.X==1&&mouseSite.Y==0) return true;
						else if (!gameOver&&mouseSite.X!=0&&mouseSite.Y!=0&&map[mouseSite.Y][mouseSite.X].status==0)
						{
							rePrintMap=true;
							if (!gameStart)
							{
								COORD clickSite;
								clickSite.X=mouseSite.X; clickSite.Y=mouseSite.Y;
								generateMap(clickSite);
								clickDiffusion(clickSite);
//								gameOver=true;//to see the distribution of mines
							}
							else if (map[mouseSite.Y][mouseSite.X].isMine)
							{
								map[mouseSite.Y][mouseSite.X].status=1;
								gameOver=true;
							}
							else if (!map[mouseSite.Y][mouseSite.X].isMine)
							{
								clickDiffusion(mouseSite);
								if (VICTORY) gameOver=true;
							}
						}
					}
					else if (preMouseStatus==2)
					{
						//rePrintmap=true
						if (map[mouseSite.Y][mouseSite.X].status!=1)
						{
							rePrintMap=true;
							switch(map[mouseSite.Y][mouseSite.X].status)
							{
								case 0: 
									map[mouseSite.Y][mouseSite.X].status=2;
									numOfLeftMine--;
									break;
								case 2: 
									map[mouseSite.Y][mouseSite.X].status=3; 
									numOfLeftMine++;
									break;
								case 3:
									map[mouseSite.Y][mouseSite.X].status=0; 
									break;
							}
						}
					}
					else if (preMouseStatus==3)
					{
						if (!map[mouseSite.Y][mouseSite.X].isMine&&map[mouseSite.Y][mouseSite.X].numOfMineNearby!=0&&map[mouseSite.Y][mouseSite.X].status==1)
						{
							COORD checkSite;
							int numOfMarkedMine=0;
							bool hiddenMineExist=false;
							for(int i=1;i<=3;i++)
							{
								checkSite.Y=mouseSite.Y+offset[i];
								for(int j=1;j<=3;j++)
								{
									checkSite.X=mouseSite.X+offset[j];
//									if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height&&map[checkSite.Y][checkSite.X].status==2) numOfMarkedMine++;
									if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height)
									{
										if (map[checkSite.Y][checkSite.X].status==2) numOfMarkedMine++;
										if (map[checkSite.Y][checkSite.X].status==0&&map[checkSite.Y][checkSite.X].isMine) hiddenMineExist=true;
									}
								}
							}
							if (numOfMarkedMine==map[mouseSite.Y][mouseSite.X].numOfMineNearby)
							{
								rePrintMap=true;
								for(int i=1;i<=3;i++)
								{
									checkSite.Y=mouseSite.Y+offset[i];
									for(int j=1;j<=3;j++)
									{
										if (i==2&&j==2) continue;
										checkSite.X=mouseSite.X+offset[j];
										if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height&&map[checkSite.Y][checkSite.X].status==0)
										{
											if (map[checkSite.Y][checkSite.X].isMine)
											{
												map[checkSite.Y][checkSite.X].status=1;
												gameOver=true;
											}
											else if (!hiddenMineExist)
											{
												clickDiffusion(checkSite);
												if (VICTORY) gameOver=true;
											}
										}
									}
								}
							}
						}
					}
					break;
				case 1:
					if (mouseSite.X==0&&mouseSite.Y==1)
					{
						dstSite.X=2*width-4; dstSite.Y=0;
						SetConsoleCursorPosition(hOut,dstSite);
						SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE|BACKGROUND_INTENSITY| 
										 			 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
						cout<<"←";
					}
					else if (mouseSite.X==1&&mouseSite.Y==0)
					{
						dstSite.X=2*width-2; dstSite.Y=0;
						SetConsoleCursorPosition(hOut,dstSite);
						SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE|BACKGROUND_INTENSITY| 
								 					 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
						cout<<"☆";
					}
					else if (!gameOver&&mouseSite.X!=0&&mouseSite.Y!=0&&map[mouseSite.Y][mouseSite.X].status==0)
					{
						dstSite.X=(mouseSite.X-1)*2; dstSite.Y=mouseSite.Y+3;
						SetConsoleCursorPosition(hOut,dstSite);
						SetConsoleTextAttribute(hOut,BACKGROUND_GREEN);
						cout<<"  ";
					}
					break;
				case 3:
					if (!gameOver&&mouseSite.X!=0&&mouseSite.Y!=0)
					{
						for(int i=1;i<=3;i++)
						{
							COORD checkSite;
							checkSite.Y=mouseSite.Y+offset[i];
							for(int j=1;j<=3;j++)
							{
								checkSite.X=mouseSite.X+offset[j];
								if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height&&map[checkSite.Y][checkSite.X].status==0)
								{
									dstSite.X=(checkSite.X-1)*2; dstSite.Y=checkSite.Y+3;
									SetConsoleCursorPosition(hOut,dstSite);
									SetConsoleTextAttribute(hOut,BACKGROUND_GREEN);
									cout<<"  ";
								}
							}
						}
					}
					break;
			}
			preMouseSite=mouseSite;
			preMouseStatus=mouseStatus;
			if (rePrintMap) break;
		}
	}
}
void initialize()
{
	numOfLeftMine=numOfMine;
	numOfExposedGrid=0;
	bothPress=false;
	gameOver=false;
	gameStart=false;
	for(int i=1;i<=height;i++)
		for(int j=1;j<=width;j++)
		{
			map[i][j].isMine=false;
			map[i][j].numOfMineNearby=0;
			map[i][j].status=0;
			map[i][j].isClicked=false;
		}
	return;
}
void printMap()
{
	int tmp=numOfLeftMine,numOfSpace=width*2-14;//numOfSpace-剩余雷数和返回按钮之间的空格数 
	if (tmp==0) numOfSpace--;
	else
	{
		if (tmp<0) numOfSpace--;
		while(tmp)
		{
			numOfSpace--;
			tmp/=10;
		}
	}
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED);
	system("cls");
	resetConsoleMode();
	cout<<"剩余雷数："<<numOfLeftMine;
	for(int i=1;i<=numOfSpace;i++) cout<<" ";
	SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE|
								 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
	cout<<"←☆\n\n";
	if (gameOver)
	{
		int numOfSpace;
		if (VICTORY)
		{
			numOfSpace=(width*2-7)/2;
			SetConsoleTextAttribute(hOut,FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
			for(int i=1;i<=numOfSpace;i++) cout<<" ";
			SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY|
									 	 FOREGROUND_RED|FOREGROUND_INTENSITY);
			cout<<"VICTORY";
			SetConsoleTextAttribute(hOut,FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
			for(int i=1;i<=width-numOfSpace-7;i++) cout<<" ";
		}
		else
		{
			numOfSpace=(width*2-6)/2;
			SetConsoleTextAttribute(hOut,FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
			for(int i=1;i<=numOfSpace;i++) cout<<" ";
			SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY|
									 	 FOREGROUND_RED|FOREGROUND_INTENSITY);
			cout<<"DEFEAT";
			SetConsoleTextAttribute(hOut,FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
			for(int i=1;i<=width-numOfSpace-6;i++) cout<<" ";
		}
	}
	cout<<"\n\n";
	for(int i=1;i<=height;i++)
	{
		for(int j=1;j<=width;j++)
		{
			if (gameOver)
			{
				switch(map[i][j].status)
				{
					case 0:
						if (!map[i][j].isMine)
						{
							SetConsoleTextAttribute(hOut,BACKGROUND_BLUE);
							cout<<"  ";
						}
						else
						{
							if (!VICTORY)
								SetConsoleTextAttribute(hOut,BACKGROUND_RED|
														 	 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							else
								SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_INTENSITY|
														 	 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							cout<<"¤"; 
						}
						break;
					case 1:
						if (!map[i][j].isMine)
						{
							SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|
														 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							switch(map[i][j].numOfMineNearby)
							{
								case 0: cout<<"  "; break;
								case 1: cout<<"①"; break;
								case 2: cout<<"②"; break;
								case 3: cout<<"③"; break;
								case 4: cout<<"④"; break;
								case 5: cout<<"⑤"; break;
								case 6: cout<<"⑥"; break;
								case 7: cout<<"⑦"; break;
								case 8: cout<<"⑧"; break;
							}
						}
						else
						{
							SetConsoleTextAttribute(hOut,BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_INTENSITY|
														 FOREGROUND_RED|FOREGROUND_INTENSITY);
							cout<<"¤";
						}
						break;
					case 2:
						if (!map[i][j].isMine)
						{
							SetConsoleTextAttribute(hOut,BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_INTENSITY|
														 FOREGROUND_RED|FOREGROUND_INTENSITY);
							cout<<"×";
						}
						else
						{
							if (!VICTORY)
								SetConsoleTextAttribute(hOut,BACKGROUND_RED|
														 	 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							else
								SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_INTENSITY|
														 	 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							cout<<"▲";
						}
						break;
					case 3:
						if (!map[i][j].isMine)
						{
							SetConsoleTextAttribute(hOut,BACKGROUND_BLUE);
							cout<<"  ";
						}
						else
						{
							if (!VICTORY)
								SetConsoleTextAttribute(hOut,BACKGROUND_RED|
														 	 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							else
								SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|BACKGROUND_INTENSITY|
														 	 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							cout<<"¤"; 
						}
						break; 
				}
			}
			else
			{
				switch(map[i][j].status)
				{
					case 0:
						SetConsoleTextAttribute(hOut,BACKGROUND_BLUE);
						cout<<"  ";
						break;
					case 1:
						if (!map[i][j].isMine)
						{
							SetConsoleTextAttribute(hOut,BACKGROUND_GREEN|
														 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
							switch(map[i][j].numOfMineNearby)
							{
								case 0: cout<<"  "; break;
								case 1: cout<<"①"; break;
								case 2: cout<<"②"; break;
								case 3: cout<<"③"; break;
								case 4: cout<<"④"; break;
								case 5: cout<<"⑤"; break;
								case 6: cout<<"⑥"; break;
								case 7: cout<<"⑦"; break;
								case 8: cout<<"⑧"; break;
							}
						}
						else cout<<"GG";//防止出现bug 
						break;
					case 2:
						SetConsoleTextAttribute(hOut,BACKGROUND_BLUE|
													 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
						cout<<"▲";
						break;
					case 3:
						SetConsoleTextAttribute(hOut,BACKGROUND_BLUE|
													 FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
						cout<<"？";
						break;
				}
			}
		}
		cout<<endl;
	}
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED);
	/*
	COORD dstSite;
	dstSite.X=0; dstSite.Y=4+height;
	SetConsoleCursorPosition(hOut,dstSite);
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
	cout<<numOfExposedGrid<<"    ";
	*/
	return;
}
void resetConsoleMode()
{
	GetConsoleMode(hIn,&Pointer); //initialize
	Pointer|=ENABLE_MOUSE_INPUT;
	SetConsoleMode(hIn,Pointer);
	return;
}
void generateMap(COORD clickSite)
{
	COORD checkSite,mineSite;
	gameStart=true;
	for(int i=1;i<=3;i++)
	{
		checkSite.X=clickSite.X+offset[i];
		for(int j=1;j<=3;j++)
		{
			checkSite.Y=clickSite.Y+offset[j];
			if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height) map[checkSite.Y][checkSite.X].isClicked=true;
		}
	}
	srand(time(0));
	for(int i=1;i<=numOfMine;i++)
	{
		while(true)
		{
			mineSite.X=random(1,width);
			mineSite.Y=random(1,height);
			if (!map[mineSite.Y][mineSite.X].isMine&&!map[mineSite.Y][mineSite.X].isClicked)
			{
				map[mineSite.Y][mineSite.X].isMine=true;
				break;
			}
		}
	}
	for(int i=1;i<=height;i++)
		for(int j=1;j<=width;j++)
		{
			if (map[i][j].isMine)
			{
				for(int k=1;k<=3;k++)
				{
					checkSite.Y=i+offset[k];
					for(int l=1;l<=3;l++)
					{
						checkSite.X=j+offset[l];
						if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height) map[checkSite.Y][checkSite.X].numOfMineNearby++;
					}
				}
			}
		}
	return;
}
void clickDiffusion(COORD clickSite)
{
	COORD now,checkSite;
	queue<COORD> que;
	int queSize;
	que.push(clickSite);
	map[clickSite.Y][clickSite.X].status=1;
	numOfExposedGrid++;
	while(!que.empty())
	{
		queSize=que.size();
		for(int i=1;i<=que.size();i++)
		{
			now=que.front();
			que.pop();
//			numOfExposedGrid++;
//			map[now.Y][now.X].status=1;
			if (map[now.Y][now.X].numOfMineNearby==0)
			{
				for(int j=1;j<=3;j++)
				{
					checkSite.Y=now.Y+offset[j];
					for(int k=1;k<=3;k++)
					{
						checkSite.X=now.X+offset[k];
						if (checkSite.X>0&&checkSite.X<=width&&checkSite.Y>0&&checkSite.Y<=height&&map[checkSite.Y][checkSite.X].status==0&&!map[checkSite.Y][checkSite.X].isMine)
						{
							que.push(checkSite);
							numOfExposedGrid++;
							map[checkSite.Y][checkSite.X].status=1;
						}
					}
				}
			}
		}
	}
	return;
}

