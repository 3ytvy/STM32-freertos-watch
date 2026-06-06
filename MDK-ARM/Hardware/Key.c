#include "stm32f1xx_hal.h"

extern uint8_t key_open_flag;/*引自main.c*/

uint8_t Key_Num;
int press_time;

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
	*	说		明：CUBEMX已自动生成引脚初始化函数
  */
//void Key_Init(void)
//{
//	/*开启时钟*/
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
//	
//	/*GPIO初始化*/
//	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PB1引脚初始化为上拉输入

//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_6;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

//}





//函数：检测PB1、PA6、PA4的端口，返回对应的状态值
//若PB1为低电平（被按下），函数返回1
//若PA6为低电平（被按下），函数返回2
//若PA4为低电平（被按下），函数返回3
//否则，函数返回0
//这个函数就是去获取那3个按钮是哪个按钮被按下了，然后返回对应的值
uint8_t Key_GetState(void)
{
	
	//用if...else if...有顺序地互斥地检查按键

	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==0)
	{
		return 1;
	}
	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)==0)
	{
		return 2;
	}
//////////	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0 && press_time>1000)
//////////	{
//////////		return 4;
//////////	}
	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0)
	{
		return 3;
	}
	
	else
	{
		return 0;
	}
}

void Key_Tick(void)
{
	static uint8_t Count;//这个是用来计数的，这个是在这个只在函数里的变量（反复调用时不会重新定义），只有在第一次进入函数时会初始化，多次调用此函数时，它的值会在多次调用之间保持、累加
	static uint8_t CurrentState,PreState;
	//PreState是Previous，是过去的状态
	//Current是现在的状态
	//这个状态是：那3个按钮是哪个按钮被按下了，如果是按键1，那就是1，如果是按键2，那就是2，如果是按键3，那就是3,如果都没，就是0
	Count++;
	if(Count>=20)
	{
		Count=0;
		
		PreState=CurrentState;
		CurrentState = Key_GetState();//注意这里调用了Key_GetState()函数
		//这句代码就是，把获取到的那3个按钮是哪个按钮被按着了的返回值赋给CurrentState
		//所以CurrentState的意思是：现在那3个按钮是哪个按钮被按下了

		//如果PreState（刚刚）是那3个按钮中有一个按钮被按着，这个值在PreState里
		//并且现在CurrentState==0，就是说3个按钮都松开了
		//这个状态就表示：刚刚有一个按钮被按着并且被松开
		//这里这种，直接拿引脚来判断3个按钮是哪个按钮被按下，所以只有“按着”（包括一直按着）和“松开”（也就是没有按着）两个状态
		//这里的思路是，把这个函数丢进TIM2的中断服务子程序里
		//也就是每1ms执行一次这个函数，每1ms判断一下有没有出现“刚刚按下，现在松开”的状态，如果有，就是这个按钮被按下
		if(PreState != 0 && CurrentState == 0)
		{
			Key_Num = PreState;//把PreState（刚刚）那3个按钮中是哪一个按钮被按下返回给Key_Num
		}
	}
}

//这个函数用于把Key_Num返回去给别的地方（因为TIM2被开启，每1msKey_Num就会刷新一次，指示那个按钮被按下了）
//函数：按键获取键码（当按键（任一）被按下时，此函数返回Key_Num，Key_Num（1、2、3）反应是哪个按键被按下，否则返回0
uint8_t Key_GetNum(void)
{
	uint8_t Temp;
	if(Key_Num)//只有Key_Num>=1时候才执行
	{
		Temp=Key_Num;
		Key_Num=0;
		return Temp;
	}
	else
	{
		return 0;
	}
}



//检测Key3是否被长按的函数
void Key3_Tick(void)
{
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4) == 0)//如果Key3被按着
	{
		press_time++;
	}
	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==1)//一旦松开
	{
		press_time = 0;
	}
}
