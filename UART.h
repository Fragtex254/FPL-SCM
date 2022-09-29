#ifndef _UART_H_
#define _UART_H_

void Uart_Init(void);															//初始化串口通信
void Uart_Send_Byte(unsigned char c);							//发送一个Byte
unsigned char Uart_Receive_Byte();								//接收一个Byte

#endif
