#ifndef __DRV_KEY_H__
#define __DRV_KEY_H__


#include "sys.h"

/*
K1 - PB1 - ����
K2 - PB2 - ����
K3 - PB10 - ����
K4 - PB11 - ����
*/

//#define USE_EXIT

#define K1 PBin(1)
#define K2 PBin(2)
#define K3 PBin(10)
#define K4 PBin(11)

void key_init(void);//IO��ʼ��
u8 key_get(u8);    //����ɨ�躯��


#endif
