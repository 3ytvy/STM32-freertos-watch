#ifndef __LIFT_INT_H
#define __LIFT_INT_H

extern uint8_t text;


void MPU6050_EnableMotionInterrupt(void);
uint8_t MPU6050_Gesture_IsWristRaise(void);
void MPU6050_Gesture_ClearInterrupt(void);


#endif
