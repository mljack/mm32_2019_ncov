#include "drv_uart1.h"

//C��
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

void uart1_init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    UART_InitTypeDef UART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_UART1 | RCC_APB2Periph_GPIOA, ENABLE);	

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    UART_InitStructure.UART_BaudRate = baudrate; 
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
    UART_InitStructure.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;
    UART_InitStructure.UART_Parity = UART_Parity_No;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_Init(UART1, &UART_InitStructure); 

    NVIC_InitStructure.NVIC_IRQChannel = UART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     
    NVIC_Init(&NVIC_InitStructure); 
    
    UART_ITConfig(UART1, UART_IT_RXIEN, ENABLE);
    UART_Cmd(UART1, ENABLE);               
}

void MyUartPrintf(UART_TypeDef *UARTx, char *fmt, ...)
{
    unsigned char UARTPrintfBuf[296];
    va_list ap;
    unsigned char *pStr = UARTPrintfBuf;

    va_start(ap, fmt);
    vsnprintf((char *)UARTPrintfBuf, sizeof(UARTPrintfBuf), fmt, ap);							//��ʽ��
    va_end(ap);

    while(*pStr != '\0')
    {
        UARTx->TDR = ((*pStr++) & (uint16_t)0x00FF);
        while ((UARTx->CSR & UART_IT_TXIEN) == 0); //Send data cyclically
    }
}
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
    int handle;

};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
_sys_exit(int x)
{
    x = x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
    while ((UART1->CSR & UART_IT_TXIEN) == 0); //Send data cyclically
    UART1->TDR = (ch & (uint16_t)0x00FF);
    return ch;
}

void UART1_IRQHandler(void)                 
{
    uint8_t dat;
    if (UART_GetITStatus(UART1, UART_IT_RXIEN)  != RESET)
    {
        dat = UART_ReceiveData(UART1);
        UART_ClearITPendingBit(UART1, UART_IT_RXIEN);
    }
}

