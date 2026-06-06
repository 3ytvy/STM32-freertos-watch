#include "stm32f1xx_hal.h"
#include "rtc.h"
#include "time.h"

extern RTC_HandleTypeDef hrtc;

#define MYRTC_FLAG 0xA5A5

int MyRTC_Time[] = {2026, 3, 22, 9, 27, 50,5};	//extern
int MyRTC_Time_Init[] = {2026, 3, 22, 9, 27, 50,5};	

//如果LSE无法起振导致程序卡死在初始化函数中
//可将CubeMx里使用LSI当作RTCCLK
//但是LSI无法由备用电源供电，故主电源掉电时，RTC走时会暂停

/*
VBAT 不需要在 CubeMX 里点开关，也不需要你写代码开启！
VBAT 是硬件自动的
*/

/*
RTC的本质就是一个计数器
它还有多个备份寄存器
亦即BKP_DRx
用HAL_RTCEx_BKUPRead()函数从指定的RTC备份数据寄存器读取数据
用HAL_RTCEx_BKUPWrite()函数将数据写入指定的RTC备份寄存器里去
*/

void MyRTC_SetTime(void);
void MyRTC_Init_Time(void);

/*初次开启RTC，初始化RTC*/
void MyRTC_Init(void)
{
//	//开启后备区域访问
//	PWR_BackupAccessCmd(ENABLE);
	HAL_PWR_EnableBkUpAccess();//开启备份域访问权限（防止误写 BKP/RTC）


	if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1) != MYRTC_FLAG)//判断是否为初次开启RTC，如果是第一次开启，设置初始时间，写入记号
	{		 
			/*这是从rtc.c复制过来的cubemx自动生成的初始化函数-------------*/
			if (HAL_RTC_Init(&hrtc) != HAL_OK)
			{
				Error_Handler();
			}
			/*----------------从rtc.c复制过来的cubemx自动生成的初始化函数*/
		
		//设置初始时间
		MyRTC_Init_Time();
		
		//写入BKP，说明是第一次开启
		HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1, MYRTC_FLAG);
	}
	else
	{
			MyRTC_SetTime();//给RTC设置时间（把 全局数组变量MyRTC_Time[]的内容 写入 RTC的counter里）
	}
}

void MyRTC_Init_Time(void)//给RTC初始化时间（把 全局数组变量MyRTC_Time_Init[]的内容 写入 RTC的counter里）
{		
	RTC_TimeTypeDef sTime={0};//定义HAL库要的结构体sTime
	RTC_DateTypeDef dTime={0};//定义HAL库要的结构体dTime
	sTime.Hours = MyRTC_Time_Init[3];
	sTime.Minutes = MyRTC_Time_Init[4];
	sTime.Seconds = MyRTC_Time_Init[5];
	
	dTime.Year = MyRTC_Time_Init[0] - 2000;
	dTime.Month = MyRTC_Time_Init[1];
	dTime.WeekDay = MyRTC_Time_Init[6];
	dTime.Date = MyRTC_Time_Init[2];
	
	HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN);//RTC_FORMAT_BCD指的压缩码，RTC_FORMAT_BIN指的是直接填数字
	HAL_RTC_SetDate(&hrtc,&dTime,RTC_FORMAT_BIN);
}


void MyRTC_SetTime(void)//给RTC设置时间（把 全局数组变量MyRTC_Time[]的内容 写入 RTC的counter里）
{		
	RTC_TimeTypeDef sTime={0};//定义HAL库要的结构体sTime
	RTC_DateTypeDef dTime={0};//定义HAL库要的结构体dTime
	sTime.Hours = MyRTC_Time[3];
	sTime.Minutes = MyRTC_Time[4];
	sTime.Seconds = MyRTC_Time[5];
	
	dTime.Year = MyRTC_Time[0] - 2000;
	dTime.Month = MyRTC_Time[1];
	dTime.WeekDay = MyRTC_Time[6];
	dTime.Date = MyRTC_Time[2];
	
	HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN);//RTC_FORMAT_BCD指的压缩码，RTC_FORMAT_BIN指的是直接填数字
	HAL_RTC_SetDate(&hrtc,&dTime,RTC_FORMAT_BIN);
}



void MyRTC_ReadTime(void)//读取RTC时间（读取 RTC的counter 到 全局数组变量MyRTC_Time[]里）
{	
	RTC_TimeTypeDef sTime={0};//定义HAL库要的结构体sTime
	RTC_DateTypeDef dTime={0};//定义HAL库要的结构体dTime
	
	HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&dTime,RTC_FORMAT_BIN);
	
	MyRTC_Time[0] = dTime.Year + 2000;
	MyRTC_Time[1] = dTime.Month;
	MyRTC_Time[2] = dTime.Date;
	MyRTC_Time[3] = sTime.Hours;
	MyRTC_Time[4] = sTime.Minutes;
	MyRTC_Time[5] = sTime.Seconds;
	MyRTC_Time[6] = dTime.WeekDay;
}


/*外部调用的改时间函数*/
/*
		参		数：uint8_t i				MyRTC_Time[]数组索引：0 - 年 | 1 - 月 | 2 - 日 | 3 - 时 | 4 - 分 | 5 - 秒 | 6 - 周几
							uint8_t flag		加减标志位 flag=1:往上加1，flag=0往下减1
		说		明：这个是底层函数，可被外部设备的业务函数调用。
*/

void Change_RTC_Time(uint8_t i,uint8_t flag)
{
	if(flag==1)
	{
		MyRTC_Time[i]++;
	}
	else
	{
		MyRTC_Time[i]--;
	}
	MyRTC_SetTime();	
}

