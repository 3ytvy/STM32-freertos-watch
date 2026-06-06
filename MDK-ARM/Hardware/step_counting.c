#include "stm32f1xx_hal.h"
#include "MPU6050.h"
#include "MyI2C.h"
#include "public.h"
#include "MPU6050_Reg.h"

#include "Serial.h"
#include "OLED.h"


#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


int16_t AX, AY, AZ, GX, GY, GZ;			//定义用于存放各个数据的变量


axis_info_t current_sample;

uint8_t conf;


/*-------temp_text------*/

int16_t aax=0,aay=0,aaz=0;

/*-------temp_text------*/

extern uint8_t loop_test_flag;	/*引自main.c*/
extern uint8_t if_count;	/*引自main.c*/
extern uint8_t time_count_flag;	/*引自main.c*/


/*均值滤波-----------------------------------------------------*/

//宏定义两个方法（类似函数），得出两者比较中的大和小的值
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

#define filtercnt 4//定义滤波是4次取一次平均值

int16_t fil_ax,fil_ay,fil_az;//经过均值滤波后的x、y、z轴方向上的加速度ax、ay、az
//这三个是全局变量


//filter_avg_t类型结构体定义
//定义一个filter_avg_t结构体类型
typedef struct{
	axis_info_t info[filtercnt];//这个拿axis_info_t这个结构体类型来定义一个结构体info的数组，用于存放最近 filtercut 次的数据
	//这个是结构体数组，也就是这个是由多个同类型结构体构成的数组
	unsigned char index;//下一次写入的位置
	unsigned char is_full;//是否已满的标志位
}filter_avg_t;

filter_avg_t filter;//定义一个filter_avg_t类型的结构体

//函数：初始化滤波器，把所有历史数据清零
void filter_init(void)
{
	uint8_t i;
	
	//把filter.info这个axis_info_t类型结构体数组的的数据清除
	for(i=0;i<filtercnt;i++)
	{
		filter.info[i].x=0;
		filter.info[i].y=0;
		filter.info[i].z=0;
	}
	
	//重置位置索引和已满标志位
	filter.index=0;
	filter.is_full=0;
	
}



//函数：滤波函数
//简介：对读取到的3个轴方向的加速度进行均值滤波
//作用：实现均值滤波，就是拿到 filtercnt 组x,y,z三轴数据，求平均值。最后的平均值作为输出结果，使输出结果更加平滑。完成初步滤波。
//参数：x、y、z轴方向上的加速度ax、ay、az（由MPU6050传入）
void filter_calculate(int16_t ax,int16_t ay,int16_t az)
{
	
	uint8_t i;
	int32_t x_sum=0,y_sum=0,z_sum=0;// 用32位求和，防止溢出
	uint8_t valid_cnt;// 有效数据的个数
	
	// #1.存入新数据
	filter.info[filter.index].x=ax;
	filter.info[filter.index].y=ay;
	filter.info[filter.index].z=az;
	
	// #2.更新位置索引，使形成环形移动
	filter.index++;
	if(filter.index >= filtercnt)
	{
		filter.index=0;//位置索引归零，绕回开头
		filter.is_full=1;//filter.info已存满一圈的标志
	}
	
	// #3.确定有几个有效数据
	if(filter.is_full==1)
	{
		valid_cnt=filtercnt;//已满，用全部 filtercnt 个数据
	}
	else
	{
		valid_cnt=filter.index;//未满，只用已存入的数据
	}
	//如果还没有数据，直接返回
	if(valid_cnt==0)
	{
		return;
	}
	
	// #4.求和
	for(i=0;i<valid_cnt;i++)
	{
		x_sum += filter.info[i].x;
		y_sum += filter.info[i].y;
		z_sum += filter.info[i].z;
	}
	
	// #5.计算平均值，存入全局变量已滤波的值fil_ax,fil_ay,fil_az中
	fil_ax = (int16_t)(x_sum/filtercnt);
	fil_ay = (int16_t)(y_sum/filtercnt);
	fil_az = (int16_t)(z_sum/filtercnt);
		
}

/*-----------------------------------------------------均值滤波*/


/*计算动态阈值-----------------------------------------------------*/

