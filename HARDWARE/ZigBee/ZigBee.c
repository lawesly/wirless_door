#include "stm32f10x.h"
#include "ZigBee.h"
#include "delay.h"
#include "lcd.h"
#include "sdio_sdcard.h"   
#include "ff.h"
#include "my_math.h"
extern FIL fdsts;      	 //�洢�ļ���Ϣ
extern UINT readnum;
FIL fdsts_recive; 
u8 receive_ok = 0;				//�������ݰ���ɱ�־
u8 receiveNByte_ok = 0;			//������ɱ�־
u16 Z_receive_len = 0;			//8�ֽڽ��ռ�����
Z_Serialize_Pack serialize_pack;
u8 receiveSwitch = 0;			//���տ���
u16 numR;
u32 receiveCounter = 0;			//���յ������ֽ���

u8 rec_init[8];
u8 Zigbe_INIT=0;				//zigbee��ʼ��ǰ
//STM32��CRC
u32 CRC32(u8 *pBuf,u16 head,u16 tail)
{
        CRC_ResetDR();        	//��λCRC        
        for(; head <= tail; head++)
        {
                CRC->DR = (u32)pBuf[head];
        }
        return (CRC->DR);
}

//��ʼ��IO ����1 
//bound:������
void uart1_init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
 	USART_DeInit(USART1);  //��λ����1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //Usart1 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 

}

//����1����һ�ֽ�����
//����������������
//����ֵ ��
void sendByte(u8 data)
{      
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//�ر��ж�
    USART1->DR = data;      
		while((USART1->SR&0X40)==0);//ѭ������,ֱ���������  
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
}

//���������
//��������
//����ֵ ��
void cleanReceiveData()
{
	
	u8 i;
	Z_receive_len = 0;
	receive_ok = 0;
	for(i = 0;i < 149;i++)
	{
		serialize_pack.buf[i] = 0;
	}
}

//����Ӧ��
//������addr����ַ 		cmd������		 block�����ݿ��� 		byte��������Ч�ֽ��� 	total���������ݴ������ֽ���
//����ֵ ��	
void sendHead(u8 addr,u8 cmd,u16 block,u8 byte,u32 total)
{
	u32 crcTemp = 0;
	CRC_ResetDR();        					//��λCRC
	
	sendByte((u8)((Z_HEAD&0xff000000)>>24));
	sendByte((u8)((Z_HEAD&0x00ff0000)>>16));
	sendByte((u8)((Z_HEAD&0x0000ff00)>>8));
	sendByte((u8)((Z_HEAD&0x000000ff)>>0));	//��������ͷ
	
	sendByte(addr);							//���͵�ַ
	
	sendByte(cmd);							//��������
	
	sendByte((u8)((block&0xff00)>>8));
	sendByte((u8)((block&0x00ff)>>0));		//���͵�ǰ���ݿ���

	sendByte(byte);							//���͵�ǰ����Ч�ֽ���
	
	sendByte((u8)((total&0xff000000)>>24));
	sendByte((u8)((total&0x00ff0000)>>16));
	sendByte((u8)((total&0x0000ff00)>>8));
	sendByte((u8)((total&0x000000ff)>>0));		//�������ֽ���

	crcTemp = CRC_CalcCRC((u32)addr);					//����CRC
	crcTemp = CRC_CalcCRC((u32)cmd);					//����CRC
	crcTemp = CRC_CalcCRC((u32)((block&0x00ff)>>0));	//����CRC
	crcTemp = CRC_CalcCRC((u32)((block&0xff00)>>8));	//����CRC
	crcTemp = CRC_CalcCRC((u32)byte);					//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0x000000ff)>>0);	//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0x0000ff00)>>8);	//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0x00ff0000)>>16);	//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0xff000000)>>24);	//����CRC

	sendByte((u8)((crcTemp&0xff000000)>>24));
	sendByte((u8)((crcTemp&0x00ff0000)>>16));
	sendByte((u8)((crcTemp&0x0000ff00)>>8));
	sendByte((u8)((crcTemp&0x000000ff)>>0));		//����CRC
	
	sendByte((u8)((Z_TAIL&0xff000000)>>24));
	sendByte((u8)((Z_TAIL&0x00ff0000)>>16));
	sendByte((u8)((Z_TAIL&0x0000ff00)>>8));
	sendByte((u8)((Z_TAIL&0x000000ff)>>0));			//��������β
}

