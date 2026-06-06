#include "stm32f1xx_hal.h"
#include "OLED.h"
#include "Key.h"
#include "stdlib.h"
#include "math.h"

int Score;//存放游戏分数的变量
uint16_t Ground_Pos;
uint16_t Barrier_Pos;
uint16_t Cloud_Pos;
uint16_t jump_t;
uint8_t Jump_Pos;

extern uint8_t game_Flag;

extern uint8_t time_count_flag;/*引自main.c*/
extern uint32_t time_count_ms;/*引自main.c*/
extern uint32_t time_count_s;/*引自main.c*/
extern uint8_t now_flag;/*引自main.c*/
extern uint8_t key_irq;/*引自main.c*/

double pi = 3.141592653589793;

struct Object_Position{//定义一个存放物体的minx,maxx,miny,maxy的结构体Object_Position
	uint8_t minX,maxX,minY,maxY;
};


//显示分数的函数
void Show_Score(void)
{
	OLED_ShowNum (80,0,Score,8,OLED_6X8);

}




//地面移动动画函数
//要求：地面每隔20ms向左移动1个像素点
void Show_Gruond(void)
{
	if(Ground_Pos<128)
	{
		for(uint8_t i=0;i<128;i++)//这个for循环只不过是一个遍历
		//其作用就是把提供出来的128列放到显存函数里去显示
		{
			OLED_DisplayBuf[7][i]=Ground[i+Ground_Pos];
		}
	}
	else
	{
		//分两次for循环遍历
		//第一次遍历取 第二批128列里的
		//从0取到255-Ground_Pos
		//第二次遍历取 第一批128列里的
		//从255-Ground_Pos取到128
		
		//两次遍历共取得128列
		
		for(uint8_t i=0;i<255-Ground_Pos;i++)
		//举例：Ground_Pos=200（这个节点瞬间）
		//i取值就是 0到255-200=50 即 0到55
		//即从Ground[200]起往后取55列
		//Ground[200]到Ground[255]，就是第二批的73到128（共55个）
		{
			OLED_DisplayBuf[7][i]=Ground[i+Ground_Pos];
		}
		for(uint8_t i=255-Ground_Pos;i<128;i++)
		//举例：Ground_Pos=200（这个节点瞬间）
		//i取值就是 255-200=50到128 即 55到128
		//然后 (255-Ground_Pos) 这个值就是 55 ，也就是刚刚在第二组里取过了55列
		//那么全部要128，那就还剩128-55=73列
		//就要从第一批里的0取到73
		//i取值就是 255-200=50到128 即 55到128
		//所以有公式 [i-(255-Ground_Pos)] 就是 0到73
		//Ground[0]到Ground[73]，就是第一批的0到73
		{
			OLED_DisplayBuf[7][i]=Ground[i-(255-Ground_Pos)];

		}
	}
}

//显示障碍物的动画函数
uint8_t barrier_flag;//定义一个指向Barrier数组索引值的标志位
struct Object_Position barrier;
//这个结构体变量barrier存放障碍物的坐标，并且在Show_Barrier（运动函数）中赋值，使其值随着运动实时更新
void Show_Barrier(void)
{
	if(Barrier_Pos>=143)//当前一个障碍物完全左移到左边界面外时取一次随机数
	{
		barrier_flag=rand()%3;//取随机数对3余，得到0,1,2三者随机一个数
	}
	OLED_ShowImage(127-Barrier_Pos,44,16,18,Barrier[barrier_flag]);
	
	barrier.minX=127-Barrier_Pos;
	barrier.maxX=barrier.minX+16;//障碍物宽16个像素
	barrier.minY=44;//固定的最顶端
	barrier.maxY=62;//固定的最底端
}




//显示云朵的动画函数
//目的：云朵每50ms向左移动一格
void Show_Cloud(void)
{
	OLED_ShowImage(127-Cloud_Pos,11,16,8,Cloud);
}



//显示小恐龙的函数
uint8_t dino_jump_flag=0;//这个是 是否跳跃 的标志位，0：奔跑，1：跳跃
//要求：小恐龙的前后脚每50ms切换一次
//刚好Cloud_Pos是每50ms +1
extern uint8_t KeyNum;//把KeyNum给引进