//	定义一个存储三轴加速度数据的结构体类型
//	由于上面已经定义过，这里注释掉
//typedef struct{
//	int16_t x;
//	int16_t y;
//	int16_t z;
//}axis_info_t

#define SAMPLE_SIZE 100 // 每100次采样更新一次阈值

//定义动态阈值结构体类型peak_value_t
typedef struct{
	axis_info_t newmax;//当前周期50个样本中的最大值
	axis_info_t newmin;//当前周期50个样本中的最小值
	axis_info_t oldmax;//上一个周期50个样本中的最大值
	axis_info_t oldmin;//上一个周期50个样本中的最小值
}peak_value_t;

peak_value_t peak;//创建一个动态阈值结构体实例
//注意：这个是全局共用的


//定义全局变量（已在.h中声明extern）
int16_t threshold_x;// X轴的动态阈值
int16_t threshold_y;// y轴的动态阈值
int16_t threshold_z;// z轴的动态阈值


//函数：初始化newmax和newmin
//注意：max要初始化为int16_t的最小值，min要初始化为int16_t的最大值
//参数：动态阈值结构体的实例
void peak_newvalue_init(peak_value_t *peak)
{
	// 将newmax初始化为int16_t的最小值 -32768
  // 这样任何实际值都比它大，MAX宏才能正确更新
	peak -> newmax.x = -32768;
	peak -> newmax.y = -32768;
	peak -> newmax.z = -32768;
	
	// 将newmin初始化为int16_t的最小值 32767
  // 这样任何实际值都比它小，MAX宏才能正确更新
	peak -> newmin.x = 32767;
	peak -> newmin.y = 32767;
	peak -> newmin.z = 32767;
	
}


//函数：动态阈值更新
//前提：这个是上个周期的阈值，用于判断下一个周期的数据。
//			同时新周期开始收集新的最大值最小值，为下一次更新做准备
//功能：每来一个新数据，更新新周期的动态阈值，
//			在收集满一个周期（50个样本）后把收集到的阈值赋给oldmax和oldmin
//参数：peak - 动态阈值结构体的实例，
//			cur_sample - 存储三轴加速度数据的结构体实例，它是用来存放当前采集到的数据的
//注意：cur_sample的值是在运行主函数的时候，由MPU6050直接给的

//定义一个全局计数器，用于记录已经采样了多少次
//这个是全局变量，已声明extern
unsigned int sample_count = 0;

void peak_update(peak_value_t *peak,axis_info_t *cur_sample)
{
	// #1.计数器加一
	sample_count++;
	
	// #2.判断是否达到 SAMPLE_SIZE 次
	if(sample_count > SAMPLE_SIZE)
	{
		//每达到50次（满一个周期），就更新一次oldmax和oldmin
		peak->oldmax = peak->newmax;
		peak->oldmin = peak->newmin;
		
		//重置计数器为1，进入下一个周期的采样
		sample_count=1;
		
		//重新初始化newmax和newmin，准备下一个周期
		peak_newvalue_init(peak);
	}
	
	// #3.更新当前采样周期的newmax和newmin
	// 注意：这段代码是在if外的，所以每次调用本函数都会执行 
	
	//更新x轴的newmax和newmin
	peak->newmax.x=MAX(peak->newmax.x,cur_sample->x);
	peak->newmin.x=MIN(peak->newmin.x,cur_sample->x);
	
	//更新y轴的newmax和newmin
	peak->newmax.y=MAX(peak->newmax.y,cur_sample->y);
	peak->newmin.y=MIN(peak->newmin.y,cur_sample->y);
	
	//更新z轴的newmax和newmin
	peak->newmax.z=MAX(peak->newmax.z,cur_sample->z);
	peak->newmin.z=MIN(peak->newmin.z,cur_sample->z);
	
	
}

//函数：计算阈值
//功能：根据oldmax和oldmin计算动态阈值
//注意：这个函数应该在每次更新完oldmax和oldmin后调用
//			threshold_x/y/z这三个变量是全局变量（extern）
void calculate_threshold(peak_value_t *peak)
{
	
	threshold_x = (peak->oldmax.x + peak->oldmin.x) / 2;
	threshold_y = (peak->oldmax.y + peak->oldmin.y) / 2;
	threshold_z = (peak->oldmax.z + peak->oldmin.z) / 2;

}


