#ifndef __MYI2C_H
#define __MYI2C_H

void MyI2C_Start(void);
void MyI2C_Stop(void);
void MyI2C_SendByte(uint8_t Byte);
uint8_t MyI2C_ReceiveByte(void);
void MyI2C_SendAck(uint8_t AckBit);
uint8_t MyI2C_ReceiveAck(void);

void MyI2C_ReadBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t len);


#endif
