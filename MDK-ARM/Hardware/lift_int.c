#include "stm32f1xx_hal.h"
#include "MPU6050.h"
#include "MPU6050_Reg.h"
/*
第一层：（硬件判断）MPU6050 硬件运动中断（粗判）
	只要动作够大、持续够久 → 触发中断
	作用：把单片机从停机模式唤醒
第二层：（软件判断）自己写的抬腕算法（精判）
	中断唤醒后，再用软件算法判断是不是真的抬腕
	作用：决定亮不亮屏
*/


extern uint32_t GetTick(void);//引自main.c

//简介：配置好MPU6050的运动中断，并且将MPU6050的INT引脚接到单片机的PA0（外部中断）引脚
//注意：有一个小bug，就是上电后，主函数调用MPU6050_Gesture_IsWristRaise判断是否抬腕，一开始总是会得到是抬腕。后续的就正常
//			但是这个bug刚好满足上电就开机的要求，就不修改了。

/*--------text---------*/

uint8_t text=0;
uint8_t text1=0;

/*--------text---------*/


//定义抬腕检测结果
#define WRIST_RAISE_DETECTED 1	//检测到抬腕
#define WRIST_RAISE_NONE 0	//未检测到抬腕

//定义一个计算绝对值的宏
#define ABS(a) (0-(a)) > 0 ? (-(a)):(a)

//函数声明
void MPU6050_EnableMotionInterrupt(void);	// 初始化配置抬腕检测（加速度）中断
uint8_t MPU6050_Gesture_IsWristRaise(void);	// 判断是否是抬腕动作
void MPU6050_Gesture_ClearInterrupt(void);	// 清除中断标志


//函数实现

//函数：配置MPU6050的运动中断
//执行完这个函数，就开启了MPU6050的运动中断，只要有一个轴的加速度计超过阈值持续一段时间，就会触发MPU6050中断，并且反应到INT引脚上去
void MPU6050_EnableMotionInterrupt(void)
{
	
//////////////	// #1.设置加速度计量程为 ±8g  
//////////////	//		使用到的寄存器是：ACCEL_CONFIG
////////////////	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG,0x00);
//////////////	//原文：加速度传感器量程和高通滤波器配置，原文是±16g 0x1C就是±16g和有高通滤波
//////////////		MPU6050_WriteReg(MPU6050_ACCEL_CONFIG,0x04);
	//Bit7~Bit5：X/Y/Z A_ST是自检，都为0
	//Bit4~Bit3：AFS_SEL 设置量程 10即2，位±8g
	//Bit2~Bit0：滤波，设为0
	// 000 10 000
	// 000 01 100   1C
	// 000 00 100
	
	// #2.设置运动阈值（MOT_THR = 0x1F）
	//		这个值决定了多大幅度的运动会触发
	//		对于抬腕，设置在0x20到0x40之间
//	MPU6050_WriteReg(MPU6050_MOT_THR,0x30);
	MPU6050_WriteReg(MPU6050_MOT_THR,0x10);
//	MPU6050_WriteReg(MPU6050_MOT_THR, 0x38);   // 阈值更高

	
	// #3.设置运动持续时间（MOT_DUR=0x20）
	//		这个值决定了上面决定的那个运动触发持续多久会触发中断
	//		Bit7~Bit0组成一个8位无符号值，整个值就是这个时间，单位：1ms
	//		这里设置时间：20ms   DEC:20 HEX:14
	MPU6050_WriteReg(MPU6050_MOT_DUR,0x14);
//	MPU6050_WriteReg(MPU6050_MOT_DUR, 0x28);  // 时间更长：100ms

	
	// #4.配置中断引脚（INT_PIN_CFG=0x1C）
	//		该寄存器配置中断信号在INT引脚上的行为
	//		位7：中断电平决定是高电平（0）还是低电平（1）有效
	//		位6：中断开启，是用来决定INT引脚是推挽（0）还是开漏（1）
	//		位5：锁存中断使能，0时：INT引脚发出一个50微秒长的脉冲；1时：INT引脚持续信号直到中断结束
	//		位4：中断读清除，0时：中断状态位仅通过读取INT_STATUS（寄存器58）清除；1时：仍和读取操作都会清除中断标志位
	//		位3~位0：其他，跟随原文配置
	//		要求：中断引脚高电平有效，推挽输出，INT引脚发出脉冲
	//		0001 1100 → 0x1C
//	MPU6050_WriteReg(MPU6050_INT_PIN_CFG,0xA0);
	MPU6050_WriteReg(MPU6050_INT_PIN_CFG,0x1C);
	
	// #5.使能运动中断（INT_ENABLE = 0x38）
	//		bit6是加速度中断使能
	//		0100 0000 0x40
	MPU6050_WriteReg(MPU6050_INT_ENABLE,0x40);
	
	// #6.清除残留的中断标志
	MPU6050_ReadReg(MPU6050_INT_STATUS);
}



//函数：配置外部中断1（对应引脚PA1）
/*CUBEMX已自动生成初始化代码*/




//函数：判断是否是真正的抬腕动作
//返回值： 1 - 是抬腕 | 0 - 不是抬腕