/*-----------------------------------------------------计算动态阈值*/


/*动态精度-----------------------------------------------------*/

/*
x、y、z三轴的动态精度都不一样
动态精度还与所设置的加速度量程有关，因为加速度量程决定灵敏度，灵敏度决定值大小，值大小关乎动态精度
*/


//定义一个计算绝对值的宏
#define ABS(a) (0-(a)) > 0 ? (-(a)):(a)
//定义一个动态精度的值
#define DYNAMIC_PRECISION_X 1500
#define DYNAMIC_PRECISION_Y 800
#define DYNAMIC_PRECISION_Z 800

//定义存放近两次有效数据的结构体类型slid_reg_t（相当于寄存器）
typedef struct{
	axis_info_t new_sample;//上  次有效的数据
	axis_info_t old_sample;//上上次有效的数据
}slid_reg_t;
//创建slid_reg_t的实例slid
slid_reg_t slid;


//函数：“动态精度过滤器”
//作用：不断采集下一个数据，判断这个新的数据与前一个数据比是否变化很大
//			如果变化很大，就存入slid这个寄存器（上面定义的slid_reg_t类型结构体）里
//			如果变化不大，就忽略这个数据
//			这个变化由定义的动态精度的值确定
//			
//			相当于：slid->old_sample.x、slid_new_sample.x这两个值在不断刷新，前者由后者赋值，相当于步其后尘
//			只有当cur_sample.x和slid_new_sample.x比较起来比规定的那个精度变化来的要大，才会去改变slid_new_sample.x的值
//参数：slid_reg_t类型结构体slid
//			axis_info_t类型结构体cur_sample（这个结构体是用来存放3个轴的数据的）
//
//			注意cur_sample里的值由MPU6050直接赋值
char slid_update(slid_reg_t *slid,axis_info_t *cur_sample)
{
	char res = 0;//有无变化的标志位
	
	//x轴处理
	if(ABS((cur_sample->x - slid->new_sample.x)) > DYNAMIC_PRECISION_X)
	{
		//如果 变化量 > DYNAMIC_PRECISION：这是一个有效变化
		slid->old_sample.x=slid->new_sample.x;//上上个有效数据变成上个有效数据
		slid->new_sample.x=cur_sample->x;//采集到的这个符合有效变化的值变成上个有效数据
		res=1;//标记：有变化
	}
	else
		//不然就是一个小波动，忽略
		//只更新old_sample,new_sample保持不变
	{
		slid->old_sample.x=slid->new_sample.x;
	}
	
		//y轴处理
	if(ABS((cur_sample->y - slid->new_sample.y)) > DYNAMIC_PRECISION_Y)
	{
		//如果 变化量 > DYNAMIC_PRECISION：这是一个有效变化
		slid->old_sample.y=slid->new_sample.y;//上上个有效数据变成上个有效数据
		slid->new_sample.y=cur_sample->y;//采集到的这个符合有效变化的值变成上个有效数据
		res=1;//标记：有变化
	}
	else
		//不然就是一个小波动，忽略
		//只更新old_sample,new_sample保持不变
	{
		slid->old_sample.y=slid->new_sample.y;
	}
	
		//z轴处理
	if(ABS((cur_sample->z - slid->new_sample.z)) > DYNAMIC_PRECISION_Z)
	{
		//如果 变化量 > DYNAMIC_PRECISION：这是一个有效变化
		slid->old_sample.z=slid->new_sample.z;//上上个有效数据变成上个有效数据
		slid->new_sample.z=cur_sample->z;//采集到的这个符合有效变化的值变成上个有效数据
		res=1;//标记：有变化
	}
	else
		//不然就是一个小波动，忽略
		//只更新old_sample,new_sample保持不变
	{
		slid->old_sample.z=slid->new_sample.z;
	}
	
/*-------temp_text------*/

	aax=slid->old_sample.x;
	aay=slid->old_sample.y;
	aaz=slid->old_sample.z;

/*-------temp_text------*/
	
	return res;//返回1就表示至少有一个轴的加速度值发生了有效变化
	

	
	
}




/*-----------------------------------------------------动态精度*/




/*步伐判断-----------------------------------------------------*/