//����һ�����ݿ�
//������data������������ addr����ַ cmd������ block�����ݿ��� byte��������Ч�ֽ���     total���������ݴ������ֽ���
//����ֵ ��
void sendBlock(u8 *data,u8 addr,u8 cmd,u16 block,u8 byte,u32 total)
{
	u8 i = 0;
	u32 crcTemp = 0;
	CRC_ResetDR();        					//��λCRC
	
	sendByte((u8)((Z_HEAD&0xff000000)>>24));
	sendByte((u8)((Z_HEAD&0x00ff0000)>>16));
	sendByte((u8)((Z_HEAD&0x0000ff00)>>8));
	sendByte((u8)((Z_HEAD&0x000000ff)>>0));	//��������ͷ
	
	sendByte(addr);							//���͵�ַ
	
	sendByte(cmd);							//��������
	
	sendByte((u8)((block&0xff00)>>8));
	sendByte((u8)((block&0x00ff)>>0));		//���͵�ǰ���ݿ���

	sendByte(byte);							//���͵�ǰ����Ч�ֽ���
	
	sendByte((u8)((total&0xff000000)>>24));
	sendByte((u8)((total&0x00ff0000)>>16));
	sendByte((u8)((total&0x0000ff00)>>8));
	sendByte((u8)((total&0x000000ff)>>0));	//�������ֽ���

	crcTemp = CRC_CalcCRC((u32)addr);					//����CRC
	crcTemp = CRC_CalcCRC((u32)cmd);					//����CRC
	crcTemp = CRC_CalcCRC((u32)((block&0x00ff)>>0));	//����CRC
	crcTemp = CRC_CalcCRC((u32)((block&0xff00)>>8));	//����CRC
	crcTemp = CRC_CalcCRC((u32)byte);					//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0x000000ff)>>0);	//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0x0000ff00)>>8);	//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0x00ff0000)>>16);	//����CRC
	crcTemp = CRC_CalcCRC((u32)(total&0xff000000)>>24);	//����CRC
	for(i = 0;i < 128;i++)
	{
		sendByte(data[i]);						//��������
		crcTemp = CRC_CalcCRC((u32)data[i]);	//����CRC
	}
	
	sendByte((u8)((crcTemp&0xff000000)>>24));
	sendByte((u8)((crcTemp&0x00ff0000)>>16));
	sendByte((u8)((crcTemp&0x0000ff00)>>8));
	sendByte((u8)((crcTemp&0x000000ff)>>0));	//����CRC
	
	sendByte((u8)((Z_TAIL&0xff000000)>>24));
	sendByte((u8)((Z_TAIL&0x00ff0000)>>16));
	sendByte((u8)((Z_TAIL&0x0000ff00)>>8));
	sendByte((u8)((Z_TAIL&0x000000ff)>>0));		//��������β
}

