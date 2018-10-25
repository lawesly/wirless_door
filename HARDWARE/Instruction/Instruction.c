#include "Instruction.h"
#include "sdio_sdcard.h"   
#include "ff.h"
#include "string.h"			//�ַ�������
#include "fingerprint.h"
#include "compare.h"
#include "ZigBee.h"
#include "rtc.h" 
//#include "stdlib.h"
#include "my_math.h"

extern LinkInfoTypeDef LinkInfo[MaxUserNum];	//�û���Ϣ����
extern u16 USER_SUM;
extern Union_info Client;		//������Ա��Ϣ�洢������
extern FIL fdsts;      		 	//�洢�ļ���Ϣ
u16 SD_NUM_OVER;						//SD����Ա��ϢOVER����λ
extern FIL fdsts_recive; 
extern UINT readnum;
extern const u8 zero[900];
extern u8 data[1024];
u8 eigen[193];                      //�洢ָ����Ϣ

UINT bws;        	 // File R/W count
UINT bws_p;
FIL fds_pho;
u16 USER_SUM=0;	
FRESULT res;
DIR dirp;
FILINFO fileinfop;

/*********************************************************************
*��    �ܣ��´�ָ������ֵ����û�
*��ڲ�����
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��9��18�� 22:12:15
*********************************************************************/
u8 adduser(void)
{
    static Union_userinfo transferinfo;        //��ʱ�洢��SD���ڵ��´���Ϣ
    u16 order_temp=0;                   //��Ա��Ϣ��SD���ı��е�λ��
    u16 userNum=0;                      //�û���
    u8 i,m,n;
    u8 TAB[1]={9};						//TAB asc2��
		u8 set_over[5]={"OVER"};			//����SD���û���Ϣ������־λOVER������
    char order_array[5]={0};			//���ڼ����û���
    u8  Huiche[2];                      //�س�����
    Huiche[0]=13;
    Huiche [1]=10;
	
    if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)               //������ʱ�洢��SD�����´���Ϣ��transferinfo
    {	 	
			f_lseek(&fdsts_recive,5);                          				 //�ƶ��ļ�ָ��
			f_read(&fdsts_recive,order_array,4,&readnum);					 //���û���
			f_lseek(&fdsts_recive,5);
			f_read(&fdsts_recive,transferinfo.userinfo_arrary,227,&readnum); //���û���Ϣ
			f_close(&fdsts_recive);	  							
    }
    ClearnSDCache();        											 //���SD��������Ϣ
		userNum=my_atoi(order_array);										 //�����û���
//    userNum=(transferinfo.userinfo.order[0]-0x30)*1000;		
//    userNum+=(transferinfo.userinfo.order[1]-0x30)*100;
//    userNum+=(transferinfo.userinfo.order[2]-0x30)*10;
//    userNum+=transferinfo.userinfo.order[3]-0x30;		
    if(userNum>USER_SUM)												 //��������Ա���û��Ŵ����ִ��û��ŵ����ֵ
    {   		
		if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)               	 //����SD���洢���û�����
    {	
			f_lseek(&fdsts,44);                         				 //�ƶ��ļ�ָ��
			f_write(&fdsts,transferinfo.userinfo.order,4,&readnum);		 //�޸�����û���
			f_lseek(&fdsts,USER_SUM*619+50);                         	 //�޸�OVER����λ
			f_write(&fdsts,zero,4,&readnum);	
			f_lseek(&fdsts,userNum*619+50);                         
			f_write(&fdsts,set_over,4,&readnum);
			f_close(&fdsts);	  							
		}
		for(i=USER_SUM;i<userNum;i++)
		{
			if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)               	//��ո��û�SD���ڵ���Ϣ
			{
				order_temp=i*619+50;
				f_lseek(&fdsts,order_temp);
				f_write(&fdsts,zero ,619,&readnum);			 	  	//����û���Ϣ  �����س�����
				f_close(&fdsts);
			}
		}
        USER_SUM=userNum;												 //��������û��� 		
    } 
	for(i=0;i<193;i++)													//��ָ������ֵת�����ַ����SD��
	{
		HexToChar(transferinfo.userinfo.Fingerprint[i]);
		data[3*i]=char_temp[0];
		data[3*i+1]=char_temp[1];
		data[3*i+2]=32;
	}
	data[578]=TAB[0];										//SD���ĸ�ʽ
	order_temp=(userNum-1)*619+50;    									//�������SD����Ա��Ϣ���λ��	    
	for(i=0;i<193;i++)
	{
			eigen[i]=transferinfo.userinfo.Fingerprint[i];
	}
	ResetFingerFlag();													//ָ��ģ���־λ��λ
	m=downloadAddUser(USART2,userNum,1,eigen);
	if(m==ACK_SUCCESS)													//�´�����ֵ����ָ���û��Ŵ��� DSP ģ�����ݿ�
	{
	ResetFingerFlag();												//ָ��ģ���־λ��λ
	n=downloadAddUser(USART3,userNum,1,eigen);
	if(n==ACK_SUCCESS)
	{
		//дһ���˵�������Ϣ
		if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)              		//����Ƿ��и��ļ�����д��  
		{																	//���и��ļ�д������  ��û�и��ļ�������һ����		
			f_lseek(&fdsts,order_temp);                          			//�ƶ��ļ�ָ��
			f_write(&fdsts,transferinfo.userinfo.order,4,&readnum);			//����SD����Ա��Ϣ
			f_write(&fdsts,TAB,1,&readnum);					//���TAB��
			f_write(&fdsts,transferinfo.userinfo.CardNum,8,&readnum);			
			f_write(&fdsts,TAB,1,&readnum);
			f_write(&fdsts,transferinfo.userinfo.StudentNum,11,&readnum);
			f_write(&fdsts,TAB,1,&readnum);
			f_write(&fdsts,transferinfo.userinfo.Name,6,&readnum);
			f_write(&fdsts,TAB,1,&readnum);
			f_write(&fdsts,transferinfo.userinfo.Jurisdiction,1,&readnum);
			f_write(&fdsts,TAB,1,&readnum);
			f_write(&fdsts,transferinfo.userinfo.state,1,&readnum);
			f_write(&fdsts,TAB,1,&readnum);
			f_write(&fdsts,data,193*3,&readnum);
			f_write(&fdsts,TAB,1,&readnum);
			f_write(&fdsts,Huiche ,2,&readnum);
			f_close(&fdsts);
			sendHead(THISADDRESS,CMD_ADD_USER,0,n,0xffffffff);			//���ͳɹ�Ӧ��
		}	    
		return ACK_SUCCESS; 
		}
		else	//ʧ��
		{
			sendHead(THISADDRESS,CMD_ADD_USER,0,n,0xffffffff);			//����ʧ��Ӧ��
			return n;
		}
    }
    else		//ʧ��
    {   
			sendHead(THISADDRESS,CMD_ADD_USER,0,m,0xffffffff);				//����ʧ��Ӧ��
      return m;
    }		
}

