#ifndef _ZIGBEE_H_
#define _ZIGBEE_H_

#include "stm32f10x.h"
#include "sys.h"

#define StorageADDRESS			0x03		//�ֿ��ַ
#define THISADDRESS					0x01		//������ַ
#define UPPERADDRESS		    0x00		//��λ����ַ
#define Z_HEAD 					0x5123A456	//��ͷ
#define Z_TAIL 					0xA7895ABC	//��β
#define ReSendTimes  			50			//����ط�����
__packed typedef struct
{
	u32 HEAD;			//0-3		��ͷ
	u8 ADDRESS;			//4			��ַ
	u8 CMD;				//5			����
	u16 BLOCK;			//6-7		���ݿ���
	u8 BYTE;			//8			�����ݿ���Ч�ֽ���
	u32 TOTAL;			//9-12		���ݰ����ֽ���
	u8 DATA[128];		//13-140	128�ֽ�����
	u32 CRC_DATA;		//141-144	CRC
	u32 TAIL;			//145-148	��β
}Data_Pack;

__packed typedef struct
{
	u32 HEAD;			//0-3
	u8 ADDRESS;			//4
	u8 CMD;				//5
	u16 BLOCK;			//6-7
	u8 BYTE;			//8
	u32 TOTAL;			//9-12
	u32 CRC_DATA;		//13-16
	u32 TAIL;			//17-20
}Data_PackHead;

typedef union
{
	u8 buf[149];		//���ջ�����
	Data_Pack pack;		//���ݰ�
	Data_PackHead packHead;//Ӧ��
}Z_Serialize_Pack;

extern u8 receive_ok;				//�������ݰ���ɱ�־
extern u16 receive_len;				//8�ֽڽ��ռ�����
extern u8 receiveNByte_ok;			//������ɱ�־

void uart1_init(u32 bound);		//��ʼ������
u8 sendNByte(u8 *data,u8 cmd,u8 addr,u32 byte,u16 outTime);	//����N�ֽ�
void cleanReceiveData(void);		//������ջ���ͱ�־λ
void sendHead(u8 addr,u8 cmd,u16 block,u8 byte,u32 total);  //����Ӧ��
//������data������������ addr����ַ cmd������ block�����ݿ��� byte��������Ч�ֽ���     total���������ݴ������ֽ���
void sendBlock(u8 *data,u8 addr,u8 cmd,u16 block,u8 byte,u32 total);

void Zigbee_Init(void);						//zigbee��ʼ��
void MODULE_SOFTWARE_RESTARRT(void);	//ģ������
void DATA_TRANS_MODE(u8 mode);			//���ݴ���ģʽ
u8 READ_CHANNEL(void);					//��ȡģ��Ƶ��
void SET_CHANNEL(u8 Channel);			//����ģ��Ƶ��
u8 READ_NODE(void);						//��ȡ�ڵ�����
void SET_ROUTER(void);					//����ģ��Ϊ��ģʽ
void SET_COORDINATOR(void);				//����ģ��Ϊ��ģʽ
void SET_BOUND(u8 bound);				//����ģ�鴮�ڲ�����
u16 READ_PIN_ID(void);					//��ȡģ��PIN ID
u16 SET_PIN_ID(u8 high,u8 low);			//�趨ģ��PIN IDΪ�ض�ֵXX XX
void SetBound(void);					//zigbee����������

#endif



