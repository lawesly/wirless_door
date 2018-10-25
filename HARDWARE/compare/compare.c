#include "compare.h"
#include "sdio_sdcard.h" 
#include "ff.h"
#include "lcd.h"
#include "text.h"
#include "string.h"			//�ַ�������
#include "delay.h"
#include "sys.h"
#include "fingerprint.h"
#include "Instruction.h"
#include "my_math.h"
#include "piclib.h"

UINT readnum;
extern FIL fdsts;  
extern char Card_flag;			//�ڶ�������⵽����־  0 ���ޣ�1����
extern char P_Card_flag;		//���������⵽����־  0 ���ޣ�1����
extern u8 finger_ask;				//ָ��ģ�������־λ
extern u8 u3finger_ask;			//ָ��ģ�������־λ
extern FILINFO fileinfop;		//��ʾͼƬ
extern FRESULT res;
extern DIR dirp;
Union_info Client;					//������Ա��Ϣ�洢������
LinkInfoTypeDef LinkInfo[MaxUserNum];	//�û���Ϣ����

/*********************************************************************
*��    �ܣ���ȡ�û���Ϣ����
*��ڲ�����
*���ڲ�����
*********************************************************************/
void GetLinkInfo(void)
{
	u16 i,k;
	char SdUserNumArray[5]={0};	//����SD���洢�û�����
	u16 SdUserNum;							//SD���洢�û�����
	char SdUserInfo[18]={0};		//����SD�û���Ϣ����
	if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)	//��SD���û�����
	{
		f_lseek(&fdsts,44);  
		f_read(&fdsts,SdUserNumArray,4,&readnum);
		f_close(&fdsts);
	}
	SdUserNum=my_atoi(SdUserNumArray);	//ת��SD�洢�û���
	for(i=0;i<MaxUserNum;i++)						//����û���Ϣ����
	{
		LinkInfo[i].UserNum=0;
		LinkInfo[i].ICID=0;
		LinkInfo[i].StateAndLimits=0;
	}
	if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)
	{
		for(i=0;i<SdUserNum;i++)		//��SD�û���Ϣ����
		{
			f_lseek(&fdsts,i*425+50); 
			f_read(&fdsts,SdUserInfo,14,&readnum);	//��SD�û��š�ѧ��
			f_lseek(&fdsts,i*425+50+33);
			f_read(&fdsts,&SdUserInfo[14],3,&readnum);	//��SDȨ�ޡ�״̬
			LinkInfo[i].UserNum=my_atoi(SdUserInfo);		//�洢�û�������û���
			for(k=0;k<8;k++)	//�洢�û�����IC����
			{
				if(SdUserInfo[k+5]>0x39)	//Ϊ��д��ĸ
				{
					LinkInfo[i].ICID=(LinkInfo[i].ICID<<4)+SdUserInfo[k+5]-0x37;
				}
				else	//Ϊ����
				{
					LinkInfo[i].ICID=(LinkInfo[i].ICID<<4)+SdUserInfo[k+5]-0x30;
				}
			}
			if(SdUserInfo[14]>0x39)	//�洢�û�������û�Ȩ��
			{
				LinkInfo[i].StateAndLimits=SdUserInfo[14]-0x37;
			}
			else
			{
				LinkInfo[i].StateAndLimits=SdUserInfo[14]-0x30;
			}
			if(SdUserInfo[16]==0x30)	//�洢�û�������û�״̬
			{
				LinkInfo[i].StateAndLimits&=0x7f;	//������ ���λΪ0
			}
			else
			{
				LinkInfo[i].StateAndLimits|=1<<7;	//������ ���λΪ1
			}
		}
		f_close(&fdsts);	  							
	}
}