/*********************************************************************
*��    �ܣ�ɾ��ָ���û�
*��ڲ�����
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��9��18�� 22:12:15
*********************************************************************/
u8 deleteuse(void)
{
    u16 order_temp;														//ָ��SD���洢Ҫɾ����Ա��λ��
    u16 userNum;														//�û���
	u8 delete_order[5]={0};												//�洢�û���
	u8 delete_name[7]={0};												//�洢�û���
	
    if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)               //����Ƿ��и��ļ�����д��  ���и��ļ�д������  ��û�и��ļ�������һ����
    {	 	
        f_lseek(&fdsts_recive,5);                          				 //�ƶ��ļ�ָ��
        f_read(&fdsts_recive,delete_order,4,&readnum);					 //��ȡ�û���
		f_read(&fdsts_recive,delete_name,6,&readnum);					 //��ȡ�û���
        f_close(&fdsts_recive);	  							
    }
    userNum=my_atoi(delete_order);										 //�����û���	
//    userNum=(transferinfo.userinfo.order[0]-0x30)*1000;				 //�����û���
//    userNum+=(transferinfo.userinfo.order[1]-0x30)*100;
//    userNum+=(transferinfo.userinfo.order[2]-0x30)*10;
//    userNum+=transferinfo.userinfo.order[3]-0x30;	
    order_temp=(userNum-1)*619+50;    									 //�������SD����Ա��Ϣ���λ��    
																		 //619��ÿ������Ϣ�ֽ�����50��SD����ͷռ���ֽ���
    if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)               
	{	 	
        f_lseek(&fdsts,order_temp);                          			 //�ƶ��ļ�ָ��
        f_read(&fdsts,Client.info_arrary,sizeof(Client.info_arrary),&readnum);	//��ȡSD���洢����Ա��Ϣ
        f_close(&fdsts);	  							
    }		
    Client.Student_Information[0].Name[6]=0;  	
	if(strcmp(Client.Student_Information[0].Name,delete_name)==0)		//�Ƚ��û����Ƿ���ͬ
	{
		Client.Student_Information[0].order[4]=0;						//�ַ�����β
		if(strcmp(Client.Student_Information[0].order,delete_order)!=0) //����û��Ų���ȷ
        {
			sendHead(THISADDRESS,CMD_DELETE_USER,0,ACK_FAIL,0xffffffff);//����Ӧ��ʧ��
			return ACK_FAIL;
		}
	}
	else
	{
		sendHead(THISADDRESS,CMD_DELETE_USER,0,ACK_FAIL,0xffffffff);	//���û������Է���ʧ��Ӧ��
		return ACK_FAIL;
	}
     if((deleteOneUser(USART2,userNum)==ACK_SUCCESS)&&
		(deleteOneUser(USART3,userNum)==ACK_SUCCESS))
    { 																  	//������ָ��ģ���ɾ���ɹ�
		ClearnSDCache();            								 	//���SD��������Ϣ
		if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)               	//��ո��û�SD���ڵ���Ϣ
		{	
			f_lseek(&fdsts,order_temp);                          
			f_write(&fdsts,zero ,617,&readnum);			 	  	//����û���Ϣ  �����س�����
			f_close(&fdsts);	  							
		}
		sendHead(THISADDRESS,CMD_DELETE_USER,0,ACK_SUCCESS,0xffffffff); //������ӳɹ�Ӧ��
        return ACK_SUCCESS; 
    }
    else
    {  
		sendHead(THISADDRESS,CMD_DELETE_USER,0,ACK_FAIL,0xffffffff);    //��ָ��ģ��ɾ�����ɹ�������ʧ��Ӧ��
        return ACK_FAIL;
    }
}