////////////滴答计时器（这个是STM32自带的计时器，需要手动配置开启）
////////////全局变量，每1ms自动+1
////////////uint32_t uwTick=0;
////////////内核定时中断（每1ms进一次）
////////////本来是在库函数里的，库函数里那个被我注释掉，移到这里来了
//////////void SysTick_Handler(void)
//////////{
//////////	uwTick++;
//////////}
//////////uint32_t GetTick(void)
//////////{
//////////	return uwTick;
//////////}





//抬腕检测参数
////#define MOTION_THRESHOLD 10000//角速度检测阈值（抬腕时角速度应大于此值）
////#define HORIZONTAL_THRESHOLD 300//水平检测阈值（抬腕时，X/Y轴加速度小于此值）
////#define TIME_WINDOWS_MS 250//时间窗口（抬腕后多少毫秒内检测水平）
////#define GYRO_STOP_THRESHOLD  2000     // 角速度“静止”阈值（小于此值认为停止）

#define MOTION_THRESHOLD 0//角速度检测阈值（抬腕时角速度应大于此值）
#define HORIZONTAL_THRESHOLD 500//水平检测阈值（抬腕时，X/Y轴加速度小于此值）
#define TIME_WINDOWS_MS 100//时间窗口（抬腕后多少毫秒内检测水平）
#define GYRO_STOP_THRESHOLD  100000     // 角速度“静止”阈值（小于此值认为停止）

//判断是否抬腕的函数主体
uint8_t MPU6050_Gesture_IsWristRaise(void)
{
	
	int16_t ax,ay,az,gx,gy,gz;
	uint32_t start_time;
	uint8_t is_horizontal = 0;//手表是否是保持水平不动的标志位
	uint8_t is_gyro_stop=0;//手表角速度是否接近静止的标志位
	
	// #1.读取加速度和角速度，判断是否有运动
	MPU6050_GetData(&ax,&ay,&az,&gx,&gy,&gz);
	
	uint8_t gyro=0;//定义用角速度判断是否有运动的标志位
	
//原角速度判断逻辑：
	//检查是否有足够大的运动（抬手腕时有明显的角速度变化）
	//只要x/y/z中其中一个角速度符合即可
//	if(ABS(gx)>MOTION_THRESHOLD || ABS(gy)>MOTION_THRESHOLD || ABS(gz)>MOTION_THRESHOLD)
		
//现角速度判断逻辑：
	if(ABS(gx)>30000 && ABS(gy)>30000 && ABS(gz)>30000) //某个角速度极端大（对应非常急促动作）
	{
		gyro=1;
	}
	
	if(((ABS(gx)+ABS(gy)+ABS(gz))/3) > MOTION_THRESHOLD) //平均角速度大（对应正常抬腕）
	{
		gyro=1;
	}
	
	if(gyro==1)
	{
		text=1;//表示过了第一关
		//检测到运动，记录开始时间
		start_time=GetTick();//获取当前毫秒数

		
		// #2.在时间窗口内循环检测是否水平
		//		并且还要求角速度变小（因为抬腕完手表应该是保持水平和表盘朝上，这时角速度就接近0）
		while((GetTick()-start_time)<TIME_WINDOWS_MS)
		{
			//在这个循环里，读取数据（加速度）
			MPU6050_GetData(&ax,&ay,&az,&gx,&gy,&gz);
			
			//通过加速度数据判断是否水平
			//	抬腕后手表应该接近水平（因为在看手表或者操作）
			//	此处要求x轴和y轴的加速度数据都小于阈值
			if(ABS(ax)<HORIZONTAL_THRESHOLD && ABS(ay)<HORIZONTAL_THRESHOLD && az>100)
			{
				is_horizontal=1;//水平标志位置一
				break;//在时间窗口内检测到水平，退出循环
			}
			else
			{
				text=0;
				is_horizontal=0;
			}
			//要求角速度逐渐接近0，这样反映抬腕完手表静止看表
			if((GetTick()-start_time)>TIME_WINDOWS_MS*(1/2) && ABS(gx) < GYRO_STOP_THRESHOLD && ABS(gy) < GYRO_STOP_THRESHOLD && ABS(gz) < GYRO_STOP_THRESHOLD)
			{
				is_gyro_stop=1;
			}
			//延时10ms再检测下一次，避免过于频繁
			HAL_Delay(10);
		}
		
		// #3.如果在时间窗口内测到水平，相当于完全符合抬腕动作，即检测到了抬腕，返回1
		if(is_horizontal == 1 && is_gyro_stop == 1)
		{
			text=2;//表示过了第二关
			text1 = WRIST_RAISE_DETECTED;
			return WRIST_RAISE_DETECTED;
		}
		else//否则，过了角速度的第一关，过不了加速度的第二关，也不算抬腕，返回0
		{
			text1=WRIST_RAISE_NONE;
			return WRIST_RAISE_NONE;
		}
	}
	else//否则，连第一关（通过角速度判断出有大的动作）都过不了，就是没检测到抬腕，返回0
		{
			text1=WRIST_RAISE_NONE;
			return WRIST_RAISE_NONE;
		}
	
}

//函数：清除MPU6050的运动中断标志
void MPU6050_Gesture_ClearInterrupt(void)
{
	// 读取INT_STATUS寄存器（0x3A）即可清除中断标志
		MPU6050_ReadReg(0x3A);
}





