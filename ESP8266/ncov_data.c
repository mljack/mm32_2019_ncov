#include "ncov_data.h"
#include "drv_led.h"

struct ncov_data dataChina;
struct ncov_data dataGlobal;

uint8_t build_ssl_connect(char *type, char *ip, char *port)
{
    char str_tmp[200];
    uint8_t TryConnectTime = 0;

    atk_8266_send_cmd("AT+CIPMUX=0", "OK", 100);
    if(strcmp(type, "SSL")==0)
        atk_8266_send_cmd("AT+CIPSSLSIZE=4096", "OK", 100);

    sprintf((char *)str_tmp, "AT+CIPSTART=\"%s\",\"%s\",%s", type, ip, port);

    while (atk_8266_send_cmd((uint8_t *)str_tmp, "OK", 200))
    {
        printf("api����ʧ��,���ڳ��Ե� %d ������\r\n", TryConnectTime++);
        delay_ms(100);
        if (TryConnectTime >= 10)
        {
            return 1;
        }
    };
    led_set(3, ON);

    delay_ms(300);
    atk_8266_send_cmd("AT+CIPMODE=1", "OK", 100);    //����ģʽΪ��͸��

    UART2_RX_STA = 0;
    atk_8266_send_cmd("AT+CIPSEND", "OK", 100);       //��ʼ͸��
    return 0;
}

/* �����ڶ��api������ָ���������� */
uint8_t get_ncov_api(char *api, uint8_t (*parse_fun)(struct ncov_data *dataChina, struct ncov_data *dataGlobal))
{
    printf("��ȡ��������:GET %s\r\n", api);
    u2_printf("GET %s\n\n", api);

    delay_ms(20);
    UART2_RX_STA = 0;
    delay_ms(1000);

    if (UART2_RX_STA & 0X8000)
    {
        UART2_RX_BUF[UART2_RX_STA & 0X7FFF] = 0; //��ӽ�����
    }

//    printf("���ݳ���:%d\r\n%s\r\n", strlen((const char *)UART2_RX_BUF), UART2_RX_BUF);	//JSONԭʼ����
    parse_fun(&dataChina, &dataGlobal);
    
    UART2_RX_STA = 0;
    memset(UART2_RX_BUF, 0, sizeof(UART2_RX_BUF));
    led_all_off();

    return 0;
}

uint8_t parse_ncov_data(struct ncov_data *dataChina, struct ncov_data *dataGlobal)
{
    cJSON *root;
    cJSON *results_arr;
    cJSON *results;
    cJSON *globalStatistics;
    
    time_t updateTime;
    struct tm *time;
    
    printf("��ʼ������������, ���ݳ���:%d\r\n", strlen((const char *)UART2_RX_BUF));
    root = cJSON_Parse((const char *)UART2_RX_BUF);
    if(!root)
    {
        printf("�������ݸ�ʽ����\r\n");
        return 0;
    }
    else
    {
        printf("�������ݸ�ʽ��ȷ����ʼ����\r\n");

        results_arr = cJSON_GetObjectItem(root, "results");
        if(results_arr)
        {
            results = cJSON_GetArrayItem(results_arr, 0);
            if(results)
            {
                dataChina->confirmedCount = cJSON_GetObjectItem(results, "currentConfirmedCount")->valueint;
                dataChina->confirmedIncr = cJSON_GetObjectItem(results, "currentConfirmedIncr")->valueint;
                dataChina->curedCount = cJSON_GetObjectItem(results, "curedCount")->valueint;
                dataChina->curedIncr = cJSON_GetObjectItem(results, "curedIncr")->valueint;
                dataChina->deadCount = cJSON_GetObjectItem(results, "deadCount")->valueint;
                dataChina->deadIncr = cJSON_GetObjectItem(results, "deadIncr")->valueint;

                printf("------------��������-------------\r\n");
                printf("�ִ�ȷ��: %-7d, ������:%-5d\r\n", 
                    dataChina->confirmedCount, dataChina->confirmedIncr);
                printf("�ۼ�����: %-7d, ������:%-5d\r\n", 
                    dataChina->curedCount, dataChina->curedIncr);
                printf("�ۼ�����: %-7d, ������:%-5d\r\n", 
                    dataChina->deadCount, dataChina->deadIncr);

                globalStatistics = cJSON_GetObjectItem(results, "globalStatistics");
                if(globalStatistics)
                {
                    dataGlobal->confirmedCount = cJSON_GetObjectItem(globalStatistics, "currentConfirmedCount")->valueint;
                    dataGlobal->confirmedIncr = cJSON_GetObjectItem(globalStatistics, "currentConfirmedIncr")->valueint;
                    dataGlobal->curedCount = cJSON_GetObjectItem(globalStatistics, "curedCount")->valueint;
                    dataGlobal->curedIncr = cJSON_GetObjectItem(globalStatistics, "curedIncr")->valueint;
                    dataGlobal->deadCount = cJSON_GetObjectItem(globalStatistics, "deadCount")->valueint;
                    dataGlobal->deadIncr = cJSON_GetObjectItem(globalStatistics, "deadIncr")->valueint;

                    /*
                    ��06-24��api�ӿڲ����ṩȫ��������������ݣ�ֻ�ṩ�ۼ�����
                    */
                    printf("------------ȫ������-------------\r\n");
                    printf("�ִ�ȷ��: %-7d, ������:%-5d\r\n", 
                        dataGlobal->confirmedCount, dataGlobal->confirmedIncr);
                    printf("�ۼ�����: %-7d, ������:%-5d\r\n",
                        dataGlobal->deadCount, dataGlobal->curedIncr);
                    printf("�ۼ�����: %-7d, ������:%-5d\r\n", 
                        dataGlobal->curedCount, dataGlobal->deadIncr);
                }
                
                /* ���뼶ʱ���ת�ַ��� */
                updateTime = (time_t )(cJSON_GetObjectItem(results, "updateTime")->valuedouble/1000);
                updateTime += 8*60*60; /* UTC8У�� */
                time = localtime(&updateTime);
                /* ��ʽ��ʱ�� */
                strftime(dataChina->updateTime, 20, "%m-%d %H:%M", time);
                printf("������:%s\r\n", dataChina->updateTime);/* 06-24 11:21 */
                led_set(4, ON);
            }
        }
    }

//    cJSON_Delete(results);
//    cJSON_Delete(results_arr);
    cJSON_Delete(root);

    printf("*********�������*********\r\n");
    return 1;
}