struct Object_Position dino;
//这个结构体变量dino存放小恐龙的坐标，并且在Show_Dino（运动函数）中赋值，使其值随着运动实时更新

void Show_Dino(void)
{	
	KeyNum=Key_GetNum();//获取键值
		if(KeyNum !=0 && now_flag == 1)
		{
			key_irq = 1;
		}
		//判断按下哪个键（在亮屏情况下）
		if(KeyNum != 0 && now_flag == 0)
	{
	//亮屏计时手动复零
	time_count_ms = 0;
	time_count_s = 0;
	if(KeyNum==1){dino_jump_flag=1;}
	else if(KeyNum==2){dino_jump_flag=1;}
	else if(KeyNum==3){dino_jump_flag=1;}
	}
	Jump_Pos=28*sin((float)(pi*jump_t/1000));//这个是速度公式，其实也就是步长
	
	if(dino_jump_flag==0)//没有跳跃，一直切换前后脚
	{
		//通过Cloud_Pos每50ms变一次奇偶的特性
		if(Cloud_Pos%2==0)//如果是偶数
		{OLED_ShowImage(0,44,16,18,Dino[0]);}
		else//如果是奇数
		{OLED_ShowImage(0,44,16,18,Dino[1]);}
	}
	else//跳跃，按正弦函数的速度来跳跃
	{
		OLED_ShowImage(0,44-Jump_Pos,16,18,Dino[2]);
	}
	
	dino.minX=0;//最左端固定
	dino.maxX=16;//最右端固定，小恐龙宽16个像素
	dino.minY=44-Jump_Pos;//小恐龙的最顶端
	dino.maxY=dino.minY+16;//小恐龙的最底端，小恐龙高16个像素
}


//碰撞检测函数
int isColliding(struct Object_Position *a,struct Object_Position *b)
{
	if((a->maxX > b->minX) && (a->maxY > b->minY) && (a->minX < b->maxX) && (a->minY < b->maxY))
		//其实条件是（(a->maxX > b->minX) && (a->maxY > b->minY)就可以的了）
	{
		OLED_Clear();
		OLED_ShowString(28,24,"Game Over",OLED_8X16);
		OLED_Update();
		HAL_Delay(600);
		OLED_ShowChinese(8,24,"得分：");
		OLED_Printf(56,24,OLED_8X16,"%08d",Score);
		OLED_Update();
		HAL_Delay(1000);
		Score=0;//清除数据
		
		OLED_Clear();
		OLED_Update();
		return 1;//退出标志
	}
	else{return 0;}
}


//分频函数，此函数用于在TIM2调用，是用来分频的
//再来个全局（整个项目）的判断标志位，就是之前的那个game_Flag，判断游戏是否在进行
//若在进行，就把这个分频函数丢到中断函数执行去
//若没进行，中断函数就不执行这个函数
void Dino_Tick(void)
{
	static uint8_t Score_Count,Ground_Count,Cloud_Count;
	Score_Count ++;
	Ground_Count ++;
	Cloud_Count ++;
	if(Score_Count>=100)//Score_Count满100才溢出，使达到分频的效果
	{
		Score_Count=0;
		Score++;
	}
	
	if(Ground_Count>=20)
	{
		Ground_Count=0;
		Ground_Pos++;
		Barrier_Pos++;
		if(Ground_Pos >= 256){Ground_Pos=0;}
		if(Barrier_Pos>=144){Barrier_Pos=0;}
	}
	
	if(Cloud_Count>=50)
	{
		Cloud_Count=0;
		Cloud_Pos++;
		if(Cloud_Pos>=200){Cloud_Pos=0;}//这样做，每次移出一个云朵，要等一阵才会有下一个云朵移入
	}
	
	if(dino_jump_flag==1)
	{
		jump_t ++;
		if(jump_t==1000)
		{dino_jump_flag=0;jump_t=0;}
	}
}

//整合所有动画的函数
int DinoGame_Animation(void)
{
	game_Flag=1;
	OLED_Clear();
	Show_Score();
	Show_Gruond();
	Show_Barrier();
	Show_Cloud();
	Show_Dino();
	if(isColliding(&dino,&barrier)==1){game_Flag=0;return 0;}
	OLED_Update();
}