//�������ݿ�
//��������
//����ֵ ��
void zigBeeReceiveData()
{
	static u32 oldBlock = 0;
	if(serialize_pack.pack.ADDRESS == THISADDRESS)	//ͨ����ַ�ж��Ƿ��������
	{
		if((serialize_pack.pack.BLOCK == oldBlock + 1)||(serialize_pack.pack.BLOCK == 0))
		{
			oldBlock = serialize_pack.pack.BLOCK;
			receiveCounter += serialize_pack.pack.BYTE; //�����յ����ֽ����ӵ��ֽڼ�������
			//����serialize_pack.pack.CMD;
			//���ֽ���serialize_pack.pack.TOTAL;
			//serialize_pack.pack.DATA					//�����յ����ݴ����ڴ濨
			
			if(serialize_pack.pack.BLOCK==0)
			{
				receiveCounter = serialize_pack.pack.BYTE;
				if(f_open(&fdsts_recive,"Receive.txt",FA_WRITE|FA_CREATE_ALWAYS)==FR_OK)
				{			
					f_lseek(&fdsts_recive,0);                         				 //�ƶ��ļ�ָ��
					f_write(&fdsts_recive,&serialize_pack.pack.CMD,1,&readnum);		 //����
					f_write(&fdsts_recive,&serialize_pack.pack.BYTE+1,4,&readnum);	 //���ݰ����ֽ���
					f_close(&fdsts_recive);	 
				}
				else
				{
					return;
				}
			}            
			if(f_open(&fdsts_recive,"Receive.txt",FA_WRITE)==FR_OK) 
			{	
				f_lseek(&fdsts_recive,serialize_pack.pack.BLOCK*128+5);                         	 //�ƶ��ļ�ָ��
				f_write(&fdsts_recive,serialize_pack.pack.DATA,serialize_pack.pack.BYTE,&readnum);	 
				f_close(&fdsts_recive);	  							
			} 
			else
			{
				return;
			}
			if(receiveCounter == serialize_pack.pack.TOTAL)		//���ݰ��������
			{
				receiveCounter=0;
				//cleanReceiveData();
				receiveNByte_ok = 1;				//���ݰ�������ɱ�־λ
			}
			sendHead(serialize_pack.pack.ADDRESS,
					serialize_pack.pack.CMD,
					serialize_pack.pack.BLOCK,
					serialize_pack.pack.BYTE,
					serialize_pack.pack.TOTAL);		//����Ӧ��
			cleanReceiveData();						//������ջ���ͱ�־λ
		}
	}   
	receive_ok = 0;								//��ȡ���֮�󽫱�־λ��λ���Ա����´ν�������
}

//������������
//����: num��ת������ head:��ʼλ�� tail:����λ��
void exchangeOrder(u8 *num,u8 head,u8 tail)
{
	u8 temp = 0,i = 0,time = ((tail+1)-head)/2;
	for(;time > 0;tail--)
	{
		temp = num[head+i];
		num[head+i] = num[tail];
		num[tail] = temp;
		i++;
		time--;
	}
}
//��������
//������data�������͵����� cmd�����addr����ַ    byte�� ���ֽ���     outTime(ms)����ʱʱ��
//����ֵ 0���ɹ� 1����ʱ 2:���ط����� 3:ͨ�Ŵ���
u8 sendNByte(u8 *data,u8 cmd,u8 addr,u32 byte,u16 outTime)
{
	u8 reSendTimes;					//�ط�����
	u8 ret = 0;						//����ֵ
	u16 outTimeX = 0,				//��װ��ʱʱ����
		block = 0;					//���ݿ������
	u16 quotient = byte/128;		//��Ҫ���͵����ݿ���
	u8	remainder = byte%128;		//���һ������Ч���ֽ���
	if(remainder != 0){quotient++;}
	receiveSwitch = 1;				//��������ʱ����������
	for(;quotient > 0;quotient--)
	{
		for(reSendTimes = ReSendTimes;reSendTimes > 0;reSendTimes--)
		{
			cleanReceiveData();											//��ʼ����������
			if((quotient != 1)||(remainder == 0))
			{
				sendBlock(data+block*128,addr,cmd,block,128,byte);		//���Ͱ�ͷ	
			}else
			{
				sendBlock(data+block*128,addr,cmd,block,remainder,byte);//���Ͱ�ͷ	
			}
			outTimeX = outTime;
			while((Z_receive_len < 20) && (outTimeX != 0))				//�ȴ�Ӧ������
			{
				outTimeX--;
				delay_ms(1);
			}
			exchangeOrder(serialize_pack.buf,6,7);
			exchangeOrder(serialize_pack.buf,9,12);
			exchangeOrder(serialize_pack.buf,13,16);
			exchangeOrder(serialize_pack.buf,17,20);	//���ø�λ��ǰ���ͣ�������Ҫ���ߵ�λ����
			if(outTimeX == 0){continue;}				//�����ʱ�����·���
			if((serialize_pack.packHead.HEAD != Z_HEAD)
				||(serialize_pack.packHead.ADDRESS != addr)
				||(serialize_pack.packHead.CMD != cmd)
				||(serialize_pack.packHead.BLOCK != block)
				||(serialize_pack.packHead.TOTAL != byte))	//�ж�Ӧ���Ƿ���ȷ
			{continue;}		//�������ط�
			else
			{break;}		//��ȷ�ͷ�����һ������
		}
		block++;
		if(outTimeX == 0){ret = 1;break;}						//���س�ʱ
		if(reSendTimes == 0){ret = 2;break;}				
		if(serialize_pack.pack.ADDRESS != addr){ret = 3;break;}	//��ַӦ��ƥ��
	}
	receiveSwitch = 0;		//���������ݴ򿪽���
	return ret;
}