/*********************************************************************
*��    �ܣ���ȡ��λ���û��б�
*��ڲ�����
*���ڲ�����
*ʱ�䣺2015��9��18�� 22:12:15
*********************************************************************/
void  Uploaduserlist(void)
{
  u8 i,k;		
	u16 j=0;
	u8 cmd_user_start[5]={0};	//�ӵڼ����û���ʼ
	u8 cmd_user_add[5]={0};		//�ϴ��û���
	u16 user_order_start=0;		//��ʼ�û���
	u16 user_order_add=0;		//��ֹ�û���
	u16 order_temp=0;			//SD��ԭ����û���
    u32 Menber_site=0;			//��ȡ��Ա��Ϣ���ֽ�λ��
	u16 num_t=0;				//��Ч�ϴ����û�����
	char SdUserNumArray[5]={0};	//����SD���洢�û�����
	u16 SdUserNum;				//SD���洢�û�����
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)               
	{	 	
        f_lseek(&fdsts_recive,5);                          				//�ƶ��ļ�ָ��
        f_read(&fdsts_recive,cmd_user_start,4,&readnum);   				//��ȡҪ�ϴ��ĵڼ����û����ϴ��û���
		f_read(&fdsts_recive,cmd_user_add,4,&readnum);     				//��ȡҪ�ϴ��ĵڼ����û����ϴ��û���
        f_close(&fdsts_recive);	  							
    }
	ClearnSDCache();        							   				//���SD��������Ϣ
	user_order_start=my_atoi(cmd_user_start);			   				//����ڼ����û�
	user_order_add=my_atoi(cmd_user_add);			       				//�����ϴ��û���
	
	
	if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)	//��SD���û�����
	{	 	
		f_lseek(&fdsts,44);  
		f_read(&fdsts,SdUserNumArray,4,&readnum);	
		f_close(&fdsts);	  							
	}
	SdUserNum=my_atoi(SdUserNumArray);	//ת��SD�洢�û���
	if((user_order_start+user_order_add)<=SdUserNum)
	{
		for(j=0;j<user_order_add;j++)
		{
			 if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)
			{	 	
				f_lseek(&fdsts,(user_order_start+j-1)*425+50);                  				 //�ƶ��ļ�ָ��
				f_read(&fdsts,Client.info_arrary,sizeof(Client.info_arrary),&readnum);	//��ȡ�û���Ϣ��������Clients
				f_close(&fdsts);	  							
			}
			for(i=0;i<4;i++)
			{   data[i+j*31+4]=Client.Student_Information[0].order[i];  }   		//�û���
			for(i=0;i<8;i++)
			{   data[i+j*31+4+4]=Client.Student_Information[0].CardNum[i];  }		//����
			for(i=0;i<11;i++)
			{   data[i+j*31+4+12]=Client.Student_Information[0].StudentNum[i];  }	//ѧ��
			for(i=0;i<6;i++)
			{   data[i+j*31+4+23]=Client.Student_Information[0].Name[i];  }    		//����
			data[j*31+4+29]=Client.Student_Information[0].Jurisdiction[0]; 			//Ȩ��
			data[j*31+4+30]=Client.Student_Information[0].state[0];                 //״̬            
		}
		HexToChar(j>>8);													//��ʵ���ϴ�������ס�����ַ������ڷ���
		data[0]=char_temp[0];
		data[1]=char_temp[1];
		HexToChar(j&0xff);
		data[2]=char_temp[0];
		data[3]=char_temp[1];	
		sendNByte(data,CMD_GET_USER_LIST,UPPERADDRESS,j*31+4,500);			//�����û���Ϣ	
	}
	if((user_order_start+user_order_add)>SdUserNum)
	{
		for(j=0;j<SdUserNum-user_order_start;j++)
		{
			 if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)
			{
				f_lseek(&fdsts,(user_order_start+j-1)*425+50);                  				 //�ƶ��ļ�ָ��
				f_read(&fdsts,Client.info_arrary,sizeof(Client.info_arrary),&readnum);	//��ȡ�û���Ϣ��������Clients
				f_close(&fdsts);	  							
			}
			for(i=0;i<4;i++)
			{   data[i+j*31+4]=Client.Student_Information[0].order[i];  }   		//�û���
			for(i=0;i<8;i++)
			{   data[i+j*31+4+4]=Client.Student_Information[0].CardNum[i];  }		//����
			for(i=0;i<11;i++)
			{   data[i+j*31+4+12]=Client.Student_Information[0].StudentNum[i];  }	//ѧ��
			for(i=0;i<6;i++)
			{   data[i+j*31+4+23]=Client.Student_Information[0].Name[i];  }    		//����
			data[j*31+4+29]=Client.Student_Information[0].Jurisdiction[0]; 			//Ȩ��
			data[j*31+4+30]=Client.Student_Information[0].state[0];                 //״̬            
		}
		HexToChar(j>>8);													//��ʵ���ϴ�������ס�����ַ������ڷ���
		data[0]=char_temp[0];
		data[1]=char_temp[1];
		HexToChar(j&0xff);
		data[2]=char_temp[0];
		data[3]=char_temp[1];	
		sendNByte(data,CMD_GET_USER_LIST,UPPERADDRESS,j*31+4,500);			//�����û���Ϣ	
	}
}


