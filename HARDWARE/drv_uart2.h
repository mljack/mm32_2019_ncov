#ifndef __DRV_UART2_H
#define __DRV_UART2_H
#include "sys.h"

#define UART2_MAX_RECV_LEN     2000                    //�����ջ����ֽ���
#define UART2_MAX_SEND_LEN     600                 //����ͻ����ֽ���
#define UART2_RX_EN            1                   //0,������;1,����.

extern uint8_t  UART2_RX_BUF[UART2_MAX_RECV_LEN];      //���ջ���,���UART2_MAX_RECV_LEN�ֽ�
extern uint8_t  UART2_TX_BUF[UART2_MAX_SEND_LEN];      //���ͻ���,���UART2_MAX_SEND_LEN�ֽ�
extern uint16_t UART2_RX_STA;                          //��������״̬

void uart2_init(u32 bound);                //����2��ʼ��
void u2_printf(char *fmt, ...);


#endif













