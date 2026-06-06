#include "stm32f1xx_hal.h"

void Delay_us(uint32_t us)
{
    uint32_t i;
    while (us--)
    {
        // 这个循环在72MHz下约等于1us，经过验证可用
        i = 17;
        while (i--);
    }
}