/*********************************************************************
*��    �ܣ���ȡ��λ���û�ȫ����Ϣ
*��ڲ�����
*���ڲ�����
*ʱ�䣺2015��9��18�� 22:12:15
*********************************************************************/
void  Uploadalluserlist(void)
{
    u8 i;		
	u16 j=0,k=0;
	u8 cmd_user_start[5]={0};	//�ӵڼ����û���ʼ
	u8 cmd_user_add[5]={0};		//�ϴ��û���
	u16 user_order_start=0;		//��ʼ�û���
	u16 user_order_add=0;		//��ֹ�û���
	u16 order_temp=0;
    u32 Menber_site=0;		//��ȡ��Ա��Ϣ���ֽ�λ��
	u16 num_t=0;
	
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)               
	{	 	
        f_lseek(&fdsts_recive,5);                          				//�ƶ��ļ�ָ��
        f_read(&fdsts_recive,cmd_user_start,4,&readnum);   				//��ȡҪ�ϴ��ĵڼ����û����ϴ��û���
		f_read(&fdsts_recive,cmd_user_add,4,&readnum);     				//��ȡҪ�ϴ��ĵڼ����û����ϴ��û���
        f_close(&fdsts_recive);	  							
    }
	ClearnSDCache();        							   				//���SD��������Ϣ
	user_order_start=my_atoi(cmd_user_start);			   				//����ڼ����û�
	user_order_add=my_atoi(cmd_user_add);			       				//�����ϴ��û���
    do
    {
        Menber_site=k*619+50;											 //�����ȡSD������λ��
		k++;
        if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)              
		{	 	
            f_lseek(&fdsts,Menber_site);                         		 //�ƶ��ļ�ָ��
            f_read(&fdsts,Client.info_arrary,sizeof(Client.info_arrary),&readnum);	//��ȡSD����Ա��Ϣ��������Clients
            f_close(&fdsts);	  							
        }	
		Client.Student_Information[0].order[4]=0;
		order_temp=my_atoi(Client.Student_Information[0].order);		 //SD��ԭ����û���
//		order_temp=(Client.Student_Information[0].order[0]-0x30)*1000;	 //�����û���
//		order_temp+=(Client.Student_Information[0].order[1]-0x30)*100;
//		order_temp+=(Client.Student_Information[0].order[2]-0x30)*10;
//		order_temp+=(Client.Student_Information[0].order[3]-0x30);
		if(k>user_order_start+100)										 //������Ա��Ϣ���ȫ��0�ǵ���ѭ��
		{
			return;
		}
		if(order_temp==0)												 //�û���Ϊ�գ����û��ѽ���ɾ��
		{
			continue;
		}	
		num_t++;														 //��Ч�ϴ����û�����+1
		if(num_t<user_order_start)										 //�����Ч�ϴ�������С�ڿ�ʼ�ϴ�������
		{
			continue;
		}		
        for(i=0;i<4;i++)
        {   data[i+j*224+4]=Client.Student_Information[0].order[i];  }          //�û���
        for(i=0;i<8;i++)
        {   data[i+j*224+4+4]=Client.Student_Information[0].CardNum[i];  }  	//����
        for(i=0;i<11;i++)
        {   data[i+j*224+4+12]=Client.Student_Information[0].StudentNum[i];  }  //ѧ��
        for(i=0;i<6;i++)
        {   data[i+j*224+4+23]=Client.Student_Information[0].Name[i];  }   	    //����
        data[j*224+4+29]=Client.Student_Information[0].Jurisdiction[0]; 		//Ȩ��
        data[j*224+4+30]=Client.Student_Information[0].state[0];                //״̬   
		for(i=0;i<193;i++)														//ָ��
		{
			data[j*224+4+31+i]=CharToHex(Client.Student_Information[0].Fingerprint[3*i],Client.Student_Information[0].Fingerprint[3*i+1]);
		}		
		j++;		
		if(j>4)																	//һ������ϴ�4����Ϣ
		{
			return;																//��ֹ�����ڴ���ѭ��
		}
    }while(j<user_order_add);													//ʵ���ϴ�������������Ҫ�ϴ�����ʱѭ��
	HexToChar(j>>8);															//��ʵ���ϴ�������ס�����ַ������ڷ���
	data[0]=char_temp[0];
	data[1]=char_temp[1];
	HexToChar(j&0xff);
	data[2]=char_temp[0];
	data[3]=char_temp[1];
//���������ֽ�����
//������data�������͵����� cmd�����addr����ַ    byte�� ���ֽ���     outTime(ms)����ʱʱ��
//����ֵ 0���ɹ� 1����ʱ 2:���ط����� 3:ͨ�Ŵ���
	sendNByte(data,CMD_GET_ALL_LIST,UPPERADDRESS,j*224+4,500);					//�����û���Ϣ
}

/*������������Ӧ��*/
void Sendonline(void)
{
    ClearnSDCache();         /*���SD��������Ϣ*/
    sendHead(THISADDRESS,CMD_ONLINE,0,0,0xffffffff);
}

/*********************************************************************
*��    �ܣ�����ʱ��
*��ڲ�����
*���ڲ�����
*ʱ�䣺2015��9��18�� 22:12:15
*********************************************************************/
void settime(void)
{
  u16 year;
  u8 mon,day,hour,min,sec;
	u8 n=0;
  Union_Time Gettime;
    if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
    {	 	
        f_lseek(&fdsts_recive,5);                          		//�ƶ��ļ�ָ��
        f_read(&fdsts_recive,Gettime.time_arrary,14,&readnum);	//��ȡ�趨ʱ��
        f_close(&fdsts_recive);	  							
    }
    ClearnSDCache();            								//���SD��������Ϣ
    year=(Gettime.TIME.year[0]-0x30)*1000;     					//����ʱ��
		year+=(Gettime.TIME.year[1]-0x30)*100;
		year+=(Gettime.TIME.year[2]-0x30)*10;
		year+=(Gettime.TIME.year[3]-0x30);
	
		mon=(Gettime.TIME.month[0]-0x30)*10;
		mon+=(Gettime.TIME.month[1]-0x30);
	
		day=(Gettime.TIME.date[0]-0x30)*10;
		day+=(Gettime.TIME.date[1]-0x30);
	
		hour=(Gettime.TIME.hour[0]-0x30)*10;
		hour+=(Gettime.TIME.hour[1]-0x30);
	
		min=(Gettime.TIME.minute[0]-0x30)*10;
		min+=(Gettime.TIME.minute[1]-0x30);
	
		sec=(Gettime.TIME.second[0]-0x30)*10;
		sec+=(Gettime.TIME.second[1]-0x30);
	
    n=RTC_Set(year,mon,day,hour,min,sec);						//����ʱ��	
		sendHead(THISADDRESS,CMD_SET_TIME,0,n,0xffffffff);			//��������ʱ�䷵��ֵ 0 �ɹ���1ʧ��
}