//����1�жϷ�������������յ�������
u8 rec_num=0;
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	Zigbe_INIT=1;
	if(Zigbe_INIT==1)
	{
		if((USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) && (receive_ok != 1))  //�����ж�
		{
			serialize_pack.buf[Z_receive_len++] = USART_ReceiveData(USART1);//��ȡ���յ�������
			if(Z_receive_len == 1)	//���Ұ�ͷ
			{
				if(serialize_pack.buf[0] != (u8)((Z_HEAD&0xff000000)>>24))
				{cleanReceiveData();}
			}
			if(Z_receive_len == 4)	//У���ͷ
			{
				exchangeOrder(serialize_pack.buf,0,3);
				if(serialize_pack.pack.HEAD != Z_HEAD)
				{cleanReceiveData();}	//�����ͷ���ԣ����½���
			}
			if(Z_receive_len == 149)
			{
				exchangeOrder(serialize_pack.buf,6,7);
				exchangeOrder(serialize_pack.buf,9,12);
				exchangeOrder(serialize_pack.buf,141,144);
				exchangeOrder(serialize_pack.buf,145,148);	//���ø�λ��ǰ���ͣ�������Ҫ���ߵ�λ����
				CRC_ResetDR();        						//��λCRC
				if((CRC32(serialize_pack.buf,4,140) != serialize_pack.pack.CRC_DATA)
					|| (serialize_pack.pack.TAIL != Z_TAIL))	//����CRCУ���Լ�У���β
				{
					cleanReceiveData();	//���CRCУ������Լ���β�������½���
				}else
				{
					Z_receive_len = 0;
					receive_ok = 1;		//������ɣ���λ��־λ
				}
			}
			if((receive_ok == 1) && (receiveSwitch == 0))
			{
				zigBeeReceiveData();	//����У�����ݲ�����Ӧ��
			}
		} 
	}
	else
	{		
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)   //�����ж�
		{
			rec_init[rec_num]= USART_ReceiveData(USART1);//��ȡ���յ�������
			rec_num++;
		}
	}
//	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ClearFlag(USART1, USART_FLAG_RXNE);
} 



/*********************************************************************
*��    �ܣ��趨ģ��PIN IDΪ�ض�ֵXX XX
*��ڲ�����PIN ID	0001-FF00
*���ڲ�����PIN ID
*********************************************************************/
u16 SET_PIN_ID(u8 high,u8 low)
{
	u8 sum;
	sum=(0XFC+0X02+0X91+0X01+high+low)&0XFF;
	sendByte(0XFC);
	sendByte(0X02);
	sendByte(0X91);
	sendByte(0X01);
	sendByte(high);
	sendByte(low);
	sendByte(sum);
	delay_ms(100);
	return (rec_init [0]<<8)+rec_init [1];	
}


