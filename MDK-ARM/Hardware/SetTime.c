#include "stm32f1xx_hal.h"
#include "MyRTC.h"
#include "Key.h"
#include "OLED.h"
#include "Menu.h"
#include "public.h"
//////////////#include "main.h"

/*自加*/

//#include "FreeRTOSConfig.h"
//#include "FreeRTOS.h"
//#include "cmsis_os2.h"
//#include "queue.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"

extern uint8_t time_count_flag;/*引自main.c*/
extern uint32_t time_count_ms;/*引自main.c*/
extern uint32_t time_count_s;/*引自main.c*/
extern uint8_t now_flag;/*引自main.c*/
extern uint8_t key_irq;/*引自main.c*/


extern void	My_Send_Task_Queue(uint8_t val);/*引自main.c*/


extern uint8_t KeyNum;  //extern告诉编译器这个变量已经在其他地方定义过了


/*----显示日期时间设置的UI-------------------*/

//日期时间设置第一页UI
void Show_SetTime_FirstUI(void)
{
	MyRTC_ReadTime();
	OLED_ShowImage(0,0,16,16,Return);
	OLED_ShowChinese(0,16,"年：");
	OLED_Printf(32,16,OLED_8X16,"%4d",MyRTC_Time[0]);
	OLED_ShowChinese(0,32,"月：");
	OLED_Printf(32,32,OLED_8X16,"%2d",MyRTC_Time[1]);
	OLED_ShowChinese(0,48,"日：");
	OLED_Printf(32,48,OLED_8X16,"%2d",MyRTC_Time[2]);
}

//日期时间设置第二页UI
void Show_SetTime_SecondUI(void)
{
//	OLED_Printf(0,0,OLED_8X16,"时：%2d",MyRTC_Time[3]);
//	OLED_Printf(0,16,OLED_8X16,"分：%2d",MyRTC_Time[4]);
//	OLED_Printf(0,32,OLED_8X16,"秒：%2d",MyRTC_Time[5]);
	MyRTC_ReadTime();
	OLED_ShowChinese(0,0,"时：");
	OLED_Printf(32,0,OLED_8X16,"%2d",MyRTC_Time[3]);
	OLED_ShowChinese(0,16,"分：");
	OLED_Printf(32,16,OLED_8X16,"%2d",MyRTC_Time[4]);
	OLED_ShowChinese(0,32,"秒：");
	OLED_Printf(32,32,OLED_8X16,"%2d",MyRTC_Time[5]);
}

/*------------------显示日期时间设置的UI-----*/





/*-------------------------设置时间函数-----*/

//日期时间设置的逻辑：
//右键按下（KeyNum==2）数值加1
//左键按下（KeyNum==1）数值减1
//确认键按下（KeyNum==3）确认数值并退出

//保存时间数值的思路：
//把数值写入数组MyTime[]里
//通过调用MyRTC.c里的SetTime函数，把数组MyTime[]的值写入RTC里去
//这个思路用另一个函数Change_RTC_Time（在下方）实现

//void Change_RTC_Time(uint8_t i,uint8_t flag)//flag=1:往上加1，flag=0往下减1
//{
//	if(flag==1)
//	{
//		MyRTC_Time[i]++;
//	}
//	else
//	{
//		MyRTC_Time[i]--;
//	}
//	MyRTC_SetTime();	
//}

int SetYear(void)
{
	while(1)//检测按键
	{
		KeyNum=Key_GetNum();
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
		if(KeyNum == 2)//数值加1
		{
			Change_RTC_Time(0,1);
		}
		else if(KeyNum == 1)//数值减1
		{
			Change_RTC_Time(0,0);
		}
		else if(KeyNum == 3)//确认数值并退出
		{
			return 0;
		}
		}
		Show_SetTime_FirstUI();
		OLED_ReverseArea(32,16,32,16);
		OLED_Update();
	}
}

int SetMonth(void)
{
		while(1)//检测按键
		{
		KeyNum=Key_GetNum();
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
		if(KeyNum == 2)//数值加1
		{
			Change_RTC_Time(1,1);
			if(MyRTC_Time[1]>=13){MyRTC_Time[1]=1;MyRTC_SetTime();}//判断月份会不会大于12
		}
		else if(KeyNum == 1)//数值减1
		{
			Change_RTC_Time(1,0);
			if(MyRTC_Time[1]<=0){MyRTC_Time[1]=12;MyRTC_SetTime();}//判断月份会不会小于1
		}
		else if(KeyNum == 3)//确认数值并退出
		{
			return 0;
		}
	}
		Show_SetTime_FirstUI();
		OLED_ReverseArea(32,32,16,16);
		OLED_Update();
}
}

int SetDay(void)//不用判断一个月是31天还是30天什么的，反正界限是0-31天，其余在RTC内部的转换函数写好了
{
		while(1)//检测按键
		{
		KeyNum=Key_GetNum();
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
		if(KeyNum == 2)//数值加1
		{
			Change_RTC_Time(2,1);
			if(MyRTC_Time[2]>=32){MyRTC_Time[2]=1;MyRTC_SetTime();}//判断号数会不会大于31
		}
		else if(KeyNum == 1)//数值减1
		{
			Change_RTC_Time(2,0);
			if(MyRTC_Time[2]<=0){MyRTC_Time[2]=31;MyRTC_SetTime();}//判断号数会不会小于1
		}
		else if(KeyNum == 3)//确认数值并退出
		{
			return 0;
		}
	}
		Show_SetTime_FirstUI();
		OLED_ReverseArea(32,48,16,16);
		OLED_Update();
}
}