/*********************************************************************
*��    �ܣ��Աȿ��Ż��û���
*��ڲ�����
*���ڲ�����
*********************************************************************/
extern unsigned char SN[4];
u16 compare(void)
{
	u16 i;
	u16 FingerUserNum=0;		//��⵽���û���
	u32 RFID_ICID=0;
	if((u3finger_ask==1)||(finger_ask==1))	//�����⵽ָ��
	{
		FingerUserNum=receive_pack.pack.Q1*100+receive_pack.pack.Q2;			//��⵽���û���
		if(FingerUserNum==0)
		{
			return 0xffff;		//�����û��������򷵻�0xffff
		}
		for(i=0;i<MaxUserNum;i++)	//���û������в�ѯ
		{
			if(LinkInfo[i].UserNum==FingerUserNum)	//�Ա��û���
			{
				if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)	//��ȡSD�����û���Ϣ
				{
					f_lseek(&fdsts,i*425+50); 
					f_read(&fdsts,Client.info_arrary,37,&readnum);	
					f_close(&fdsts);	  							
				}
				return i;			//�����û��������
			}
		}
	}
	else if(P_Card_flag||Card_flag)	//�����⵽IC��
	{
		for(i=0;i<4;i++)	//ת��IC����
		{
			RFID_ICID=(RFID_ICID<<8)+SN[i];
			SN[i]=0;
		}
		for(i=0;i<MaxUserNum;i++)	//���û������в�ѯ
		{
			if(LinkInfo[i].ICID==RFID_ICID)		//�Ա�IC����
			{
				if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)	//��SD���û���Ϣ
				{
					f_lseek(&fdsts,i*425+50);  
					f_read(&fdsts,Client.info_arrary,37,&readnum);	
					f_close(&fdsts);
				}
				return i;		//�����û��������
			}
		}
	}
	return 0xffff;		//�����û��������򷵻�0xffff
}