/*********************************************************************
*��    �ܣ���ȡģ��PIN ID
*��ڲ�����
*���ڲ�����PIN ID
*********************************************************************/
u16 READ_PIN_ID(void)
{
	u8 sum;
	sum=(0XFC+0X00+0X91+0X03+0XA3+0XB3)&0XFF;
	sendByte(0XFC);
	sendByte(0X00);
	sendByte(0X91);
	sendByte(0x01);
	sendByte(0X03);
	sendByte(0XB3);
	sendByte(sum);
	delay_ms(100);
	return (rec_init[0]<<8)+rec_init[1];	
}


/*********************************************************************
*��    �ܣ�����ģ�鴮�ڲ�����
*��ڲ�����
*���ڲ�����PIN IDbound
*********************************************************************/
void SET_BOUND(u8 bound)
{
	u8 sum;
	sum=(0XFC+0X01+0X91+0X06+bound+0XF6)&0XFF;
	sendByte(0XFC);
	sendByte(0X01);
	sendByte(0X91);
	sendByte(0X06);
	sendByte(bound);
	sendByte(0XF6);
	sendByte(sum);
	delay_ms(100);
		
}

/*********************************************************************
*��    �ܣ�����ģ��Ϊ��ģʽ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void SET_COORDINATOR(void)
{
	u8 sum;
	sum=(0XFC+0X00+0X91+0X09+0XA9+0XC9)&0XFF;
	sendByte(0XFC);
	sendByte(0X00);
	sendByte(0X91);
	sendByte(0X09);
	sendByte(0XA9);
	sendByte(0XC9);
	sendByte(sum);
	delay_ms(100);
		
}


/*********************************************************************
*��    �ܣ�����ģ��Ϊ��ģʽ
*��ڲ�����
*���ڲ�����
*********************************************************************/
void SET_ROUTER(void)
{
	u8 sum;
	sum=(0XFC+0X00+0X91+0X0A+0XBA+0XDA)&0XFF;
	sendByte(0XFC);
	sendByte(0X00);
	sendByte(0X91);
	sendByte(0X0A);
	sendByte(0XBA);
	sendByte(0XDA);
	sendByte(sum);
	delay_ms(100);
		
}


/*********************************************************************
*��    �ܣ���ȡ�ڵ�����
*��ڲ�����
*���ڲ�����0 ��ģʽ��1��ģʽ
*********************************************************************/
u8 READ_NODE(void)
{
	u8 sum;
	sum=(0XFC+0X00+0X91+0X0B+0XCB+0XEB)&0XFF;
	sendByte(0XFC);
	sendByte(0X00);
	sendByte(0X91);
	sendByte(0X0B);
	sendByte(0XCB);
	sendByte(0XEB);
	sendByte(sum);
	delay_ms(100);
	if(rec_init[5]==0X69)		//��ģʽ
	{
		return 0;
	}
	else if(rec_init[5]==0X72)	//��ģʽ
	{
		return 1;
	}
}


/*********************************************************************
*��    �ܣ�����ģ��Ƶ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void SET_CHANNEL(u8 Channel)
{
	u8 sum;
	sum=(0XFC+0X01+0X91+0X0C+Channel+0X1A)&0XFF;
	sendByte(0XFC);
	sendByte(0X01);
	sendByte(0X91);
	sendByte(0X0C);
	sendByte(Channel);
	sendByte(0X1A);
	sendByte(sum);
	delay_ms(100);
		
}


/*********************************************************************
*��    �ܣ���ȡģ��Ƶ��
*��ڲ�����
*���ڲ�����Ƶ��
*********************************************************************/
u8 READ_CHANNEL(void)
{
	u8 sum;
	sum=(0XFC+0X00+0X91+0X0D+0X34+0X2B)&0XFF;
	sendByte(0XFC);
	sendByte(0X00);
	sendByte(0X91);
	sendByte(0X0D);
	sendByte(0X34);
	sendByte(0X2B);
	sendByte(sum);
	delay_ms(100);
	return rec_init[5];	
}