/*********************************************************************
*��    �ܣ��洢���˽�������Ϣ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Save_access(void)
{
	u32 IO_num=0;				//�洢�ĳ�����Ϣ����
	u32 IO_SD_SATE=0;			//ָ�򱾴δ洢��λ��
	char IO_num_array[9]={0};	//�洢�ĳ�����Ϣ����
	char time_array[16]={0};	//�洢ʱ��
	char time_temp[6]={0};		//ʱ�仺����
	if(f_open(&fdsts,"SaveIO.txt",FA_READ)==FR_OK)               
	{	 	
		f_lseek(&fdsts,0);                         		 //�ƶ��ļ�ָ��
		f_read(&fdsts,IO_num_array,8,&readnum);			 //��ȡ�Ѵ���Ϣ����
		f_close(&fdsts);
	}
	IO_num=my_atoi(IO_num_array);						 //���ַ���ת��������
	IO_SD_SATE=IO_num*36+8;								 //ָ�򱾴δ洢��λ��
	IO_num++;											 //�洢����+1
	sprintf(IO_num_array,"%08d",IO_num); 				 //������ת�����ַ���	
	sprintf(time_array, "%04d",calendar.w_year);			 //��ʽ����� ʱ��	
	sprintf(time_temp, "%02d",calendar.w_month);
	strcat(time_array,time_temp);						 //�ַ���ƴ��
	sprintf(time_temp, "%02d",calendar.w_date);
	strcat(time_array,time_temp);
	sprintf(time_temp, "%02d",calendar.hour);
	strcat(time_array,time_temp);
	sprintf(time_temp, "%02d",calendar.min);
	strcat(time_array,time_temp);
	sprintf(time_temp, "%02d",calendar.sec);
	strcat(time_array,time_temp);
//	if(Client.Student_Information[0].state[0]=='1')		 //�ı�ˢ���û��洢��״̬
//	{
//		Client.Student_Information[0].state[0]='0';		 //0�������ڣ�1��������
//	}
//	else
//	{
//		Client.Student_Information[0].state[0]='1';
//	}
	if(f_open(&fdsts,"SaveIO.txt",FA_WRITE)==FR_OK)
    {			
        f_lseek(&fdsts,0);                         		 //�ƶ��ļ�ָ��
		f_write(&fdsts,IO_num_array,8,&readnum);		 //�����Ѵ������Ϣ����
		f_lseek(&fdsts,IO_SD_SATE); 
        f_write(&fdsts,Client.Student_Information[0].order,4,&readnum);		 //�洢�û���	
		f_write(&fdsts,Client.Student_Information[0].StudentNum,11,&readnum);//�洢ѧ��
        f_write(&fdsts,Client.Student_Information[0].Name,6,&readnum);		 //�洢�û���
        f_write(&fdsts,Client.Student_Information[0].state,1,&readnum);	   	 //�洢����״̬  
		f_write(&fdsts,time_array,14,&readnum);               				 //�洢ʱ��
        f_close(&fdsts);	  							
    }	
}

/*********************************************************************
*��    �ܣ��ϴ����˽�������Ϣ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Upload_access(void)
{
	u8 ret=0;
	char IO_num_array[9]={0};		//�洢������Ϣ����
	u16 IO_num=0;								//������Ϣ����	
	u16 i;
	u8 clean_IO_num[9]={"00000000"};		//���ڽ�����������
	char seed_num[9]={0};
	for(i=0;i<1024;i++)
	{
		data[i]=0;
	}
	if(f_open(&fdsts,"SaveIO.txt",FA_READ)==FR_OK)
	{	 	
		f_lseek(&fdsts,0);                      //�ƶ��ļ�ָ��
		f_read(&fdsts,data,8,&readnum);					//��ȡ�洢������Ϣ����
		f_close(&fdsts);	  							
	}
	IO_num=my_atoi(data);								//�Ѵ������Ϣ����
	if(IO_num==0)
	{
		strncpy(data,clean_IO_num,8);
	}
	if(IO_num>=10)										//���������Ϣ��������10
	{
		sprintf(seed_num,"%08d",10);
		seed_num[8]=0;
		strncpy(&data[8],seed_num,8);
		if(f_open(&fdsts,"SaveIO.txt",FA_READ)==FR_OK)
		{	 	
			f_lseek(&fdsts,(IO_num-10)*36+8);         //�ƶ��ļ�ָ��
			f_read(&fdsts,&data[16],360,&readnum);		//��ȡ10��������Ϣ
			f_close(&fdsts);
		}
		ret=sendNByte(data,CMD_GTE_I_O,UPPERADDRESS,376,500	);//���ͽ�����Ϣ
		if(ret==0)
		{
			IO_num-=10;										//��Ϣ����-10
			sprintf(IO_num_array, "%08d",IO_num); 			//ת�����ַ���
			if(f_open(&fdsts,"SaveIO.txt",FA_WRITE)==FR_OK)
			{	 	
				f_lseek(&fdsts,0); 
				f_write(&fdsts,IO_num_array,8,&readnum);	//���´洢��Ϣ����			
				f_lseek(&fdsts,IO_num*36+8);                        		
				f_truncate(&fdsts);							//�ض��ļ�
				f_close(&fdsts);			
			}
		}
		
	}
	else
	{
		sprintf(seed_num,"%08d",IO_num);
		seed_num[8]=0;
		strncpy(&data[8],seed_num,8);
		if(IO_num > 0)
		{
			if(f_open(&fdsts,"SaveIO.txt",FA_READ)==FR_OK)
			{	 	
				f_lseek(&fdsts,8);                          
				f_read(&fdsts,&data[16],IO_num*36,&readnum);	//��ȡʣ�������Ϣ
				f_close(&fdsts);
			}
		
		}
		ret=sendNByte(data,CMD_GTE_I_O,UPPERADDRESS,IO_num*36+16,500	);			
		if(ret==0)
		{
			if(f_open(&fdsts,"SaveIO.txt",FA_WRITE)==FR_OK) 
			{	 	
				f_lseek(&fdsts,0); 
				f_write(&fdsts,clean_IO_num,8,&readnum);					
				f_truncate(&fdsts);							//�ض��ļ�
				f_close(&fdsts);			
			}	
		}
			
	}
}

/*********************************************************************
*��    �ܣ���ȡSD��������û���
*��ڲ�����
*���ڲ�����
*********************************************************************/
void GetMaxUserOrder(void)
{

	char USER_SUM_ARRAY[5]={0};
	if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)               	 //����SD���洢���û�����
	{	
		f_lseek(&fdsts,44);                         				 //�ƶ��ļ�ָ��
		f_read(&fdsts,USER_SUM_ARRAY,4,&readnum);		 //�޸�����û���
		f_close(&fdsts);	  							
	}		
	USER_SUM=my_atoi(USER_SUM_ARRAY);	
 }

