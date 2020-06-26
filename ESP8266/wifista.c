#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "wifista.h"
#include "drv_uart2.h"
#include "drv_uart1.h"
#include "drv_led.h"
#include "delay.h"
#include "config.h"


//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������UART2_RX_STA;
//     1,����UART2_RX_STA;
void atk_8266_at_response(uint8_t mode)
{
    if (UART2_RX_STA & 0X8000)     //���յ�һ��������
    {
        UART2_RX_BUF[UART2_RX_STA & 0X7FFF] = 0; //��ӽ�����
        printf("%s", UART2_RX_BUF); //���͵�����
        if (mode)UART2_RX_STA = 0;
    }
}
//ATK-ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
uint8_t *atk_8266_check_cmd(uint8_t *str)
{
    char *strx = 0;
    if (UART2_RX_STA & 0X8000)     //���յ�һ��������
    {
        UART2_RX_BUF[UART2_RX_STA & 0X7FFF] = 0; //��ӽ�����
        strx = strstr((const char *)UART2_RX_BUF, (const char *)str);
    }
    return (uint8_t *)strx;
}
//��ATK-ESP8266��������
//cmd:���͵������ַ���
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
uint8_t atk_8266_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t  waittime)
{
    uint8_t res = 0;
    UART2_RX_STA = 0;
    u2_printf("%s\r\n", cmd);   //��������
    if (ack && waittime)    //��Ҫ�ȴ�Ӧ��
    {
        while (--waittime)  //�ȴ�����ʱ
        {
            delay_ms(10);
            if (UART2_RX_STA & 0X8000) //���յ��ڴ���Ӧ����
            {
                if (atk_8266_check_cmd(ack))
                {
                    printf("���ͣ�%s ��Ӧ:%s\r\n", cmd, (uint8_t *)ack);
                    break;//�õ���Ч����
                }
                UART2_RX_STA = 0;
            }
        }
        if (waittime == 0)res = 1;
    }
    return res;
}
//��ATK-ESP8266����ָ������
//data:���͵�����(����Ҫ��ӻس���)
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)luojian
uint8_t atk_8266_send_data(uint8_t *data, uint8_t *ack, uint16_t  waittime)
{
    uint8_t res = 0;
    UART2_RX_STA = 0;
    u2_printf("%s", data);  //��������
    if (ack && waittime)    //��Ҫ�ȴ�Ӧ��
    {
        while (--waittime)  //�ȴ�����ʱ
        {
            delay_ms(10);
            if (UART2_RX_STA & 0X8000) //���յ��ڴ���Ӧ����
            {
                if (atk_8266_check_cmd(ack))break; //�õ���Ч����
                UART2_RX_STA = 0;
            }
        }
        if (waittime == 0)res = 1;
    }
    return res;
}
//ATK-ESP8266�˳�͸��ģʽ
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
uint8_t atk_8266_quit_trans(void)
{
    while ((UART2->CSR & UART_IT_TXIEN) == 0); //�ȴ����Ϳ�
    UART2->TDR = ('+' & (uint16_t)0x00FF);
    delay_ms(15);                   //���ڴ�����֡ʱ��(10ms)
    while ((UART2->CSR & UART_IT_TXIEN) == 0); //�ȴ����Ϳ�
    UART2->TDR = ('+' & (uint16_t)0x00FF);
    delay_ms(15);                   //���ڴ�����֡ʱ��(10ms)
    while ((UART2->CSR & UART_IT_TXIEN) == 0); //�ȴ����Ϳ�
    UART2->TDR = ('+' & (uint16_t)0x00FF);
    delay_ms(500);                  //�ȴ�500ms
    return atk_8266_send_cmd("AT", "OK", 20); //�˳�͸���ж�.
}

//��ȡATK-ESP8266ģ�������״̬
//����ֵ:0,δ����;1,���ӳɹ�.
uint8_t atk_8266_consta_check(void)
{
    uint8_t *p;
    uint8_t res;
    if (atk_8266_quit_trans())return 0;         //�˳�͸��
    atk_8266_send_cmd("AT+CIPSTATUS", ":", 50); //����AT+CIPSTATUSָ��,��ѯ����״̬
    p = atk_8266_check_cmd("+CIPSTATUS:");
    res = *p;                               //�õ�����״̬
    return res;
}

//��ȡClient ip��ַ
//ipbuf:ip��ַ���������
void atk_8266_get_wanip(uint8_t *ipbuf)
{
    uint8_t *p, *p1;
    if (atk_8266_send_cmd("AT+CIFSR", "OK", 50)) //��ȡWAN IP��ַʧ��
    {
        ipbuf[0] = 0;
        return;
    }
    p = atk_8266_check_cmd("\"");
    p1 = (uint8_t *)strstr((const char *)(p + 1), "\"");
    *p1 = 0;
    sprintf((char *)ipbuf, "%s", p + 1);
}

//����ESP8266λstaģʽ�������ӵ�·����
uint8_t atk_8266_wifista_config(void)
{
    uint8_t p[200];
	char str[200];
    uint8_t TryConnectTime = 1;
	printf("׼������\r\n");
    led_set(1, ON);
    while (atk_8266_send_cmd("AT", "OK", 20)) //���WIFIģ���Ƿ�����
    {
        atk_8266_quit_trans();//�˳�͸��
		delay_ms(1000);
        printf("δ��⵽ģ��\r\n");
    }
    atk_8266_send_cmd("AT+RESTORE", "OK", 200); //�ر�͸��ģʽ
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
    
    atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200); //�ر�͸��ģʽ
    while (atk_8266_send_cmd("ATE0", "OK", 20)); //�رջ���
    atk_8266_send_cmd("AT+CWMODE=1", "OK", 50);     //����WIFI STAģʽ
//    atk_8266_send_cmd("AT+RST", "OK", 20);      //DHCP�������ر�(��APģʽ��Ч)
	delay_ms(1000);         //��ʱ3S�ȴ������ɹ�
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
    //�������ӵ���WIFI��������/���ܷ�ʽ/����,�⼸��������Ҫ�������Լ���·�������ý����޸�!!
    atk_8266_send_cmd("AT+CIPMUX=0", "OK", 20); //0�������ӣ�1��������
    sprintf((char *)p, "AT+CWJAP=\"%s\",\"%s\"", wifista_ssid, wifista_password); //�������߲���:ssid,����

	sprintf((char *)str, "%s", wifista_ssid);

	while (atk_8266_send_cmd(p, "WIFI GOT IP", 300))
    {
        printf("WiFi����ʧ��,���ڳ��Ե� %d ������\r\n", TryConnectTime++);
        if (TryConnectTime >= 250)
            TryConnectTime = 0;
        delay_ms(100);
    };                  //����Ŀ��·����,���һ��IP
    printf("WiFi���ӳɹ�\r\n");
    led_set(2, ON);

    return 0;
}

