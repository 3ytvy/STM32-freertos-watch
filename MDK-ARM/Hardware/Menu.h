#ifndef __MENU_H
#define __MENU_H

#include "public.h"


extern unsigned int open_time;

extern uint8_t game_Flag;
extern uint8_t flash_open;//手电筒有无开启的标志位（整个项目共用）
extern uint8_t flash_power;//手电筒亮度



void Menu_Init(void);
void Show_Clock_UI(void);

void Show_SettingPage_UI(void);


void PWM_Init(void);
void PWM_SetCompare2(uint16_t Compare);

void Buzzer_SetFreq(uint16_t freq);

void MPU6050_Calculate(void);
int Gradienter(void);
int Clock_page(void);


void FirstPage_Clock(void *argument);
void SettingPage(void *argument);
void Menu(void *argument);
void stopwatch(void *argument);
void dinopage(void *argument);
void flashpage(void *argument);
void musicpage(void *argument);



void My_TIM3_IRQHandler(void);

#endif
