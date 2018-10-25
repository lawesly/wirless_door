#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include "sys.h"

//��λ��������λ��
#define CMD_ADD_USER          0X01      //�����Ա
#define CMD_DELETE_USER       0X02      //ɾ����Ա
#define CMD_ONLINE            0X03      //�������
#define CMD_GET_USER_LIST     0X04      //��ȡ��λ���û��б�
#define CMD_GET_ALL_LIST      0X05      //��ȡ��λ��ȫ����Ա��Ϣ
#define CMD_GET_USER_NUM			0X06			//��ȡ��λ��������
#define CMD_SET_TIME				  0X07			//������λ��ʱ��
#define CMD_GTE_I_O					  0X08			//��ȡ��Ա������Ϣ
#define CMD_SAVE_PHOTO				0X09			//�洢��Ƭ
#define CMD_DELETE_PHOTO			0X0A			//ɾ����Ƭ
#define CMD_MEMBER			 		  0X0B			//�´���Ա��Ϣ��
#define CMD_CHECK_PHOTO				0X0C			//�鿴ͼƬ�Ƿ����


typedef struct
{
	char order[4];					//˳���
	char CardNum[8];				//ID����
	char StudentNum[11];		//ѧ��
	char Name[6];						//����
	char Jurisdiction[1];		//Ȩ��
	char state[1];					//��Ա����״̬		1����ˢ�����ѽ��� �� 0����ˢ�������		
	char Fingerprint[193];	//ָ��
    char Totalnum[4];
}TransferTypeDef;					//227byte


typedef union 
{ 
	TransferTypeDef userinfo;
	char userinfo_arrary[227];
}Union_userinfo;

typedef struct
{
    u8 year[4];
    u8 month[2];
    u8 date[2];
    u8 hour[2];
    u8 minute[2];
    u8 second[2];
}TimeTypeDef;
    
typedef union
{
    TimeTypeDef TIME;
    u8 time_arrary[14];
}Union_Time;

u8 adduser(void);      		 		//�´�ָ������ֵ����û�
u8 deleteuse(void);    		 		//ɾ��ָ���û�
void Sendonline(void);       	//������������Ӧ��
void Uploaduserlist(void);   	//��ȡ��λ���û��б�  ����ָ��
void Uploadalluserlist(void);	//��ȡ��λ���û�ȫ����Ϣ
void settime(void);    		 		//����ʱ��
void Save_access(void);		 		//�洢���˽�������Ϣ
void Upload_access(void);    	//������ʱ�ϴ�������Ϣ
void ClearnSDCache(void);	 		//���SD��������Ϣ
void switch_CMD(void);       	//ָ��ѡ��
void GetMaxUserOrder(void);  	//��ȡSD�����û�����
void Uploadusernum(void);	 		//�ϴ�������
void Save_photo(void);		 		//�洢��Ƭ
void new_member(void);		 		//�½���Ա��Ϣ��
void photo_compare(void);	 		//ͬ��ͼƬ
void check_photo(void);		 		//��ѯͼƬ�Ƿ����

#endif

