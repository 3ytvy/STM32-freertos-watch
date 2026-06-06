#ifndef __MYRTC_H
#define __MYRTC_H

extern int MyRTC_Time[];

void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);

void Change_RTC_Time(uint8_t i,uint8_t flag);

#endif
