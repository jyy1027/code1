#ifndef	_UART_H_
#define _UART_H_

#define XCOM 

typedef struct {
	char buf[1024];
	u32  count;
	u8	 flag;
} UartRecv;


void uart1_init(u32 baudrate);
void uart1_sendByte(char data);
char uart1_recvByte(void);
void uart1_recvString(char *p);
void uart1_irq_config(void);

#endif