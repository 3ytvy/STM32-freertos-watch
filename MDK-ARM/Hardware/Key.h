#ifndef __KEY_H
#define __KEY_H

extern int press_time;
extern int Key_Num;

void Key_Init(void);
uint8_t Key_GetNum(void);
void Key_Tick(void);
void Key3_Tick(void);

#endif
