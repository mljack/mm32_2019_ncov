#include "drv_lcd_gui.h"
#include "font.h"

extern void delay_ms(uint16_t nms);
extern struct ncov_data dataChina;
extern struct ncov_data dataGlobal;

/* ��ֱ�ߣ�ˮƽ��ֱ���� */
void gui_drawLine(uint8_t x0, uint8_t y0, uint8_t len, uint8_t direction, uint16_t color)
{
    uint8_t x, y;
    uint8_t size_x, size_y;

    if(direction == DIR_Y)
    {
        size_y = y0 + len;
        if(len > LCD_SIZE_X)
            size_y = LCD_SIZE_X;
        for(y = y0; y < size_y; y++)
            lcd_draw_point(x0, y, color);
    }
    else if(direction == DIR_X)
    {
        size_x = x0 + len;
        if(len > LCD_SIZE_Y)
            size_x = LCD_SIZE_Y;
        for(x = x0; x < size_x; x++)
            lcd_draw_point(x, y0, color);
    }
}

/* ������ */
void gui_drawRectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
    gui_drawLine(x0, y0, x1 - x0, DIR_X, color);
    gui_drawLine(x0, y0, y1 - y0, DIR_Y, color);
    gui_drawLine(x0, y1, x1 - x0, DIR_X, color);
    gui_drawLine(x1, y0, y1 - y0, DIR_Y, color);
}

/* ���� */
void gui_draw_point(uint8_t x0, uint8_t y0, uint8_t en, uint16_t color)
{
    if(en)
        lcd_draw_point(x0, y0, color);
}

/* ��ʾ6X8������ַ���idx��ѯ�ֿ����� */
void gui_show_F6X8_Char(uint8_t x0, uint8_t y0, uint8_t idx, uint16_t color)
{
    uint8_t x, y;
    for(y = y0; y < y0+8; y++)
    {
        for(x = x0; x < x0+6; x++)
        {
            if(F6X8[idx][y-y0] & (0x80 >> (x-x0)))
                lcd_draw_point(x, y, color);
        }
    }
}

/* ��ʾ6X8������ַ���: "06-24 11:56" */
void gui_show_F6X8_String(uint8_t x0, uint8_t y0, uint8_t *str, uint16_t color)
{
    uint8_t x = x0;
    while(*str)
    {
        if(*str >= '0' && *str <= '9')
            gui_show_F6X8_Char(x, y0, *str-'0', color);
        else if(*str == '-')    //11
            gui_show_F6X8_Char(x, y0, 11, color);
        else if(*str == ':')    //10
            gui_show_F6X8_Char(x, y0, 10, color);
        else if(*str == ' ')
            gui_show_F6X8_Char(x, y0, 12, color);
        str++;
        x += 6;
    }
}

/* ��ʾ8X16������ַ�, idx��ѯ�ֿ����� */
void gui_show_F8X16_Char(uint8_t x0, uint8_t y0, uint8_t idx, uint16_t color)
{
    uint8_t x, y;

    for(y = y0; y < y0 + 16; y++)
    {
        for(x = x0; x < x0 + 8; x++)
        {
//            gui_draw_point(x, y, F8X16[idx][y-y0] & (0x80>>(x-x0)), color);
            if(F8X16[idx][y - y0] & (0x80 >> (x - x0)))
                lcd_draw_point(x, y, color);
        }
    }
}

/* ��ʾ8X16������ַ���: "06-24 11:56" */
void gui_show_F8X16_String(uint8_t x0, uint8_t y0, uint8_t *str, uint16_t color)
{
    uint8_t x = x0;
    while(*str)
    {
        if(*str >= '0' && *str <= '9')
            gui_show_F8X16_Char(x, y0, *str-'0', color);
        else if(*str == '-')    //11
            gui_show_F8X16_Char(x, y0, 11, color);
        else if(*str == ':')    //10
            gui_show_F8X16_Char(x, y0, 10, color);
        else if(*str == ' ')
            gui_show_F8X16_Char(x, y0, 12, color);
        str++;
        x += 8;
    }
}

uint32_t my_pow(uint8_t m, uint8_t n)
{
    u32 result = 1;
    while(n--)result *= m;
    return result;
}

