#ifndef _FINGERPRINT_H
#define _FINGERPRINT_H

#include <stdio.h>
#include "sys.h"


#define TRUE  1
#define FALSE 0

//����Ӧ����Ϣ����
#define ACK_SUCCESS      		0x00		//�����ɹ�
#define ACK_FAIL          		0x01		//����ʧ��
#define ACK_FULL          		0x04		//ָ�����ݿ�����
#define ACK_NO_USER		  	0x05		//�޴��û�
#define ACK_USER_OPD			0x06		//�û��Ѵ���
#define ACK_FIN_OPD 			0x07 		//ָ���Ѵ���
#define ACK_TIMEOUT       		0x08		//�ɼ���ʱ
#define ACK_GO_OUT		  	0x0F		//��������

//�û���Ϣ����
#define ACK_ALL_USER       		0x00
#define ACK_GUEST_USER 	  		0x01
#define ACK_NORMAL_USER 		0x02
#define ACK_MASTER_USER    		0x03

#define USER_MAX_CNT	   		1001		//�������� MAX = 1000

//�����
#define CMD_HEAD		  	0xF5		//��־ͷ
#define CMD_TAIL		  	0xF5		//��־β
#define CMD_ADD_1  		  	0x01		//���ָ�Ƶ�1������
#define CMD_ADD_2 		  	0x02		//���ָ�Ƶ�2������
#define CMD_ADD_3	  	  	0x03		//���ָ�Ƶ�3������
#define CMD_DEL_ONE  	  		0x04		//ɾ��ָ���û�
#define CMD_DEL_ALL  	  		0x05		//ɾ�������û�
#define CMD_USER_CNT    		0x09		//ȡ�û�����
#define CMD_GETLIM			0x0A		//ȡ�û�Ȩ��
#define CMD_MATCH_ONE			0x0B		//�Ա�1:1
#define CMD_MATCH_N		  	0x0C		//�Ա�1:N
#define CMD_GET_EIGEN			0x23		//�ɼ�ͼ����ȡ����ֵ�ϴ�
#define CMD_GET_IMAGE			0x24		//�ɼ�ͼ���ϴ�
#define CMD_GET_VER			0x26		//ȡ DSP ģ��汾��
#define CMD_COM_LEV			0x28		//����/��ȡ�ȶԵȼ�
#define CMD_GET_USERNUM 		0x2B		//ȡ�ѵ�¼�����û��û��ż�Ȩ��
#define CMD_LP_MODE		  	0x2C		//ʹģ���������״̬
#define CMD_SET_READ_ADDMODE		0x2D		//����/��ȡָ�����ģʽ
#define CMD_TIMEOUT		 	0x2E		//����/��ȡָ�Ʋɼ��ȴ���ʱʱ��
#define CMD_UP_EIGEN    		0x31		//�ϴ� DSP ģ�����ݿ���ָ���û�����ֵ
#define CMD_DOWN_ADD    		0x41		//�´�����ֵ����ָ���û��Ŵ��� DSP ģ�����ݿ�
#define CMD_DOWN_MATCH_ONE    		0x42		//�´�ָ������ֵ�� DSP ģ�����ݿ�ָ�Ʊȶ� 1:1
#define CMD_DOWN_MATCH_N      		0x43		//�´�ָ������ֵ�� DSP ģ�����ݿ�ָ�Ʊȶ� 1:N
#define CMD_DOWN_MATCH_NOW    		0x44		//�´�����ֵ��ɼ�ָ�Ʊȶ�

#define CMD_FINGER_DETECTED 		0x14


__packed typedef struct
{
	unsigned char HEAD;				//1
	unsigned char CMD;				//1
	unsigned char Q1;			 		//1
	unsigned char Q2;					//1
	unsigned char Q3;					//1
	unsigned char NONE;				//1
	unsigned char CHK;				//1
	unsigned char TAIL;				//1
} Recieve_Pack;

typedef union
{
	unsigned char buf[8];
	Recieve_Pack pack;
} Serialize_Pack;

typedef struct
{
	char HEAD[4];			
	char CMD[4];			
	char Q1[4];
	char Q2[4];					
	char Q3[4];					
	char NONE[4];				
	char CHK[4];				
	char TAIL[4];				
} Show_Text;

extern Serialize_Pack receive_pack;
extern u8 receive_short_ok;
extern u8 receive_long_ok;
extern u8 u3receive_short_ok;
extern u8 u3receive_long_ok;
extern Show_Text show_text;
extern u8 receiveMore[200];
extern u8 i;					//forѭ���ñ���

void TxByte(USART_TypeDef* USARTx,u8 temp);
u8 setLpMode(USART_TypeDef* USARTx);					//ʹģ���������״̬
u8 setAndReadMode(USART_TypeDef* USARTx,u8 com,u8 mode);		//����/��ȡָ�����ģʽ
u8 addUser(USART_TypeDef* USARTx,u16 userNum,u8 lim);			//���ָ��
u8 deleteOneUser(USART_TypeDef* USARTx,u16 userNum);			//ɾ��ָ���û�
u8 deleteAllUser(USART_TypeDef* USARTx);		  		//ɾ�������û�
u8 getUserCount(USART_TypeDef* USARTx);					//ȡ�û�����
u8 matchOne(USART_TypeDef* USARTx,u8 userNum);				//�ȶ� 1:1
u8 matchN(USART_TypeDef* USARTx);					//�ȶ�ָ��1:N
void fastMatchN(USART_TypeDef* USARTx);					//���ٷ��ͱȶ�ָ��1:Nָ��
u8 getLim(USART_TypeDef* USARTx,u16 userNum);				//ȡ�û�Ȩ��
u8 getVer(USART_TypeDef* USARTx);					//ȡ DSP ģ��汾��
u8 setAndReadLevel(USART_TypeDef* USARTx,u8 cmd,u8 level);		//����/��ȡ�ȶԵȼ�-�ȶԵȼ�0-9 ȡֵԽ��ȶ�Խ�ϸ�Ĭ��ֵΪ5
u8 getEigen(USART_TypeDef* USARTx);					//�ɼ�ͼ����ȡ����ֵ�ϴ�
u8 downloadAndMatchONE(USART_TypeDef* USARTx,u16 userNum,u8 *eigen);	//�´�ָ������ֵ�� DSP ģ�����ݿ�ָ�Ʊȶ� 1�� 1
u8 downloadAndMatchN(USART_TypeDef* USARTx,u8 *eigen);			//�´�ָ������ֵ�� DSP ģ�����ݿ�ָ�Ʊȶ� 1:N
u8 uploadEigen(USART_TypeDef* USARTx,u16 userNum);			//�ϴ� DSP ģ�����ݿ���ָ���û�����ֵ
u8 downloadAddUser(USART_TypeDef* USARTx,u16 userNum,u8 lim,u8 *eigen);//�´�����ֵ����ָ���û��Ŵ��� DSP ģ�����ݿ�
u8 getAllUser(USART_TypeDef* USARTx);					//ȡ�ѵ�¼�����û��û��ż�Ȩ��
u8 setAndGetTimeOut(USART_TypeDef* USARTx,u8 cmd,u8 timeOut);		//����/��ȡָ�Ʋɼ��ȴ���ʱʱ��
void ResetFingerFlag(void);		//ָ��ģ���־λ��λ
void uart2_init(u32 bound);		//����2��ʼ��
void uart3_init(u32 bound);		//����3��ʼ��
#endif /*_FINGERPRINT_H*/

