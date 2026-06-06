#include "stm32f1xx_hal.h"

//引入ADC句柄
extern ADC_HandleTypeDef hadc1;

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
	* 说		明：开启ADC1，配置其通道0（对应引脚是PA0）
	* 已由CubeMX自动生成初始化代码
  */


/**
  * 函    数：获取AD转换的值
  * 参    数：无
  * 返 回 值：AD转换的值，范围：0~4095
  */
uint16_t AD_GetValue(void)
{
	uint16_t adc_val;
	
	// #1.软件触发ADC转换
	HAL_ADC_Start(&hadc1);
	
	// #2.等待转换完成
	HAL_ADC_PollForConversion(&hadc1,HAL_MAX_DELAY);
	
	// #3.读取ADC结果
	adc_val = HAL_ADC_GetValue(&hadc1);
	
	return adc_val;
}
