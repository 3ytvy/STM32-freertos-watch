#ifndef __STEP_COUNTING_H
#define __STEP_COUNTING_H

/*-------temp_text------*/
extern int16_t aax;
extern int16_t aay;
extern int16_t aaz;
/*-------temp_text------*/




//定义动态阈值结构体类型peak_value_t
typedef struct{
	axis_info_t newmax;//当前周期50个样本中的最大值
	axis_info_t newmin;//当前周期50个样本中的最小值
	axis_info_t oldmax;//上一个周期50个样本中的最大值
	axis_info_t oldmin;//上一个周期50个样本中的最小值
}peak_value_t;

typedef struct{
	axis_info_t new_sample;//上  次有效的数据
	axis_info_t old_sample;//上上次有效的数据
}slid_reg_t;
//创建slid_reg_t的实例slid
extern slid_reg_t slid;


extern int16_t fil_ax;
extern int16_t fil_ay;
extern int16_t fil_az;

extern int16_t threshold_x;
extern int16_t threshold_y;
extern int16_t threshold_z;

extern peak_value_t peak;
extern unsigned int sample_count;

extern unsigned int step_count;



void filter_init(void);

void filter_calculate(int16_t ax,int16_t ay,int16_t az);

void peak_newvalue_init(peak_value_t *peak);

void peak_update(peak_value_t *peak,axis_info_t *cur_sample);

void calculate_threshold(peak_value_t *peak);

char slid_update(slid_reg_t *slid,axis_info_t *cur_sample);

void detect_step(peak_value_t *peak,slid_reg_t *slid,axis_info_t *cur);



void Step_Counting(void);

void step_counting_function(void *argument);



#endif
