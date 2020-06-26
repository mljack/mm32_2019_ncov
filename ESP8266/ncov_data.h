#ifndef __NCOV_DATA_H__
#define __NCOV_DATA_H__

#include "wifista.h"
#include "malloc.h"
#include "drv_uart2.h"
#include "delay.h"
#include "cJSON.h"

#include "string.h"
#include "stdio.h"
#include "time.h"

#include "config.h"


struct ncov_data{
    int confirmedCount; //�ۼ�ȷ��
    int curedCount;     //�ۼ�����
    int deadCount;      //�ۼ�����
    int confirmedIncr;  //����ȷ��
    int curedIncr;      //��������
    int deadIncr;        //��������
    char updateTime[20];
};

uint8_t build_ssl_connect(char *type, char *ip, char *port);
uint8_t get_ncov_api(char *api, uint8_t (*parse_fun)(struct ncov_data *dataChina, struct ncov_data *dataGlobal));
uint8_t parse_ncov_data(struct ncov_data *dataChina, struct ncov_data *dataGlobal);
uint8_t parse_ncov_news(void);
uint8_t parse_ncov_province(void);

#endif