/*********************************************************************
*��    �ܣ����ݴ���ģʽ
*��ڲ�����ģʽ
*���ڲ�����
*********************************************************************/
void DATA_TRANS_MODE(u8 mode)
{
	u8 sum;
	sum=(0XFC+0X01+0X91+0X64+0X58+mode)&0XFF;
	sendByte(0XFC);
	sendByte(0X01);
	sendByte(0X91);
	sendByte(0X64);
	sendByte(0X58);
	sendByte(mode);
	sendByte(sum);
	delay_ms(100);
}
	


/*********************************************************************
*��    �ܣ�ģ������
*��ڲ�����
*���ڲ�����
*********************************************************************/
void MODULE_SOFTWARE_RESTARRT(void)
{
	u8 sum;
	sum=(0XFC+0X00+0X91+0X87+0X6A+0X35)&0XFF;
	sendByte(0XFC);
	sendByte(0X00);
	sendByte(0X91);
	sendByte(0X87);
	sendByte(0X6A);
	sendByte(0X35);
	sendByte(sum);
	delay_ms(1000);
	delay_ms(1000);
}


/*********************************************************************
*��    �ܣ�zigbee����������
*��ڲ�����
*���ڲ�����
*********************************************************************/
void SetBound(void)
{
	u8 i;
	u32 Bound=0;
	u32 LastBound=0;
	for(i=1;i<=2;i++)
	{
		uart1_init(115200/i);     //Zigbee��ʼ��
		delay_ms(10);
		SET_BOUND(0X01);			//������9600
		if(rec_init[2]==9)
		{
			uart1_init(9600);     //Zigbee��ʼ��
			return;
		}	
	}
	LastBound=76800;
	for(i=0;i<5;i++)
	{
		Bound=LastBound/2;
		uart1_init(Bound);     //Zigbee��ʼ��
		LastBound=Bound;
		
		
		delay_ms(10);
		SET_BOUND(0X01);			//������9600
		if(rec_init[2]==9)
		{
			uart1_init(9600);     //Zigbee��ʼ��
			return;
		}	
	}	

}

/*********************************************************************
*��    �ܣ�zigbee��ʼ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Zigbee_Init(void)
{	
	u8 i;

	char PIN_ID[5]={0};
	char CHANNEL[3]={0};
	u8 PIN_ID_H=0;
	u8 PIN_ID_L=0;
	u8 ChannelNum;
	FRESULT res;
	if(f_open(&fdsts,"config.txt",FA_READ)==FR_OK)
	{
		f_lseek(&fdsts,7);                          	 	//�ƶ��ļ�ָ��
		f_read(&fdsts,PIN_ID,4,&readnum);
		f_lseek(&fdsts,21);                          	 	//�ƶ��ļ�ָ��
		f_read(&fdsts,CHANNEL,2,&readnum);
		f_close(&fdsts);
	}
	PIN_ID_H=(PIN_ID[0]-0x30)*16+PIN_ID[1]-0x30;
	PIN_ID_L=(PIN_ID[2]-0x30)*16+PIN_ID[3]-0x30;
	ChannelNum=my_atoi(CHANNEL);
	SetBound();		//zigbee����������
	rec_num=0;
	SET_ROUTER();				//��ģʽ
	rec_num=0;
	SET_PIN_ID(PIN_ID_H,PIN_ID_L);		//PIN ID 12 34
	rec_num=0;
	SET_CHANNEL(ChannelNum);			//Ƶ��12
	rec_num=0;
	MODULE_SOFTWARE_RESTARRT();	//ģ������
	Zigbe_INIT=1;			//zigbee��ʼ����
	
}

