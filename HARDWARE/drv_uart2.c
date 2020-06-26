#include "delay.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "drv_timer.h"
#include "drv_uart2.h"

uint8_t UART2_RX_BUF[UART2_MAX_RECV_LEN];              //���ջ���,���UART2_MAX_RECV_LEN���ֽ�.
uint8_t  UART2_TX_BUF[UART2_MAX_SEND_LEN];             //���ͻ���,���UART2_MAX_SEND_LEN�ֽ�
uint16_t UART2_RX_STA = 0;

//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���

void UART2_IRQHandler(void)
{
    uint8_t res;
    if (UART_GetITStatus(UART2, UART_IT_RXIEN)  != RESET)
    {
        res = UART_ReceiveData(UART2);
        UART_ClearITPendingBit(UART2, UART_IT_RXIEN);

        if ((UART2_RX_STA & (1 << 15)) == 0) //�������һ������,��û�б�����,���ٽ�����������
        {
            if (UART2_RX_STA < UART2_MAX_RECV_LEN) //�����Խ�������
            {
                TIM_SetCounter(TIM3, 0); //���������                         //���������
                if (UART2_RX_STA == 0)             //ʹ�ܶ�ʱ��3���ж�
                {
                    TIM_Cmd(TIM3, ENABLE); //ʹ�ܶ�ʱ��3
                }
                UART2_RX_BUF[UART2_RX_STA++] = res; //��¼���յ���ֵ
            }
            else
            {
                UART2_RX_STA |= 1 << 15;           //ǿ�Ʊ�ǽ������
            }
        }
    }
}

void uart2_init(u32 bound)
{
    GPIO_InitTypeDef gpio_initstruct;
    UART_InitTypeDef UART_initstruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART2, ENABLE);
    //PA2   TXD
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_initstruct.GPIO_Pin = GPIO_Pin_2;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstruct);
    //PA3   RXD
    gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_initstruct.GPIO_Pin = GPIO_Pin_3;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstruct);

    UART_initstruct.UART_BaudRate = bound;
    UART_initstruct.UART_HardwareFlowControl = UART_HardwareFlowControl_None;        //��Ӳ������
    UART_initstruct.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;                        //���պͷ���
    UART_initstruct.UART_Parity = UART_Parity_No;                                    //��У��
    UART_initstruct.UART_StopBits = UART_StopBits_1;                             //1λֹͣλ
    UART_initstruct.UART_WordLength = UART_WordLength_8b;                            //8λ����λ
    UART_Init(UART2, &UART_initstruct);

    NVIC_InitStruct.NVIC_IRQChannel = UART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;      //3
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;             //2
    NVIC_Init(&NVIC_InitStruct);
    
    UART_Cmd(UART2, ENABLE);                                                      //ʹ�ܴ���

    UART_ITConfig(UART2, UART_IT_RXIEN, ENABLE);

    tim3_init(1000 - 1, 9600 - 1);  //10ms�ж�
    UART2_RX_STA = 0;      //����
    TIM_Cmd(TIM3, DISABLE);         //�رն�ʱ��7
}

void u2_printf(char *fmt, ...)
{
    uint16_t i, j;
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char *)UART2_TX_BUF, fmt, ap);
    va_end(ap);
    i = strlen((const char *)UART2_TX_BUF);    //�˴η������ݵĳ���
    for (j = 0; j < i; j++)                     //ѭ����������
    {
        while ((UART2->CSR & UART_IT_TXIEN) == 0); //Send data cyclically
        UART_SendData(UART2, UART2_TX_BUF[j]);
    }
}