/*********************************************************************
*��    �ܣ��ϴ�������
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Uploadusernum(void)
{
	u8 k=0;
	u16 order_temp=0;
	u16 USER_SUM_temp=0;
	u32 Menber_site=0;		//��ȡ��Ա��Ϣ���ֽ�λ��	
	char SD_info_OVER[5]={"OVER"};
	do
	{
		Menber_site=k*619+50;														//�����ȡSD������λ��
		k++;
		if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)
		{	 	
			f_lseek(&fdsts,Menber_site);                          					//�ƶ��ļ�ָ��
			f_read(&fdsts,Client.info_arrary,sizeof(Client.info_arrary),&readnum);	//��ȡ��Ա��Ϣ
			f_close(&fdsts);	  							
		}
		Client.Student_Information[0].order[4]=0;
		order_temp=my_atoi(Client.Student_Information[0].order);					//���ַ���ת��������
		if(order_temp==0)															//�û���Ϊ��
		{																			//��������ѭ��
			continue;
		}
		USER_SUM_temp++;															//�û��Ų�Ϊ����������+1
	}while(strcmp(Client.Student_Information[0].order,SD_info_OVER));				//ֱ������������־OVER
	sendHead(THISADDRESS,CMD_GET_USER_NUM,0,0xff,USER_SUM_temp);					//����������Ӧ��
}

/*********************************************************************
*��    �ܣ��洢��Ƭ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Save_photo(void)
{
	u8 data_num_arrary[5]={0};
	u8 photo_name[9]={0};
	u32 data_num=0;
	u16 k=0;

	
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
	{	 	
		f_lseek(&fdsts_recive,1);           //�ƶ��ļ�ָ��
        f_read(&fdsts_recive,data_num_arrary,4,&readnum);
		f_read(&fdsts_recive,photo_name,4,&readnum);
		f_lseek(&fdsts_recive,16);           //�ƶ��ļ�ָ��
		f_read(&fdsts_recive,&photo_name[4],4,&readnum);
        f_close(&fdsts_recive);	  							
	}	
	if(f_open(&fds_pho,photo_name,FA_CREATE_ALWAYS)==FR_OK)
	{
		f_close(&fds_pho);
	}
	
	data_num=(data_num_arrary[3]<<24)+(data_num_arrary[2]<<16)+(data_num_arrary[1]<<8)+data_num_arrary[0]-15;
//	data_num=30060;
	while(data_num>=1024)
	{
		if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
		{	 	
			f_lseek(&fdsts_recive,k*1024+20);           //�ƶ��ļ�ָ��
			f_read(&fdsts_recive,data,1024,&readnum);
			f_close(&fdsts_recive);	  							
		}
		if(f_open(&fds_pho,photo_name,FA_WRITE)==FR_OK)
		{	 	
			f_lseek(&fds_pho,k*1024);           //�ƶ��ļ�ָ��
			f_write(&fds_pho,data,1024,&bws_p);
			f_close(&fds_pho);	  							
		}	
		data_num-=1024;
		k++;
	}
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
	{	 	
		f_lseek(&fdsts_recive,k*1024+20);           //�ƶ��ļ�ָ��
		f_read(&fdsts_recive,data,data_num,&readnum);
		f_close(&fdsts_recive);	  							
	}
	if(f_open(&fds_pho,photo_name,FA_WRITE)==FR_OK)
	{	 	
		f_lseek(&fds_pho,k*1024);           //�ƶ��ļ�ָ��
		f_write(&fds_pho,data,data_num,&bws_p);
		f_close(&fds_pho);	  							
	}
	sendHead(THISADDRESS,CMD_SAVE_PHOTO,0,0xff,0);					//���ͽ�����Ƭ���Ӧ��
}

/*********************************************************************
*��    �ܣ�ɾ����Ƭ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void deletephoto(void)
{
	u8 res=0;
	u8 photo_name[9]={0};
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
	{	 	
		f_lseek(&fdsts_recive,5);            //�ƶ��ļ�ָ��
		f_read(&fdsts_recive,photo_name,4,&readnum);
		f_lseek(&fdsts_recive,16);           //�ƶ��ļ�ָ��
		f_read(&fdsts_recive,&photo_name[4],4,&readnum);
        f_close(&fdsts_recive);	  							
	}	
	res=f_unlink(photo_name);					 //ɾ���ļ�
	sendHead(THISADDRESS,CMD_DELETE_PHOTO,0,res,0);					//���ͽ�����Ƭ���Ӧ��
}
/*********************************************************************
*��    �ܣ����SD��������Ϣ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void ClearnSDCache(void)
{
    if(f_open(&fdsts_recive,"Receive.txt",FA_WRITE)==FR_OK)
    {
        f_lseek(&fdsts_recive,0);           //�ƶ��ļ�ָ��
        f_truncate(&fdsts_recive);			//�ض��ļ�
        f_close(&fdsts_recive);
    }
}


/*********************************************************************
*��    �ܣ��½���Ա��Ϣ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void new_member(void)
{
	u8 data_num_arrary[5]={0};
	u32 data_num=0;
	u16 k=0;

	
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
	{	 	
		f_lseek(&fdsts_recive,1);           //�ƶ��ļ�ָ��
        f_read(&fdsts_recive,data_num_arrary,4,&readnum);
        f_close(&fdsts_recive);	  							
	}
	if(f_open(&fdsts,"Member.txt",FA_CREATE_ALWAYS)==FR_OK)
	{
		f_close(&fdsts);
	}
	
	data_num=(data_num_arrary[3]<<24)+(data_num_arrary[2]<<16)+(data_num_arrary[1]<<8)+data_num_arrary[0];
	while(data_num>=1024)
	{
		if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
		{	 	
			f_lseek(&fdsts_recive,k*1024+5);           //�ƶ��ļ�ָ��
			f_read(&fdsts_recive,data,1024,&readnum);
			f_close(&fdsts_recive);
		}
		if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)
		{	 	
			f_lseek(&fdsts,k*1024);                          	 	//�ƶ��ļ�ָ��
			f_write(&fdsts,data,1024,&readnum);	
			f_close(&fdsts);	  							
		}
			
		data_num-=1024;
		k++;
	}
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
	{	 	
		f_lseek(&fdsts_recive,k*1024+5);           //�ƶ��ļ�ָ��
		f_read(&fdsts_recive,data,data_num,&readnum);
		f_close(&fdsts_recive);
	}
	if(f_open(&fdsts,"Member.txt",FA_WRITE)==FR_OK)
	{	 	
		f_lseek(&fdsts,k*1024);                          	 	//�ƶ��ļ�ָ��
		f_write(&fdsts,data,data_num,&readnum);	
		f_close(&fdsts);	  							
	}
//	GetMaxUserOrder();
	GetLinkInfo();
	SD_Finger_Compare(USART2);		//SD������Ϣ��ָ��ģ������Ϣͬ��
	SD_Finger_Compare(USART3);		//SD������Ϣ��ָ��ģ������Ϣͬ��
	ResetFingerFlag();			//ָ��ģ���־λ��λ
	fastMatchN(USART2);	//��������ָ��ģ��
	fastMatchN(USART3);	//��������ָ��ģ��	

	ClearnSDCache();
	sendHead(THISADDRESS,CMD_MEMBER,0,0X0B,0);					//���ͽ�����Ƭ���Ӧ��
}

/*********************************************************************
*��    �ܣ���ѯͼƬ�Ƿ����
*��ڲ�����
*���ڲ�����
*********************************************************************/
void check_photo(void)
{
	char name_buff[9]={0};
	char photo_size_buf[9]={0};
	u32 photo_size=0;
	
	if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
	{	 	
		f_lseek(&fdsts_recive,5);           //�ƶ��ļ�ָ��
        f_read(&fdsts_recive,name_buff,8,&readnum);
		f_read(&fdsts_recive,photo_size_buf,8,&readnum);
        f_close(&fdsts_recive);	  							
	}
	photo_size = my_atoi(photo_size_buf);
	
	res = f_opendir(&dirp,(const TCHAR*)"0:/"); //��һ��Ŀ¼
    if (res == FR_OK) 
	{	
		while(1)
		{
	        res = f_readdir(&dirp, &fileinfop);                   //��ȡĿ¼�µ�һ���ļ�
	        if (res != FR_OK || fileinfop.fname[0] == 0) 
			{
				sendHead(THISADDRESS,CMD_CHECK_PHOTO,0,0,0);					//������Ƭ������Ӧ��
				break;  //������/��ĩβ��,�˳�
			}
	
			fileinfop.fname[8]=0;
			if((strcmp(name_buff,fileinfop.fname))==0)
			{
				if(fileinfop.fsize == photo_size)
				{
					sendHead(THISADDRESS,CMD_CHECK_PHOTO,0,1,0);					//������Ƭ����Ӧ��
					break;
				}				
			}			
		}
	}
}




