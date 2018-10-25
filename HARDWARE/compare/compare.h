#ifndef __COMPARE_H
#define __COMPARE_H	 
#include "sys.h"


#define Door_state PEout(2)		// PE2			0��		1��
#define Buzzer_state PEout(3)	// PB1		0��		1��	
#define MaxUserNum 	500			//����û���

typedef struct		//�ڴ����û���Ϣ����
{
	u16 UserNum;
	u32 ICID;
	u8  StateAndLimits;	//bit7 ״̬ bit3--bit0 Ȩ��
}LinkInfoTypeDef;


typedef struct
{
	char order[5];				//0--4  	˳���
	char CardNum[9];			//5--13 	ID����
	char StudentNum[12];		//14--25    ѧ��
	char Name[7];				//26--32    ����
	char Jurisdiction[2];		//33--34    Ȩ��
	char state[2];				//35--36	��Ա����״̬		1����ˢ�����ѽ��� �� 0����ˢ�������		
	char Fingerprint[388];		//37--425   ָ��
}Student_InformationTypeDef;	//425byte


typedef union 
{ 
	Student_InformationTypeDef Student_Information[1];
	char info_arrary[425];
}Union_info;

void GetLinkInfo(void);							//��ȡ�û���Ϣ����
u16 compare(void);								//�Աȿ��Ż��û���
void Check_Status(u16 LinkNum);					//�Ա�Ȩ�޺ͽ���״̬,���ƿ���
void Door_Buzzer_Init(void);					//�źͷ������˿ڳ�ʼ��
void SD_Finger_Compare(USART_TypeDef* USARTx);	//SD������Ϣ��ָ��ģ������Ϣͬ��
void offlcd(void);								//��ʱϨ��	
void show_photo(void);							//��ʾ��Ƭ
#endif