int SetHour(void)
{
while(1)//检测按键
		{
		KeyNum=Key_GetNum();
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
		if(KeyNum == 2)//数值加1
		{
			Change_RTC_Time(3,1);
			if(MyRTC_Time[3]>=24){MyRTC_Time[3]=0;MyRTC_SetTime();}
		}
		else if(KeyNum == 1)//数值减1
		{
			Change_RTC_Time(3,0);
			if(MyRTC_Time[3]<0){MyRTC_Time[3]=23;MyRTC_SetTime();}
		}
		else if(KeyNum == 3)//确认数值并退出
		{
			return 0;
		}
	}
		Show_SetTime_SecondUI();
		OLED_ReverseArea(32,0,16,16);
		OLED_Update();
}
}

int SetMin(void)
{
while(1)//检测按键
		{
		KeyNum=Key_GetNum();
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
		if(KeyNum == 2)//数值加1
		{
			Change_RTC_Time(4,1);
			if(MyRTC_Time[4]>=60){MyRTC_Time[4]=0;MyRTC_SetTime();}
		}
		else if(KeyNum == 1)//数值减1
		{
			Change_RTC_Time(4,0);
			if(MyRTC_Time[4]<0){MyRTC_Time[4]=59;MyRTC_SetTime();}
		}
		else if(KeyNum == 3)//确认数值并退出
		{
			return 0;
		}
	}
		Show_SetTime_SecondUI();
		OLED_ReverseArea(32,16,16,16);
		OLED_Update();
}
}

int SetSec(void)
{
while(1)//检测按键
		{
		KeyNum=Key_GetNum();
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
		if(KeyNum == 2)//数值加1
		{
			Change_RTC_Time(5,1);
			if(MyRTC_Time[5]>=60){MyRTC_Time[5]=0;MyRTC_SetTime();}
		}
		else if(KeyNum == 1)//数值减1
		{
			Change_RTC_Time(5,0);
			if(MyRTC_Time[5]<0){MyRTC_Time[5]=59;MyRTC_SetTime();}
		}
		else if(KeyNum == 3)//确认数值并退出
		{
			return 0;
		}
	}
		Show_SetTime_SecondUI();
		OLED_ReverseArea(32,32,16,16);
		OLED_Update();
}
}





/*----设置时间函数--------------------------*/








/*-----光标操作函数-------------------------*/

//这个函数是在这个日期时间设置界面里操作光标

int SetTimeflag=1;//这个是设置界面的光标位置指示位

void SetTime(void *argument)
{
	while(1)
	{
		uint8_t SetTimeFlag_Temp=0;//与上一级“设置页面”的那个临时变量作用一样，可以参考之前的注释
		//一定要记得这个临时变量SetTimeFlag_Temp要先清零
		KeyNum=Key_GetNum();
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
		if(KeyNum==1)//上一项
		{
			SetTimeflag--;
			if(SetTimeflag<=0)
				{
				SetTimeflag=7;
				}
		}
		else if(KeyNum==2)//下一项
			{
				SetTimeflag++;
				if(SetTimeflag>=8)
				{
					SetTimeflag=1;
				}
			}
		else if(KeyNum==3)//按下按键
			{	
				OLED_Clear();
				OLED_Update();	
				//return Setflag;	//只在选中的这个case里返回值，这样在main函数里clkflag1得到的就只有是选中的时候才有对应返回值（1或2），其他情况不返回
				SetTimeFlag_Temp=SetTimeflag;
			}
		}
						
		//如果确认按键按下，检测按下按键时候光标是指向哪个选项
		//如果确认按键未被按下，则不管
			if(SetTimeFlag_Temp==1)//说明是在 返回键 那里按下确认
			{
				//向页面管理行列写数据
				My_Send_Task_Queue(PAGE_SETTING);
			}//返回上级
			else if(SetTimeFlag_Temp==2){SetYear();}//年
			else if(SetTimeFlag_Temp==3){SetMonth();}//月
			else if(SetTimeFlag_Temp==4){SetDay();}//日
			else if(SetTimeFlag_Temp==5){SetHour();}//时
			else if(SetTimeFlag_Temp==6){SetMin();}//分
			else if(SetTimeFlag_Temp==7){SetSec();}//秒

			
	switch(SetTimeflag)
	{
		//case为1、2、3、4时，显示第一页UI
		case 1:
			OLED_Clear();//首先清屏防止重叠
			Show_SetTime_FirstUI();//在第一页UI
			OLED_ReverseArea(0,0,16,16);//选中“年”
			OLED_Update();
			break;
		case 2:
			OLED_Clear();
			Show_SetTime_FirstUI();
			OLED_ReverseArea(0,16,16,16);//选中即反色
			OLED_Update();
			break;
		case 3:
			OLED_Clear();
			Show_SetTime_FirstUI();
			OLED_ReverseArea(0,32,16,16);//选中即反色
			OLED_Update();
			break;
		case 4:
			OLED_Clear();
			Show_SetTime_FirstUI();
			OLED_ReverseArea(0,48,16,16);//选中即反色
			OLED_Update();
			break;
		
		//case为5、6、7时，显示第二页UI
		case 5:
			OLED_Clear();
			Show_SetTime_SecondUI();
			OLED_ReverseArea(0,0,16,16);//选中“时”，即反色
			OLED_Update();
			break;
		case 6:
			OLED_Clear();
			Show_SetTime_SecondUI();
			OLED_ReverseArea(0,16,16,16);
			OLED_Update();
			break;
		case 7:
			OLED_Clear();
			Show_SetTime_SecondUI();
			OLED_ReverseArea(0,32,16,16);
			OLED_Update();
			break;

}
		// RTOS 延时
    vTaskDelay(pdMS_TO_TICKS(10));
	}}

/*-------------------------光标操作函数-----*/
