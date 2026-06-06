# STM32-Freertos-Watch 智能手表项目
基于STM32F103 + FreeRTOS实时操作系统的多功能智能手表工程

## 📌 项目简介
基于STM32主控，整合了OLED显示、RTC时钟、MPU6050运动传感器、锂电池管理、按键交互与PWM 蜂鸣器音乐播放模块，实现时间日期、多级菜单、计步、抬腕亮屏、熄屏保活、小恐龙跑酷游戏、电量检测等完整智能手表功能；基于FreeRTOS多任务调度实现业务并发运行，搭配状态机设计三级功耗管理方案；独立完成原理图、四层 PCB设计、元器件焊接与全流程软硬件调试。

## ⚙️ 硬件组成
- 主控：STM32F103C8T6
- 屏幕：OLED IIC显示屏
- 传感器：MPU6050
- 外设：按键、PWM蜂鸣器、锂电池充放电管理

## ✨ 实现功能
1. RTC实时时钟、年月日星期显示
2. 抬腕亮屏，亮屏超时熄屏的低功耗休眠功能
3. 计步功能
4. 多按键菜单切换（时间/菜单/各功能/设置页面）
5. 蜂鸣器音乐播放功能
6. 锂电池电压采集与电量显示

## 🛠️ 编译环境
- 开发软件：STM32CubeMX + MDK-Keil5
- 系统版本：CMSIS_V2
- 工程框架：HAL库开发

## 📷 实物展示
[智能手表成品]
<img width="1279" height="1395" alt="53ed2dd9a6582b314b72a57c67c894a5" src="https://github.com/user-attachments/assets/c9208ce5-6df3-445b-ac6d-6d39e091ab05" />
<img width="375" height="346" alt="image" src="https://github.com/user-attachments/assets/ec38e43a-a6c8-4e03-aa3e-ffa34c2e11b1" />
<img width="559" height="660" alt="image" src="https://github.com/user-attachments/assets/9598cde9-d984-424f-9b65-f568d2fabb57" />