/*********************************************************************
*��    �ܣ�ͬ��ͼƬ
*��ڲ�����
*���ڲ�����
*********************************************************************/

void photo_compare(void)
{
	
	char jpg[5]={".jpg"};
	u8 k;
	u16 PhotoNum;
	u16 order_temp;
	u32 Menber_site;
	char name_buff[9]={0};
	u8 check_ok=0;
	char SdUserNumArray[5]={0};	//����SD���洢�û�����
	u16 SdUserNum;				//SD���洢�û�����
	if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)	//��SD���û�����
	{	 	
		f_lseek(&fdsts,44);  
		f_read(&fdsts,SdUserNumArray,4,&readnum);	
		f_close(&fdsts);	  							
	}
	SdUserNum=my_atoi(SdUserNumArray);	//ת��SD�洢�û���
	res = f_opendir(&dirp,(const TCHAR*)"0:/"); //��һ��Ŀ¼
    if (res == FR_OK)
	{	
		while(1)
		{
	        res = f_readdir(&dirp, &fileinfop);                   //��ȡĿ¼�µ�һ���ļ�
	        if (res != FR_OK || fileinfop.fname[0] == 0) 
			{
				break;  //������/��ĩβ��,�˳�
			}
	
			fileinfop.fname[8]=0;
			if((strcmp(jpg,&fileinfop.fname[4]))!=0)
			{
				continue;
			}
			memcpy(name_buff,fileinfop.fname,4);
			PhotoNum=my_atoi(name_buff);
			check_ok=0;	
			
			for(k=0;k<SdUserNum;k++)
			{
				LinkInfo[k].UserNum==PhotoNum;
				check_ok=1;		
				break;
			}
			if(check_ok==0)
			{
				memcpy(name_buff,fileinfop.fname,8);
				f_unlink(name_buff);					 //ɾ���ļ�				
			}	
			
//			k=0;
//			do
//			{
//				Menber_site=k*425+50;
//				k++;		
//				if(f_open(&fdsts,"Member.txt",FA_READ)==FR_OK)
//				{	 	
//					f_lseek(&fdsts,Menber_site);                         					//�ƶ��ļ�ָ��
//					f_read(&fdsts,Client.info_arrary,sizeof(Client.info_arrary),&readnum);	//��ȡ��Ա��Ϣ
//					f_close(&fdsts);	  							
//				}	
//				order_temp=(Client.Student_Information[0].order[0]-0x30)*1000;				//�����û���
//				order_temp+=(Client.Student_Information[0].order[1]-0x30)*100;
//				order_temp+=(Client.Student_Information[0].order[2]-0x30)*10;
//				order_temp+=(Client.Student_Information[0].order[3]-0x30);			
//				Client.Student_Information[0].order[4]=0;
//				if(strcmp(Client.Student_Information[0].order,name_buff) == 0)
//				{
//					check_ok=1;		
//					break;
//				}			
//				
//			}while(order_temp!=SD_NUM_OVER);	//û������������־��ѭ��
			if(check_ok==0)
			{
				memcpy(name_buff,fileinfop.fname,8);
				f_unlink(name_buff);					 //ɾ���ļ�				
			}			
			
		}
	}


}







