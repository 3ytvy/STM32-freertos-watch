#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH			3		//UTF-8编码格式给3，GB2312编码格式给2

/*字模基本单元*/
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[32];						//字模数据
} ChineseCell_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];
extern const uint8_t OLED_F12x24[][36];
extern const uint8_t OLED_F14x28[][56];


/*汉字字模数据声明*/
extern const ChineseCell_t OLED_CF16x16[];

/*图像数据声明*/
extern const uint8_t Diode[];
extern const uint8_t Return[];
extern const uint8_t Menu_Graph[][128];
extern const uint8_t Frame[];
extern const uint8_t Ground[];
extern const uint8_t Barrier[][48];
extern const uint8_t Cloud[];
extern const uint8_t Dino[][48];
extern const uint8_t Frame[];
extern const uint8_t dui[];
extern const uint8_t cuo[];
extern const uint8_t battery[];
extern const uint8_t Process_Frame[];
extern const uint8_t Step_ico_samll[];
extern const uint8_t Music_ico_small[];

extern const uint8_t Menu_ico[];
extern const uint8_t Setting_ico[];


/*按照上面的格式，在这个位置加入新的图像数据声明*/
//...

#endif


/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
