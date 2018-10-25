#include "my_math.h"
#include <ctype.h>
/*********************************************************************
*��    �ܣ����ַ���ת����������
*��ڲ�����ָ���ַ������ַ���ָ��
*���ڲ�����������ֵ
*********************************************************************/
int my_atoi(char* pstr)  
{  
    int Ret_Integer = 0;  
    int Integer_sign = 1;  
      
    /* 
    * �ж�ָ���Ƿ�Ϊ�� 
    */  
    if(pstr == NULL)  
    {  
         
        return 0;  
    }  
      
    /* 
    * ����ǰ��Ŀո��ַ� 
    */  
    while(isspace(*pstr) != 0)  
    {
        pstr++;
    } 
      
    /* 
    * �ж������� 
    * ��������ţ�ָ��ָ����һ���ַ� 
    * ����Ƿ��ţ��ѷ��ű��ΪInteger_sign��-1��Ȼ���ٰ�ָ��ָ����һ���ַ� 
    */  
    if(*pstr == '-')
    {  
        Integer_sign = -1;
    }  
    if(*pstr == '-' || *pstr == '+')  
    {  
        pstr++;
    }
      
    /* 
    * �������ַ������ת�����������������ת���õ���������Ret_Integer 
    */ 
    while(*pstr >= '0' && *pstr <= '9')  
    {
        Ret_Integer = Ret_Integer * 10 + *pstr - '0';
        pstr++;
    }
    Ret_Integer = Integer_sign * Ret_Integer;  
    
    return Ret_Integer;
}

/*********************************************************************
*��    �ܣ������ֽ�ת����һ��ʮ��������
*��ڲ�����
*���ڲ�����
*********************************************************************/
u8 CharToHex(u8 high,u8 low)
{
    u8 char_temp[2]={0}; 
    u8 Hex=0;
    if((high>0x29)&&(high<0x3A))
    {				char_temp[0]=high-48;			}
    else if((high>0x40)&&(high<0x5B))
    {				char_temp[0]=high-55;			}
    else {char_temp[0]=0;}
    if((low>0x29)&&(low<0x3A))
    {				char_temp[1]=low-48;			}
    else if((low>0x40)&&(low<0x5B))
    {				char_temp[1]=low-55;			}
    else {char_temp[1]=0;}
    Hex=(char_temp[0]<<4)+char_temp[1];		//ת������ֵ
    return Hex;
}

/*********************************************************************
*��    �ܣ�һ��ʮ��������ת���������ֽ�
*��ڲ�����һ��ʮ��������
*���ڲ�������������char_temp[2]�У���λ��ǰ
*********************************************************************/
u8 char_temp[2]={0}; 
void HexToChar(u8 Hex)
{
    char_temp[0]=0;
    char_temp[1]=0;
    if(((Hex&0x0f)>=0)&&((Hex&0x0f)<10))
    {        char_temp[1]=(Hex&0x0f)+48;    }
    else if(((Hex&0x0f)>9)&&((Hex&0x0f)<16))
    {        char_temp[1]=(Hex&0x0f)+55;    }
    if((((Hex>>4)&0x0f)>=0)&&(((Hex>>4)&0x0f)<10))
    {        char_temp[0]=((Hex>>4)&0x0f)+48;    }
    else if((((Hex>>4)&0x0f)>9)&&(((Hex>>4)&0x0f)<16))
    {        char_temp[0]=((Hex>>4)&0x0f)+55;    }
}

