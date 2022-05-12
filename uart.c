#include "stm32f10x.h"
#include <stdio.h>
#include "uart.h"

#pragma import(__use_no_semihosting_swi) //取消半主机状态

UartRecv u1recv;

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;

//fputc()是实现printf()功能的底层函数，
//修改它的定义就可以改变printf()的功能
int fputc(int ch, FILE *f) 
{
	//替换为开发者重定向的发送单字节函数
	uart1_sendByte(ch);
  return (ch);
}

int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
  //替换为开发者重定向的发送单字节函数
	uart1_sendByte(ch);
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
//UART1接口初始化
//PA9	--USART1_Tx -- 复用推挽输出
//PA10--USART1_Rx -- 浮空输入/带上拉输入
//相比F407，F103的USART1的工作频率（72MHz）下降
//会影响到USART1->BRR波特率寄存器
//但库函数USART_Init()已将波特率计算进行封装
void uart1_init(u32 baudrate)
{
	GPIO_InitTypeDef   	gpio_InitStruct ;
	USART_InitTypeDef 	usart_InitStruct;
	/*将PA9和PA10复用成USART1的功能引脚*/
	//激活GPIOA
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA , ENABLE );
	//GPIOx库函数初始化：PA9复用推挽输出，PA10浮空输入
	gpio_InitStruct.GPIO_Mode 	=	GPIO_Mode_AF_PP ;
	gpio_InitStruct.GPIO_Pin 		= GPIO_Pin_9 ;
	gpio_InitStruct.GPIO_Speed 	=	GPIO_Speed_2MHz ;
	GPIO_Init (GPIOA , &gpio_InitStruct);

	gpio_InitStruct.GPIO_Mode 	=	GPIO_Mode_IN_FLOATING ;
	gpio_InitStruct.GPIO_Pin 		= GPIO_Pin_10 ;
	gpio_InitStruct.GPIO_Speed 	=	GPIO_Speed_2MHz ;
	GPIO_Init (GPIOA , &gpio_InitStruct);
	
	/*激活USARTx*/
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE);
	
	/*过采样配置（选做）*/
	//USART_OverSampling8Cmd (USART1, ENABLE);
	
	/*USARTx库函数初始化*/
	usart_InitStruct.USART_BaudRate							=	baudrate ;
	usart_InitStruct.USART_HardwareFlowControl 	=	USART_HardwareFlowControl_None ;
	usart_InitStruct.USART_Mode 								= USART_Mode_Rx | USART_Mode_Tx ;
	usart_InitStruct.USART_Parity 							= USART_Parity_No ;
	usart_InitStruct.USART_StopBits 						= USART_StopBits_1 ;
	usart_InitStruct.USART_WordLength 					=	USART_WordLength_8b ;
	USART_Init (USART1, &usart_InitStruct);
	
	/*使能USART外设*/
	USART_Cmd (USART1, ENABLE);
}

//通过uart1发送一帧数据，并等待发送完毕
void uart1_sendByte(char data)
{
	USART_SendData (USART1, data);
	while(!USART_GetFlagStatus (USART1, USART_FLAG_TXE));
}

//等待已准备好接收数据，通过uart1接收一帧数据，
char uart1_recvByte(void)
{
	while(!USART_GetFlagStatus (USART1, USART_FLAG_RXNE));
	return USART_ReceiveData (USART1);
}

//从UART1上接收一个字符串
void uart1_recvString(char *p)
{
	char data;
#ifndef	XCOM
	do{
		data = uart1_recvByte();
		*p = data;
		p++;
	}
	while(data != '\0');//data // data != 0
#else
	do{
		data = uart1_recvByte();
		*p = data;
		p++;
	}
	while(data != '\n');//因为XCOM不能发送'\0'，只能发送到'\r' '\n'。
	*p = '\0';
#endif
}


void uart1_irq_config(void)
{
	NVIC_InitTypeDef nvic_InitStruct;
	//允许uart1在“已准备接收数据”事件后，发送中断请求; 中断的外设级配置
	USART_ITConfig (USART1, USART_IT_RXNE, ENABLE);    
	//NVIC在接收来自UART1的中断请求后，发生中断;       中断的内核级配置
	nvic_InitStruct.NVIC_IRQChannel										=	USART1_IRQn;
	nvic_InitStruct.NVIC_IRQChannelCmd								= ENABLE;
	nvic_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;//抢占2位
	nvic_InitStruct.NVIC_IRQChannelSubPriority 				= 1;//比较2位
	NVIC_Init (&nvic_InitStruct);
}
