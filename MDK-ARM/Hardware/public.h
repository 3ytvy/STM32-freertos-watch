#ifndef __PUBLIC_H
#define __PUBLIC_H

#include <stdint.h> 

#define PAGE_HOME        1    // 首页
#define PAGE_MENU        2    // 菜单
#define PAGE_SETTING     3    // 设置
#define PAGE_SETTIME     4    // 时间设置
#define PAGE_STOPWATCH   5    // 秒表
#define PAGE_DINO        6    // 小恐龙
#define PAGE_FLASH       7    // 手电筒
#define PAGE_MUSIC       8    // 音乐


// ========================
// 全局通用结构体（整个项目都能用）
// ========================
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
}axis_info_t;




#endif
