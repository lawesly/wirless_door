#ifndef _MY_MATH_H
#define _MY_MATH_H
#include "sys.h"

#define NULL 0

int my_atoi(char* pstr) ;
u8 CharToHex(u8 high,u8 low);/*�����ֽ�ת����һ��ʮ��������*/
extern u8 char_temp[2];       /*һ��ʮ��������ת���������ֽ�*/
void HexToChar(u8 Hex);       /*��������char_temp[2]�У���λ��ǰ*/


#endif


