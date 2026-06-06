#include "stm32f1xx_hal.h"
#include "MyDelay.h"


/*
SDA --- PB11
SCL --- PB10
*/

/*引脚配置层*/

/**
  * 函    数：I2C写SCL引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SCL的电平，范围0~1
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置SCL为低电平，当BitValue为1时，需要置SCL为高电平
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
	if(BitValue)
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
	}
	
	Delay_us(10);												//延时10us，防止时序频率超过要求
}

/**
  * 函    数：I2C写SDA引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SDA的电平，范围0~1
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置SDA为低电平，当BitValue为1时，需要置SDA为高电平
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
	if(BitValue)
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
	}
	Delay_us(10);												//延时10us，防止时序频率超过要求
}

/**
  * 函    数：I2C读SDA引脚电平
  * 参    数：无
  * 返 回 值：协议层需要得到的当前SDA的电平，范围0~1
  * 注意事项：此函数需要用户实现内容，当前SDA为低电平时，返回0，当前SDA为高电平时，返回1
  */
uint8_t MyI2C_R_SDA(void)
{
	uint8_t BitValue;
	BitValue = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11);
	Delay_us(10);												//延时10us，防止时序频率超过要求
	return BitValue;											//返回SDA电平
}

/**
  * 函    数：I2C初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：就是初始化引脚PB10和PB11，用CUBEMX生成了初始化代码
//////  */
//////void MyI2C_Init(void)
//////{
//////	/*开启时钟*/
//////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟
//////	
//////	/*GPIO初始化*/
//////	GPIO_InitTypeDef GPIO_InitStructure;
//////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
//////	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
//////	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//////	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB10和PB11引脚初始化为开漏输出
//////	
//////	/*设置默认电平*/
//////	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);			//设置PB10和PB11引脚初始化后默认为高电平（释放总线状态）
//////}

/*协议层*/

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  */
void MyI2C_Start(void)
{
	MyI2C_W_SDA(1);							//释放SDA，确保SDA为高电平
	MyI2C_W_SCL(1);							//释放SCL，确保SCL为高电平
	MyI2C_W_SDA(0);							//在SCL高电平期间，拉低SDA，产生起始信号
	MyI2C_W_SCL(0);							//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  */
void MyI2C_Stop(void)
{
	MyI2C_W_SDA(0);							//拉低SDA，确保SDA为低电平
	MyI2C_W_SCL(1);							//释放SCL，使SCL呈现高电平
	MyI2C_W_SDA(1);							//在SCL高电平期间，释放SDA，产生终止信号
}

/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i ++)				//循环8次，主机依次发送数据的每一位
	{
		/*两个!可以对数据进行两次逻辑取反，作用是把非0值统一转换为1，即：!!(0) = 0，!!(非0) = 1*/
		MyI2C_W_SDA(!!(Byte & (0x80 >> i)));//使用掩码的方式取出Byte的指定一位数据并写入到SDA线
		MyI2C_W_SCL(1);						//释放SCL，从机在SCL高电平期间读取SDA
		MyI2C_W_SCL(0);						//拉低SCL，主机开始发送下一位数据
	}
}

/**
  * 函    数：I2C接收一个字节
  * 参    数：无
  * 返 回 值：接收到的一个字节数据，范围：0x00~0xFF
  */
uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;					//定义接收的数据，并赋初值0x00，此处必须赋初值0x00，后面会用到
	MyI2C_W_SDA(1);							//接收前，主机先确保释放SDA，避免干扰从机的数据发送
	for (i = 0; i < 8; i ++)				//循环8次，主机依次接收数据的每一位
	{
		MyI2C_W_SCL(1);						//释放SCL，主机机在SCL高电平期间读取SDA
		if (MyI2C_R_SDA()){Byte |= (0x80 >> i);}	//读取SDA数据，并存储到Byte变量
													//当SDA为1时，置变量指定位为1，当SDA为0时，不做处理，指定位为默认的初值0
		MyI2C_W_SCL(0);						//拉低SCL，从机在SCL低电平期间写入SDA
	}
	return Byte;							//返回接收到的一个字节数据
}

/**
  * 函    数：I2C发送应答位
  * 参    数：Byte 要发送的应答位，范围：0~1，0表示应答，1表示非应答
  * 返 回 值：无
  */
void MyI2C_SendAck(uint8_t AckBit)
{
	MyI2C_W_SDA(AckBit);					//主机把应答位数据放到SDA线
	MyI2C_W_SCL(1);							//释放SCL，从机在SCL高电平期间，读取应答位
	MyI2C_W_SCL(0);							//拉低SCL，开始下一个时序模块
}

/**
  * 函    数：I2C接收应答位
  * 参    数：无
  * 返 回 值：接收到的应答位，范围：0~1，0表示应答，1表示非应答
  */
uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit;							//定义应答位变量
	MyI2C_W_SDA(1);							//接收前，主机先确保释放SDA，避免干扰从机的数据发送
	MyI2C_W_SCL(1);							//释放SCL，主机机在SCL高电平期间读取SDA
	AckBit = MyI2C_R_SDA();					//将应答位存储到变量里
	MyI2C_W_SCL(0);							//拉低SCL，开始下一个时序模块
	return AckBit;							//返回定义应答位变量
}







/**
  * 函    数：I2C从指定寄存器开始，连续读取多个字节
  * 参    数：addr - 设备I2C地址（7位）
							reg - 寄存器起始地址（8位）
							buffer - 存放读取数据的缓冲区（地址指针）
							len - 要读取的字节数量（16位）
  * 返 回 值：无
  */
void MyI2C_ReadBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t len)
{
	uint16_t i;
	
	// #1.发送设备地址+写，发送指定寄存器地址
	MyI2C_Start();
	MyI2C_SendByte(addr<<1);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(reg);
	MyI2C_ReceiveAck();
	
	// #2.重新开始，发送设备地址+读
	MyI2C_Start();
	MyI2C_SendByte(addr<<1 | 0x01);
	MyI2C_ReceiveAck();
	
	// #3.连续读取数据
	for(i=0;i<len;i++)
	{
		buffer[i]=MyI2C_ReceiveByte();
		if(i<len-1)
		{
			MyI2C_SendAck(0);//应答，继续读
		}
		else
		{
			MyI2C_SendAck(1);//非应答，结束
		}
		
	}
		// #4.停止
		MyI2C_Stop();
	
}
