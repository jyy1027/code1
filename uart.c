#include "stm32f10x.h"
#include <stdio.h>
#include "uart.h"

#pragma import(__use_no_semihosting_swi) //ȡ��������״̬

UartRecv u1recv;

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;

//fputc()��ʵ��printf()���ܵĵײ㺯����
//�޸����Ķ���Ϳ��Ըı�printf()�Ĺ���
int fputc(int ch, FILE *f) 
{
	//�滻Ϊ�������ض���ķ��͵��ֽں���
	uart1_sendByte(ch);
  return (ch);
}

int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
  //�滻Ϊ�������ض���ķ��͵��ֽں���
	uart1_sendByte(ch);
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
//UART1�ӿڳ�ʼ��
//PA9	--USART1_Tx -- �����������
//PA10--USART1_Rx -- ��������/����������
//���F407��F103��USART1�Ĺ���Ƶ�ʣ�72MHz���½�
//��Ӱ�쵽USART1->BRR�����ʼĴ���
//���⺯��USART_Init()�ѽ������ʼ�����з�װ
void uart1_init(u32 baudrate)
{
	GPIO_InitTypeDef   	gpio_InitStruct ;
	USART_InitTypeDef 	usart_InitStruct;
	/*��PA9��PA10���ó�USART1�Ĺ�������*/
	//����GPIOA
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA , ENABLE );
	//GPIOx�⺯����ʼ����PA9�������������PA10��������
	gpio_InitStruct.GPIO_Mode 	=	GPIO_Mode_AF_PP ;
	gpio_InitStruct.GPIO_Pin 		= GPIO_Pin_9 ;
	gpio_InitStruct.GPIO_Speed 	=	GPIO_Speed_2MHz ;
	GPIO_Init (GPIOA , &gpio_InitStruct);

	gpio_InitStruct.GPIO_Mode 	=	GPIO_Mode_IN_FLOATING ;
	gpio_InitStruct.GPIO_Pin 		= GPIO_Pin_10 ;
	gpio_InitStruct.GPIO_Speed 	=	GPIO_Speed_2MHz ;
	GPIO_Init (GPIOA , &gpio_InitStruct);
	
	/*����USARTx*/
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE);
	
	/*���������ã�ѡ����*/
	//USART_OverSampling8Cmd (USART1, ENABLE);
	
	/*USARTx�⺯����ʼ��*/
	usart_InitStruct.USART_BaudRate							=	baudrate ;
	usart_InitStruct.USART_HardwareFlowControl 	=	USART_HardwareFlowControl_None ;
	usart_InitStruct.USART_Mode 								= USART_Mode_Rx | USART_Mode_Tx ;
	usart_InitStruct.USART_Parity 							= USART_Parity_No ;
	usart_InitStruct.USART_StopBits 						= USART_StopBits_1 ;
	usart_InitStruct.USART_WordLength 					=	USART_WordLength_8b ;
	USART_Init (USART1, &usart_InitStruct);
	
	/*ʹ��USART����*/
	USART_Cmd (USART1, ENABLE);
}

//ͨ��uart1����һ֡���ݣ����ȴ��������
void uart1_sendByte(char data)
{
	USART_SendData (USART1, data);
	while(!USART_GetFlagStatus (USART1, USART_FLAG_TXE));
}

//�ȴ���׼���ý������ݣ�ͨ��uart1����һ֡���ݣ�
char uart1_recvByte(void)
{
	while(!USART_GetFlagStatus (USART1, USART_FLAG_RXNE));
	return USART_ReceiveData (USART1);
}

//��UART1�Ͻ���һ���ַ���
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
	while(data != '\n');//��ΪXCOM���ܷ���'\0'��ֻ�ܷ��͵�'\r' '\n'��
	*p = '\0';
#endif
}


void uart1_irq_config(void)
{
	NVIC_InitTypeDef nvic_InitStruct;
	//����uart1�ڡ���׼���������ݡ��¼��󣬷����ж�����; �жϵ����輶����
	USART_ITConfig (USART1, USART_IT_RXNE, ENABLE);    
	//NVIC�ڽ�������UART1���ж�����󣬷����ж�;       �жϵ��ں˼�����
	nvic_InitStruct.NVIC_IRQChannel										=	USART1_IRQn;
	nvic_InitStruct.NVIC_IRQChannelCmd								= ENABLE;
	nvic_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;//��ռ2λ
	nvic_InitStruct.NVIC_IRQChannelSubPriority 				= 1;//�Ƚ�2λ
	NVIC_Init (&nvic_InitStruct);
}