//最活跃轴定义
#define MOST_ACTIVE_NULL 0
#define MOST_ACTIVE_X 1
#define MOST_ACTIVE_Y 2
#define MOST_ACTIVE_Z 3

//最活跃轴最小变化值
/*
挑出最活跃轴后，虽然最活跃轴它是穿过，但是也有最小要求
*/
#define ACTIVE_PRECISION 3000

//全局步数计数器
unsigned int step_count=0;


//函数：判断当前哪个轴变化最大
//参数：peak - 动态阈值结构体（包含当前采集的最大值和最小值）
//返回：最活跃轴的编号
static char is_most_active(peak_value_t *peak)
{
	char res=MOST_ACTIVE_NULL;//用来存放最后返回值最活跃轴的编号的变量，先给个初值
	
	//计算每个轴的变化幅度
	short x_change=ABS((peak->newmax.x - peak->newmin.x));
	short y_change=ABS((peak->newmax.y - peak->newmin.y));
	short z_change=ABS((peak->newmax.z - peak->newmin.z));
	
	//判断哪个轴的变化量是最大的还满足比最活跃轴最小变化值还大
	if(x_change>y_change && x_change>z_change && x_change>=ACTIVE_PRECISION)
	{
		res=MOST_ACTIVE_X;
	}
	
	if(y_change > x_change && y_change > z_change && y_change >= ACTIVE_PRECISION)
	{
		res=MOST_ACTIVE_Y;
	}
	
	if(z_change > x_change && z_change > y_change && z_change >= ACTIVE_PRECISION)
	{
		res=MOST_ACTIVE_Z;
	}
	
	return res;
	
}

//函数：检测这个最活跃轴，判断是否走了一步
//参数：peak - 动态阈值结构体
//			slid - 最近两个加速度数据（经过动态精度过滤）寄存器结构体（包含old_sample和new_sample）
//			cur - 当前采样数据
void detect_step(peak_value_t *peak,slid_reg_t *slid,axis_info_t *cur)
{
	// #1.获取最活跃轴
	char active_axis=is_most_active(peak);
	
	// #2.根据最活跃轴进行判断
	switch(active_axis)
	{
		case MOST_ACTIVE_NULL:
			//没有找到最活跃轴，不做判断
			break;
		
		case MOST_ACTIVE_X:
			{
				if(slid->old_sample.x>threshold_x && slid->new_sample.x<threshold_x)
				{
					step_count++;//计步+1
					if(time_count_flag == 1)//确定是在“熄屏”只跑一个计步任务的时候
					{
						if_count++;
					}
				}
			}
			break;
		
		case MOST_ACTIVE_Y:
			{
				if(slid->old_sample.y>threshold_y && slid->new_sample.y<threshold_y)
				{
					step_count++;//计步+1
					if(time_count_flag == 1)//确定是在“熄屏”只跑一个计步任务的时候
					{
						if_count++;
					}
				}
			}
			break;
		
		case MOST_ACTIVE_Z:
			{
				if(slid->old_sample.z>threshold_z && slid->new_sample.z<threshold_z)
				{
					step_count++;//计步+1
					if(time_count_flag == 1)//确定是在“熄屏”只跑一个计步任务的时候
					{
						if_count++;
					}
				}
			}
			break;
		
		default:
            break;
	}
	
}



/*-----------------------------------------------------步伐判断*/




/*供外部调用的计步函数-----------------------------------------------------*/

void Step_Counting(void)
{
	// #1.读取原数据
	MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);		//获取MPU6050的数据
	
	// #2.进行滤波，滤波所得赋到全局变量里
	filter_calculate(AX, AY, AZ);//对加速度ax、ay、az进行均值滤波
	
	// #3.把滤波所得的数据放进采样结构体里（注：结构体current_sample已在上面定义）
	current_sample.x=fil_ax;
	current_sample.y=fil_ay;
	current_sample.z=fil_az;
	
	// #4.更新动态阈值
	peak_update(&peak,&current_sample);
	
	// #5.检测一个周期是否完成（采样满50个样本）
	//		如果完成，给oldmax和oldmin赋新值，并开启新采样周期
	//		执行完这个函数，就更新了那3个全局变量threshold_x/y/z的值（也就是三个方向轴上的动态阈值）
	if(sample_count==1)//当sample_count等于1时，就是说新的采样周期已开始，这时候已经更新了oldmax和oldmin（见对应.c文件的阈值更新函数的内容）
	{
		calculate_threshold(&peak);
	}
	
	
	
	// #6.进行“动态精度”的过滤，内容存到结构体slid里，有slid->old_sample和slid->new_sample
	slid_update(&slid,&current_sample);	
	// #7.计步判断
	detect_step(&peak,&slid,&current_sample);
	