/*********************************************************************
*��    �ܣ�ָ��ѡ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void switch_CMD(void)
{
    u8 CMD=0;
    if(f_open(&fdsts_recive,"Receive.txt",FA_READ)==FR_OK)
    {	 	
        f_lseek(&fdsts_recive,0);                         //�ƶ��ļ�ָ��
        f_read(&fdsts_recive,&CMD,1,&readnum);						//��ȡָ��
        f_close(&fdsts_recive);	  							
    }  
//	CMD=CMD_DELETE_PHOTO;
    switch(CMD)														//ָ��ѡ��
    {
        case CMD_ADD_USER: 			{   adduser();  		 			break;	} //����û�
        case CMD_DELETE_USER: 	{   deleteuse(); 		 			break; 	}	//ɾ��ָ���û�
        case CMD_ONLINE: 				{   Sendonline(); 		 		break; 	}	//������������Ӧ��
        case CMD_GET_USER_LIST: {   Uploaduserlist(); 	 	break; 	}	//��ȡ��λ���û��б�
        case CMD_GET_ALL_LIST:  {   Uploadalluserlist();	break;	}	//��ȡ��λ���û�ȫ����Ϣ
        case CMD_GET_USER_NUM:	{	 	Uploadusernum();	 		break;	}	//��ȡ�û�����
        case CMD_SET_TIME :			{   settime();			 			break; 	}	//����ʱ��
        case CMD_GTE_I_O : 			{   Upload_access();	 		break; 	}	//�ϴ����˽�������Ϣ
				case CMD_SAVE_PHOTO:		{	 	Save_photo();		 			break;	}	//�洢��Ƭ
				case CMD_DELETE_PHOTO:	{	 	deletephoto();		 		break;	}	//ɾ����Ƭ
				case CMD_MEMBER:				{	 	new_member();		 			break;	}	//�½���Ա��Ϣ
				case CMD_CHECK_PHOTO:		{	 	check_photo();		 		break;	}	//��ѰͼƬ�Ƿ����

	
        default:{break;}
    }
}
