/* ��ȡ����λ�������� */
void my_getNum(long num, uint8_t *cnt, bool *minusFlag)
{
    if(num < 0)
    {
        *minusFlag = TRUE;
        *cnt = (int)log10(-num) + 1;
    }
    else
    {
        *minusFlag = FALSE;
        *cnt = (int)log10(num) + 1;    
    }
}

/* ��ʾ1��Nλ�����֣�֧���з����� */
void gui_show_num(uint16_t x, uint16_t y, long num, uint16_t color)
{
    uint8_t len;
    bool minusFlag;
    long num_p;
    
    uint8_t t, temp;
    uint8_t enshow = 0;
    /* ��ȡ����λ����������־ */
    my_getNum(num, &len, &minusFlag);
    num_p = (minusFlag) ? -num : num;
    
    for(t = 0; t < len; t++)
    {
        temp = (num_p / my_pow(10, len - t - 1)) % 10;
        if(enshow == 0 && t < (len - 1))
        {
            if(temp == 0) //��λΪ0����ʾ�հ�
            {
                if(minusFlag == TRUE)
                    gui_show_F8X16_Char(x + 8 * t + 8, y, 12, color); //��ʾ�հ�
                else
                    gui_show_F8X16_Char(x + 8 * t, y, 12, color); //��ʾ�հ�
                continue;
            }
            else
                enshow = 1;
        }
        if(minusFlag == TRUE)
            gui_show_F8X16_Char(x + 8 * t + 8, y, temp, WHITE);
        else 
            gui_show_F8X16_Char(x + 8 * t, y, temp, WHITE);
    }
    if(minusFlag == TRUE)
        gui_show_F8X16_Char(x, y, 11, WHITE);       //"-"
}

/* ��ʾ16X16������ַ� */
void gui_show_F16X16_Char(uint8_t x0, uint8_t y0, uint8_t idx, uint16_t color)
{
    uint8_t x, y;

    for(y = y0; y < y0 + 16; y++)
    {
        for(x = x0; x < x0 + 16; x++)
        {
            if(x - x0 < 8)      //0-7
                gui_draw_point(x, y, F16X16[idx][(y - y0) * 2] & (0x80 >> (x - x0)), color);
            else
                gui_draw_point(x, y, F16X16[idx][(y - y0) * 2 + 1] & (0x80 >> (x - x0 - 8)), color);
        }
    }
}

void gui_show_dataChina(void)
{
    lcd_clear(BLACK);
    /*�����¹���������*/
    gui_show_F16X16_Char(16, 5, 6, WHITE);
    gui_show_F16X16_Char(32, 5, 7, WHITE);
    gui_show_F16X16_Char(48, 5, 0, WHITE);
    gui_show_F16X16_Char(64, 5, 1, WHITE);
    gui_show_F16X16_Char(80, 5, 2, WHITE);
    gui_show_F16X16_Char(96, 5, 3, WHITE);
    gui_show_F16X16_Char(112, 5, 4, WHITE);
    gui_show_F16X16_Char(128, 5, 5, WHITE);

    /*ȷ��*/
    gui_show_F16X16_Char(0, 46, 8, WHITE);
    gui_show_F16X16_Char(0 + 16, 46, 9, WHITE);
    
    /*����*/
    gui_show_F16X16_Char(0, 69, 12, WHITE);
    gui_show_F16X16_Char(0 + 16, 69, 13, WHITE);

    /*����*/
    gui_show_F16X16_Char(0, 92, 10, WHITE);
    gui_show_F16X16_Char(0 + 16, 92, 11, WHITE);

    /*�ۼ�*/
    gui_show_F16X16_Char(50, 25, 16, WHITE);
    gui_show_F16X16_Char(50 + 16, 25, 17, WHITE);

    /*����*/
    gui_show_F16X16_Char(114, 25, 18, WHITE);
    gui_show_F16X16_Char(114 + 16, 25, 19, WHITE);

    /* ���ŵ������� */
    gui_drawLine(34, 25, 85, DIR_Y, WHITE);
    gui_drawLine(97, 25, 85, DIR_Y, WHITE);

    /* ���ŵ�4���� */
    gui_drawLine(0, 44, 160, DIR_X, WHITE);
    gui_drawLine(0, 65, 160, DIR_X, WHITE);
    gui_drawLine(0, 88, 160, DIR_X, WHITE);
    gui_drawLine(0, 110, 160, DIR_X, WHITE);

    /*�ۼ�ȷ��*/
    gui_show_num(38, 46, dataChina.confirmedCount, WHITE);
    /*�ۼ�����*/
    gui_show_num(38, 69, dataChina.curedCount, WHITE);
    /*�ۼ�����*/
    gui_show_num(38, 92, dataChina.deadCount, WHITE);
    
    /*����ȷ��*/
    gui_show_num(103, 46, dataChina.confirmedIncr, WHITE);
    /*��������*/
    gui_show_num(103, 69, dataChina.curedIncr, WHITE);
    /*��������*/
    gui_show_num(103, 92, dataChina.deadIncr, WHITE);
 
    gui_show_F16X16_Char(18, 111, 20, WHITE);
    gui_show_F16X16_Char(18+16, 111, 21, WHITE);
    gui_show_F16X16_Char(18+32, 111, 22, WHITE);
    
    gui_show_F8X16_String(70, 111, (uint8_t *)dataChina.updateTime, WHITE);
}

