#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "sdio_sdcard.h"
#include "ff.h"
#include "exfuns.h"
#include "rc522.h"
#include "text.h"
#include "compare.h"
#include "timer.h"
#include "fingerprint.h"
#include "rtc.h"
#include "ZigBee.h"
#include "Instruction.h"
#include "my_math.h"
#include "wdg.h"
#include "piclib.h"

const u8 zero[900]={0};
u8 data[1024];
extern char num[9];	//�����Ŀ���
extern char Card_flag;			//�ڶ�������⵽����־  0 ���ޣ�1����
extern char P_Card_flag;			//���������⵽����־  0 ���ޣ�1����
extern Union_info Client;					//������Ա��Ϣ�洢������

FIL fdsts;      	 //�洢�ļ���Ϣ
FATFS files;		 //�洢��������Ϣ
u32 len_datar=0;
u32 SD_capp;
u32 shuzhi=0;
u8 finger_ask=0;	//ָ��ģ�������־λ
u8 u3finger_ask=0;	//ָ��ģ�������־λ

extern FIL fdsts_recive;
extern UINT readnum;

void delete_all_Finger(void);//ɾ��ָ��ģ�������û�
extern LinkInfoTypeDef LinkInfo[MaxUserNum];	//�û���Ϣ����
int main(void)
{
	u8 t=0;
	u16 ret;
	delay_init();	    							 //��ʱ������ʼ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE); //ʱ��ѡ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�	
	LCD_Init();				//LCD��ʼ��
	
	exfuns_init();				//Ϊfatfs��ر��������ڴ�
 	f_mount(fs[0],"0:",1); 		//����SD��
	piclib_init();
	ai_load_picfile("0:/Desktop.jpg",0,0,320,240,1);//��ʾͼƬ
	Show_Str(150,60,144,16,"��ӭ�߽�����ʵ����",16,1);
	
	BACK_COLOR=LGRAY;  //����ɫ
	LCD_Fill(195,95,301,121,LGRAY);
	LCD_Fill(205,135,258,161,LGRAY);
	LCD_Fill(205,175,303,201,LGRAY);
	Show_Str(200,100,96,16,"���ڳ�ʼ��..",16,0);
//	Zigbee_Init();        	//Zigbee��ʼ��
	uart1_init(115200);     //Zigbee��ʼ��
	uart2_init(115200);			//ָ��ģ�鴮��2��ʼ��   ����
	uart3_init(115200);			//ָ��	ģ�鴮��3��ʼ��   ����
	
	InitRC522();			//��ʼ����Ƶ��ģ��      ����
	P_InitRC522();			//��ʼ����Ƶ��ģ��      ����
	Tim6_Init(); 			//TIM6��ʼ��
	Tim7_Init(); 			//TIM7��ʼ��
	Door_Buzzer_Init();	 	//�źͷ�������ʼ��
	RTC_Init();	  			//RTC��ʼ�� 
	ResetFingerFlag();			//ָ��ģ���־λ��λ
	setAndReadLevel(USART2,1,0);	//���á���ȡ�ȶԵȼ�
	if(receive_pack.pack.Q2!=5)
	{
		setAndReadLevel(USART2,0,5);	//����/��ȡ�ȶԵȼ�
	}
	
	setAndGetTimeOut(USART2,0,0);	//����/��ȡָ�Ʋɼ��ȴ���ʱʱ��
	setAndReadLevel(USART3,1,0);	//����/��ȡ�ȶԵȼ�
	if(receive_pack.pack.Q2!=5)
	{
		setAndReadLevel(USART3,0,5);	//����/��ȡ�ȶԵȼ�
	}
	setAndGetTimeOut(USART3,0,0);	//����/��ȡָ�Ʋɼ��ȴ���ʱʱ��
   	GetLinkInfo();
	SD_Finger_Compare(USART2);		//SD������Ϣ��ָ��ģ������Ϣͬ��
	SD_Finger_Compare(USART3);		//SD������Ϣ��ָ��ģ������Ϣͬ��
	photo_compare();
	LCD_Fill(195,95,301,121,LGRAY);
	LCD_Fill(205,135,258,161,LGRAY);
	LCD_Fill(205,175,303,201,LGRAY);
	
	
	Show_Str(200,100,96,16,"��ʼ����ɣ�",16,0);
	delay_ms(1000);
	LCD_LED=0;					//�رձ���
	ResetFingerFlag();	//ָ��ģ���־λ��λ
	fastMatchN(USART2);	//��������ָ��ģ��
	fastMatchN(USART3);	//��������ָ��ģ��

//	IWDG_Init(7,0xfff);    //���Ƶ��Ϊ256,����ֵΪ2^11,���ʱ��ԼΪ13s
  	while(1) 
	{	
		if(t!=calendar.sec)		//ʱ����ʾ
		{
			LCD_ShowString(20,10,200,16,16,"    -  -  ");	   
			LCD_ShowString(120,10,200,16,16,"  :  :  ");
			t=calendar.sec;
			LCD_ShowNum(20,10,calendar.w_year,4,16);									  
			LCD_ShowNum(60,10,calendar.w_month,2,16);									  
			LCD_ShowNum(84,10,calendar.w_date,2,16);	 
			switch(calendar.week)
			{
				case 0:
					LCD_ShowString(200,10,200,16,16,"Sunday   ");
					break;
				case 1:
					LCD_ShowString(200,10,200,16,16,"Monday   ");
					break;
				case 2:
					LCD_ShowString(200,10,200,16,16,"Tuesday  ");
					break;
				case 3:
					LCD_ShowString(200,10,200,16,16,"Wednesday");
					break;
                
				case 4:
					LCD_ShowString(200,10,200,16,16,"Thursday ");
					break;
				case 5:
					LCD_ShowString(200,10,200,16,16,"Friday   ");
					break;
				case 6:
					LCD_ShowString(200,10,200,16,16,"Saturday ");
					break;  
			}
			LCD_ShowNum(120,10,calendar.hour,2,16);									  
			LCD_ShowNum(144,10,calendar.min,2,16);									  
			LCD_ShowNum(168,10,calendar.sec,2,16);
		}
	
		ReadID();			//��SPI1��Ӧ�Ŀ�	����
		P_ReadID();			//��SPI3��Ӧ�Ŀ�	����
		finger_ask=(receive_short_ok==1)&&(receive_pack.pack.CMD==CMD_MATCH_N);		//��⵽��������ָ������
		u3finger_ask=(u3receive_short_ok==1)&&(receive_pack.pack.CMD==CMD_MATCH_N);	//��⵽��������ָ������
		if(P_Card_flag||Card_flag||finger_ask||u3finger_ask)						//��⵽��������
		{
			ret=compare();
			if(ret!=0xffff)			//���Ż��û���ƥ��ɹ�
			{
				if(StorageADDRESS == THISADDRESS)
				{
					if((LinkInfo[ret].StateAndLimits&0x0f)==0x0f)			//�θ�Ȩ��E�ɽ����ɿ����ⷿ�����ţ�
					{
						Check_Status(ret);		//�ԱȽ���״̬
					}
					else
					{					
						LCD_LED=1;			//��������
						LCD_Fill(195,95,301,121,LGRAY);
						LCD_Fill(205,135,258,161,LGRAY);
						LCD_Fill(205,175,303,201,LGRAY);
						Show_Str(200,100,96,16,"����Ȩ���룡",16,0);
						Show_Str(210,140,48,16,(u8*) Client.Student_Information[0].Name,16,0);
						Show_Str(210,180,88,16,(u8*) Client.Student_Information[0].StudentNum,16,0);
						show_photo();
						offlcd();			//��ʱϨ��
					}
				}
				
				else if(((LinkInfo[ret].StateAndLimits&0x0f)==THISADDRESS)||		//��Ӧʵ���ұ�ſɽ�
					((LinkInfo[ret].StateAndLimits&0x0f)==0x0f)||			//���Ȩ��F�ɽ����ɿ������ţ�
					((LinkInfo[ret].StateAndLimits&0x0f)==0x0e))			//�θ�Ȩ��E�ɽ����ɿ����ⷿ�����ţ�
				{
					Check_Status(ret);		//�ԱȽ���״̬
				}
				else
				{
					LCD_LED=1;			//��������
					LCD_Fill(195,95,301,121,LGRAY);
					LCD_Fill(205,135,258,161,LGRAY);
					LCD_Fill(205,175,303,201,LGRAY);
					Show_Str(200,100,96,16,"����Ȩ���룡",16,0);
					Show_Str(210,140,48,16,(u8*) Client.Student_Information[0].Name,16,0);
					Show_Str(210,180,88,16,(u8*) Client.Student_Information[0].StudentNum,16,0);
					show_photo();
					offlcd();			//��ʱϨ��
				}
			}	
			else
			{				
				
				LCD_LED=1;				//��������  
				LCD_Fill(195,95,301,121,LGRAY);
				LCD_Fill(205,135,258,161,LGRAY);
				LCD_Fill(205,175,303,201,LGRAY);
				Show_Str(200,100,96,16,"����δע�ᣡ",16,0);
				piclib_init();
				ai_load_picfile("0:/someone.jpg",10,60,120,150,1);//��ʾͼƬ				
				offlcd();				//��ʱϨ��
			}
			
			Card_flag=0;					//��ˢ����־����
			P_Card_flag=0;				//��ˢ����־����
			finger_ask=0;					//ָ��ģ�������־λ
			u3finger_ask=0;				//ָ��ģ�������־λ
			ResetFingerFlag();		//ָ��ģ���־λ��λ
			fastMatchN(USART2);		//���¿���1��Nָ�Ƽ��
			fastMatchN(USART3);		//���¿���1��Nָ�Ƽ��
		}
        if(receiveNByte_ok==1)     		//�����������
        {
            receiveNByte_ok=0;       	//��ս�����ɱ�־λ
						delay_ms(50);
            switch_CMD();               //ָ��ѡ�� 
        }
	}
 }

///*********************************************************************
//*��    �ܣ�ָ��ģ��ɾ�������û�
//*��ڲ�����
//*���ڲ�����
//*********************************************************************/
//void delete_all_Finger(void)
//{
//	u8 userCount = 0xff;		
//	while(userCount!=0)
//	{
//		deleteAllUser(USART2);
//		getUserCount(USART2);
//		userCount = (receive_pack.pack.Q1<<8)+receive_pack.pack.Q2;
//	}
//  userCount = 0xff;
//	while(userCount!=0)
//	{
//		deleteAllUser(USART3);
//		getUserCount(USART3);
//		userCount = (receive_pack.pack.Q1<<8)+receive_pack.pack.Q2;
//	}
//    delay_ms(1);
//}
// 
// 