//////	Serial_Printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",AX,AY,AZ,fil_ax,fil_ay,fil_az,threshold_x,threshold_y,threshold_z,aax,aay,aaz,GX,GY,GZ);

////////    uint8_t regs[] = {0x6B, 0x6C, 0x19, 0x1A, 0x1B, 0x1C};
////////    const char *names[] = {"PWR_MGMT_1","PWR_MGMT_2","SMPLRT_DIV","CONFIG","GYRO_CONFIG","ACCEL_CONFIG"};
////////    uint8_t i, val;
////////    
////////    for (i = 0; i < 6; i++) {
////////        val = MPU6050_ReadReg(regs[i]);
////////        Serial_Printf("%s = 0x%02X\r\n", names[i], val);
////////    }
}






/*通用的处理FIFO的100个样本以内数据的供外部调用的计步函数-----------------------------------------------------*/

void Step_Counting_for_samples(void)
{
	// #1.读取原数据
	MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);		//获取MPU6050的数据
	
	// #2.进行滤波，滤波所得赋到全局变量里
	filter_calculate(AX, AY, AZ);//对加速度ax、ay、az进行均值滤波
	
	// #3.把滤波所得的数据放进采样结构体里（注：结构体current_sample已在上面定义）
	current_sample.x=fil_ax;
	current_sample.y=fil_ay;
	current_sample.z=fil_az;
	
	// #4.更新动态阈值
	peak_update(&peak,&current_sample);
	
	// #5.检测一个周期是否完成（采样满50个样本）
	//		如果完成，给oldmax和oldmin赋新值，并开启新采样周期
	//		执行完这个函数，就更新了那3个全局变量threshold_x/y/z的值（也就是三个方向轴上的动态阈值）
	if(sample_count==1)//当sample_count等于1时，就是说新的采样周期已开始，这时候已经更新了oldmax和oldmin（见对应.c文件的阈值更新函数的内容）
	{
		calculate_threshold(&peak);
	}
	
	
	
	// #6.进行“动态精度”的过滤，内容存到结构体slid里，有slid->old_sample和slid->new_sample
	slid_update(&slid,&current_sample);	
	// #7.计步判断
	detect_step(&peak,&slid,&current_sample);
	
//////	Serial_Printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",AX,AY,AZ,fil_ax,fil_ay,fil_az,threshold_x,threshold_y,threshold_z,aax,aay,aaz,GX,GY,GZ);

////////    uint8_t regs[] = {0x6B, 0x6C, 0x19, 0x1A, 0x1B, 0x1C};
////////    const char *names[] = {"PWR_MGMT_1","PWR_MGMT_2","SMPLRT_DIV","CONFIG","GYRO_CONFIG","ACCEL_CONFIG"};
////////    uint8_t i, val;
////////    
////////    for (i = 0; i < 6; i++) {
////////        val = MPU6050_ReadReg(regs[i]);
////////        Serial_Printf("%s = 0x%02X\r\n", names[i], val);
////////    }
}














/*将此函数丢进while循环里*/
void step_counting_function(void *argument)
{
	while(1)
	{
		if(loop_test_flag >= 50)
		{
			loop_test_flag=0;
			Step_Counting();
			
///////////////////////			OLED_Clear();
///不要共用OLED I2C////			OLED_Printf(0,0,OLED_8X16,"Step:%d",step_count);
///////////////////////			OLED_Update();
			
//////////////			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5) == GPIO_PIN_SET)
//////////////			{
//////////////				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
//////////////			}
//////////////			else
//////////////			{
//////////////				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
//////////////			}
			
		}
	
		//RTOS延时
		vTaskDelay(pdMS_TO_TICKS(10));
	}

}





/*-----------------------------------------------------供外部调用的计步函数*/