/*********************************************************************
*��    �ܣ��Ա�Ȩ�޺ͽ���״̬,���ƿ���
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Check_Status(u16 LinkNum)					
{
	u16 k=0,Menber_site,finger_save;
	u16 order_temp=0;						//�������
	char set_val[2]={"1"};					//״̬λ��������
	char reset_val[2]={"0"};				//״̬λ��������
	
	if((P_Card_flag==1)||(u3finger_ask==1))	//����ˢ��
	{
		if((LinkInfo[LinkNum].StateAndLimits>>7)==0)	//0��Ա���ڳ���
		{
			Door_state=1;					//�����ź�
			Buzzer_state=1;					//������
			delay_ms(100);				
			Door_state=0;
			Buzzer_state=0;
			LCD_LED=1;						//��������
			LCD_Fill(195,95,301,121,LGRAY);
			LCD_Fill(205,135,258,161,LGRAY);
			LCD_Fill(205,175,303,201,LGRAY);
			Show_Str(200,100,96,16,"лл���٣�  ",16,0);
			Show_Str(150,140,48,16,"������",16,1);							//��ʾ��Ա��Ϣ
			Show_Str(150,180,48,16,"ѧ�ţ�",16,1);
			Show_Str(210,140,48,16,(u8*) Client.Student_Information[0].Name,16,0);
			Show_Str(210,180,88,16,(u8*) Client.Student_Information[0].StudentNum,16,0);
			show_photo();
			LinkInfo[LinkNum].StateAndLimits|=0x80;			
			if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)
			{	 	
				f_lseek(&fdsts,LinkNum *425+35+50);                          	 	//�ƶ��ļ�ָ��
				f_write(&fdsts,set_val,1,&readnum);	
				f_close(&fdsts);	  							
			}
							
//			delay_ms(1500);
			offlcd();				//��ʱϨ��	
			Client.Student_Information[0].state[0]='1';
            Save_access();     		//������ʱ�ϴ�������Ϣ
		}
		else
		{

			LCD_LED=1;				//��������
			LCD_Fill(195,95,301,121,LGRAY);
			LCD_Fill(205,135,258,161,LGRAY);
			LCD_Fill(205,175,303,201,LGRAY);
			Show_Str(200,100,96,16,"������δˢ��",16,0);	
			Show_Str(210,140,48,16,(u8*) Client.Student_Information[0].Name,16,0);
			Show_Str(210,180,88,16,(u8*) Client.Student_Information[0].StudentNum,16,0);		
			show_photo();
//			delay_ms(1500);
			offlcd();				//��ʱϨ��			
		}
	}
	else if((Card_flag==1)||(finger_ask==1))	//����ˢ��
	{			
		if((LinkInfo[LinkNum].StateAndLimits>>7)==1)		//1��Ա�������
		{
			Door_state=1;			//�����ź�
			Buzzer_state=1;
			delay_ms(100);																																																	
			Door_state=0;
			Buzzer_state=0;
			LCD_LED=1;				//��������
			LCD_Fill(195,95,301,121,LGRAY);
			LCD_Fill(205,135,258,161,LGRAY);
			LCD_Fill(205,175,303,201,LGRAY);
			Show_Str(200,100,96,16,"��ӭ���٣�  ",16,0);
			Show_Str(150,140,48,16,"������",16,1);							//��ʾ��Ա��Ϣ
			Show_Str(150,180,48,16,"ѧ�ţ�",16,1);
			Show_Str(210,140,48,16,(u8*) Client.Student_Information[0].Name,16,0);
			Show_Str(210,180,88,16,(u8*) Client.Student_Information[0].StudentNum,16,0);
			show_photo();
			LinkInfo[LinkNum].StateAndLimits&=0x7f;			
			if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)
			{	 	
				f_lseek(&fdsts,LinkNum *425+35+50);                          	 	//�ƶ��ļ�ָ��
				f_write(&fdsts,reset_val,1,&readnum);	
				f_close(&fdsts);	  							
			}
//			delay_ms(1500);
			offlcd();			//��ʱϨ��	
			Client.Student_Information[0].state[0]='0';
            Save_access();      //������ʱ�ϴ�������Ϣ
		}	
		else
		{
			LCD_LED=1;				//��������
			LCD_Fill(195,95,301,121,LGRAY);
			LCD_Fill(205,135,258,161,LGRAY);
			LCD_Fill(205,175,303,201,LGRAY);
			Show_Str(200,100,96,16,"������δˢ��",16,0);
			Show_Str(210,140,48,16,(u8*) Client.Student_Information[0].Name,16,0);
			Show_Str(210,180,88,16,(u8*) Client.Student_Information[0].StudentNum,16,0);
			show_photo();
//			delay_ms(1500);
			offlcd();				//��ʱϨ��		
		}	
	}		
}

/*********************************************************************
*��    �ܣ���ʾ��Ƭ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void show_photo(void)
{
	char photo_addr[12]={"0:/"};
	char photo_name[9]={0};
	char jpg[5]={".jpg"};
	//0:/Desktop.jpg
	memcpy(photo_name,Client.Student_Information[0].order,4);		//�������û���
	memcpy(&photo_name[4],jpg,4);
	memcpy(&photo_addr[3],photo_name,8);	
	res = f_opendir(&dirp,(const TCHAR*)"0:/"); //��һ��Ŀ¼
  if (res == FR_OK)
	{
		while(1)
		{
			res = f_readdir(&dirp, &fileinfop);                   //��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || fileinfop.fname[0] == 0) 
			{
				piclib_init();
				ai_load_picfile("0:/someone.jpg",10,60,120,150,1);//��ʾͼƬ
				break;  //������/��ĩβ��,�˳�
			}
			fileinfop.fname[8]=0;
			if((strcmp(photo_name,fileinfop.fname))==0)
			{
				piclib_init();
				ai_load_picfile(photo_addr,10,60,120,150,1);//��ʾͼƬ	
				return;
			}
		}
	}
}

/*********************************************************************
*��    �ܣ��źͷ������˿ڳ�ʼ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Door_Buzzer_Init(void)			
{ 
 GPIO_InitTypeDef  GPIO_InitStructure; 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;	 //door	PE2,    
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz	
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 //�����趨������ʼ��
 GPIO_ResetBits(GPIOE,GPIO_Pin_2);						 //��λ
}

/*********************************************************************
*��    �ܣ�SD������Ϣ��ָ��ģ������Ϣͬ��
*��ڲ��������ں�
*���ڲ�����
*********************************************************************/
void SD_Finger_Compare(USART_TypeDef* USARTx)		
{
	u16 i,j,k;
	u8 m;
	u8 eigen[193];
	u8 fing_temp[2];
	u16 FingerAllNum=0;
	u16 FingerNum=0;
	u8 user_ok=0;
	char SdUserNumArray[5]={0};	//����SD���洢�û�����
	u16 SdUserNum;				//SD���洢�û�����
	if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)	//��SD���û�����
	{
		f_lseek(&fdsts,44);
		f_read(&fdsts,SdUserNumArray,4,&readnum);
		f_close(&fdsts);
	}
	SdUserNum=my_atoi(SdUserNumArray);	//ת��SD�洢�û���
	getAllUser(USARTx);
	FingerAllNum=(receiveMore[1]<<8)+receiveMore[2];
	for(k=0;k<FingerAllNum;k++)
	{
		FingerNum=(receiveMore[3*k+3]<<8)+receiveMore[3*k+4];
		for(i=0;i<SdUserNum;i++)
		{
			if(FingerNum==LinkInfo[i].UserNum)
			{
				user_ok=1;
				break;
			}
		}
		if(user_ok == 0)
		{
			deleteOneUser(USARTx,FingerNum);					//ɾ��ָ���û�
		}
		user_ok=0;
	}
	Show_Str(210,140,48,16,"�û���",16,0);	
	for(i=0;i<SdUserNum;i++)
	{
		if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)
		{
			f_lseek(&fdsts,i*425+50);                         					//�ƶ��ļ�ָ��
			f_read(&fdsts,Client.info_arrary,sizeof(Client.info_arrary),&readnum);	//��ȡ��Ա��Ϣ
			f_close(&fdsts);
		}

		LCD_ShowxNum(210,180,i+1,3,16,0);	//��ʾ ����
		for(j=0;j<193;j++)				//ת��ָ������ֵ
		{
			if((Client.Student_Information[0].Fingerprint[j*2] > 0x29)&&(Client.Student_Information[0].Fingerprint[j*2] < 0x3A))
			{				fing_temp[0]=Client.Student_Information[0].Fingerprint[j*2]-48;			}
			else if((Client.Student_Information[0].Fingerprint[j*2] > 0x40)&&(Client.Student_Information[0].Fingerprint[j*2] < 0x5B))
			{				fing_temp[0]=Client.Student_Information[0].Fingerprint[j*2]-55;			}
			else {fing_temp[0]=0;}
			if((Client.Student_Information[0].Fingerprint[j*2+1] > 0x29)&&(Client.Student_Information[0].Fingerprint[j*2+1] < 0x3A))
			{				fing_temp[1]=Client.Student_Information[0].Fingerprint[j*2+1]-48;			}
			else if((Client.Student_Information[0].Fingerprint[j*2+1] > 0x40)&&(Client.Student_Information[0].Fingerprint[j*2+1] < 0x5B))
			{				fing_temp[1]=Client.Student_Information[0].Fingerprint[j*2+1]-55;			}
			else {fing_temp[1]=0;}
			eigen[j] = (fing_temp[0]<<4)+fing_temp[1];				//ת������ֵ
		}
		if(downloadAndMatchONE(USARTx,LinkInfo[i].UserNum,eigen)==ACK_FAIL)	//�´�ָ������ֵ�� DSP ģ�����ݿ�ָ�Ʊȶ� 1�� 1		��ʧ�����û���Ϣ�д�
		{			
			ResetFingerFlag();										//ָ��ģ���־λ��λ
			deleteOneUser(USARTx,LinkInfo[i].UserNum);
			ResetFingerFlag();										//ָ��ģ���־λ��λ
			m=downloadAddUser(USARTx,LinkInfo[i].UserNum,1,eigen);			//�´�����ֵ����ָ���û��Ŵ��� DSP ģ�����ݿ�
			if(m==ACK_SUCCESS )										//�´�����ֵ����ָ���û��Ŵ��� DSP ģ�����ݿ�		�������û�
			{
				Show_Str(235,180,48,16,"�ųɹ�",16,0);
			}
			else
			{
				Show_Str(235,180,48,16,"��ʧ��",16,0);
			}			
		}	
		else 
		{
			Show_Str(235,180,48,16,"�ųɹ�",16,0);
		}
		ResetFingerFlag();				//ָ��ģ���־λ��λ	
	}
}

/*********************************************************************
*��    �ܣ���ʱϨ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void offlcd(void)
{
	TIM6->CR1&=~0x0001;		//ʧ��TIMx	
	TIM6->CNT = 0X0000;		//��ʱ��6���¼���
	TIM6->DIER|=0x0001;		//ʹ��ָ����TIM6�ж�,��������ж�
	TIM6->CR1|=0x0001;		//ʹ��TIMx
}