void gui_show_dataGlobal(void)
{
    lcd_clear(BLACK);
    /*ȫ���¹���������*/
    gui_show_F16X16_Char(16, 5, 14, WHITE);
    gui_show_F16X16_Char(32, 5, 15, WHITE);
    gui_show_F16X16_Char(48, 5, 0, WHITE);
    gui_show_F16X16_Char(64, 5, 1, WHITE);
    gui_show_F16X16_Char(80, 5, 2, WHITE);
    gui_show_F16X16_Char(96, 5, 3, WHITE);
    gui_show_F16X16_Char(112, 5, 4, WHITE);
    gui_show_F16X16_Char(128, 5, 5, WHITE);

    /*ȷ��*/
    gui_show_F16X16_Char(0, 46, 8, WHITE);
    gui_show_F16X16_Char(0 + 16, 46, 9, WHITE);
    
    /*����*/
    gui_show_F16X16_Char(0, 69, 12, WHITE);
    gui_show_F16X16_Char(0 + 16, 69, 13, WHITE);

    /*����*/
    gui_show_F16X16_Char(0, 92, 10, WHITE);
    gui_show_F16X16_Char(0 + 16, 92, 11, WHITE);

    /*�ۼ�*/
    gui_show_F16X16_Char(50, 25, 16, WHITE);
    gui_show_F16X16_Char(50 + 16, 25, 17, WHITE);

    /*����*/
    gui_show_F16X16_Char(114, 25, 18, WHITE);
    gui_show_F16X16_Char(114 + 16, 25, 19, WHITE);

    /* ���ŵ������� */
    gui_drawLine(34, 25, 85, DIR_Y, WHITE);
    gui_drawLine(97, 25, 85, DIR_Y, WHITE);

    /* ���ŵ�4���� */
    gui_drawLine(0, 44, 160, DIR_X, WHITE);
    gui_drawLine(0, 65, 160, DIR_X, WHITE);
    gui_drawLine(0, 88, 160, DIR_X, WHITE);
    gui_drawLine(0, 110, 160, DIR_X, WHITE);
    /*�ۼ�ȷ��*/
    gui_show_num(38, 46, dataGlobal.confirmedCount, WHITE);
    /*�ۼ�����*/
    gui_show_num(38, 69, dataGlobal.curedCount, WHITE);
    /*�ۼ�����*/
    gui_show_num(38, 92, dataGlobal.deadCount, WHITE);
    
    /*����ȷ��*/
    gui_show_num(103, 46, dataGlobal.confirmedIncr, WHITE);
    /*��������*/
    gui_show_num(103, 69, dataGlobal.curedIncr, WHITE);
    /*��������*/
    gui_show_num(103, 92, dataGlobal.deadIncr, WHITE);
     
    gui_show_F16X16_Char(18, 111, 20, WHITE);
    gui_show_F16X16_Char(18+16, 111, 21, WHITE);
    gui_show_F16X16_Char(18+32, 111, 22, WHITE);

    gui_show_F8X16_String(70, 111, (uint8_t *)dataChina.updateTime, WHITE);
}
