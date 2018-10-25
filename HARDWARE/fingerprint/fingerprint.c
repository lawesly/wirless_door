#include "fingerprint.h"
#include "delay.h"
#include "lcd.h"
#include "text.h"


u8 u3receive_short_ok = 0;			//����3����8�ֽ���ɱ�־
u8 u3receive_long_ok = 0;			//����3����>8�ֽ���ɱ�־
u16 u3receive_len = 0;				//����3     8�ֽڽ��ռ�����
u8 receive_short_ok = 0;			//����2����8�ֽ���ɱ�־
u8 receive_long_ok = 0;				//����2����>8�ֽ���ɱ�־
u16 receive_len = 0;				//����2     8�ֽڽ��ռ�����
//u8 i = 0;					//forѭ���ñ���
Serialize_Pack receive_pack;
Show_Text show_text;
u8 receiveMode = 0;				//����ģʽ 0: 8�ֽ�  1: >8�ֽ�
u16 dataLen = 0;						//>8�ֽڽ��ռ�����
u8 receiveMore[200];				//>8�ֽڽ��ջ���


//=========ȫ�ֱ���==========
u8 gTxBuf[207];				 	//���з��ͻ���

u8 getUserCount(USART_TypeDef* USARTx);

void ResetFingerFlag(void)		//ָ��ģ���־λ��λ
{
	u8 i;	
	u3receive_short_ok = 0;			//����3����8�ֽ���ɱ�־
	u3receive_long_ok = 0;			//����3����>8�ֽ���ɱ�־
	u3receive_len = 0;			//����3     8�ֽڽ��ռ�����
	receive_short_ok = 0;			//����8�ֽ���ɱ�־
	receive_long_ok = 0;			//����>8�ֽ���ɱ�־
	receive_len = 0;			//8�ֽڽ��ռ�����u8 receiveMode = 0;			//����ģʽ 0:8�ֽ�  1:>8�ֽ�
	receiveMode = 0;						//����ģʽ 0:8�ֽ�  1:>8�ֽ�
	dataLen = 0;								//>8�ֽڽ��ռ�����
	for(i=0;i<200;i++)
	{receiveMore[i]=0;	}					
	for(i=0;i<8;i++)
	{receive_pack.buf[i]=0;	}
}


void TxByte(USART_TypeDef* USARTx,u8 temp)
{
	while((USARTx->SR&0X40)==0);
	USARTx->DR = temp;
}

/******************���ڷ����ӳ���bit SendUART(U8 Scnt,U8 Rcnt,U8 Delay)******/
/*���ܣ���DSP��������********************************************************/
/*������Scnt�����ֽ�����Rcnt�����ֽ����� Delay��ʱ�ȴ���*********************/
/*����ֵ��TRUE �ɹ���FALSE ʧ��**********************************************/
u8 TxAndRsCmd(USART_TypeDef* USARTx,u16 Scnt, u16 Rcnt, u8 Delay)
{
	u16  i,CheckSum;
	u32 RsTimeCnt;
	TxByte(USARTx,CMD_HEAD);		//��־ͷ	 
	CheckSum = 0;
	for (i = 0; i < Scnt; i++)
	{
		TxByte(USARTx,gTxBuf[i]);		 
		CheckSum ^= gTxBuf[i];
	}	
	TxByte(USARTx,CheckSum);
	TxByte(USARTx,CMD_TAIL); 		//��־β 
	for(i = 0;i < 8;i++) {receive_pack.buf[i] = 0;}	//��ս��ջ���
	RsTimeCnt = Delay * 120000;
	
	if(USARTx==USART2)
	{
		receive_len = 0;
		receive_short_ok = 0;
		receive_long_ok = 0;
		while (receive_len < Rcnt && RsTimeCnt > 0)
		RsTimeCnt--;
		if (receive_len != Rcnt) return ACK_TIMEOUT;
	}
	else if(USARTx==USART3)
	{
		u3receive_len = 0;
		u3receive_short_ok = 0;
		u3receive_long_ok = 0;	
		while (u3receive_len < Rcnt && RsTimeCnt > 0)
		RsTimeCnt--;
		if (u3receive_len != Rcnt) return ACK_TIMEOUT;
	}
	if (receive_pack.pack.HEAD != CMD_HEAD) return ACK_FAIL;
	if (receive_pack.pack.TAIL != CMD_TAIL) return ACK_FAIL;
	if (receive_pack.pack.CMD != (gTxBuf[0])) return ACK_FAIL;

	return ACK_SUCCESS;
}	 

/*********************************************************************
*��    �ܣ���֤Ȩ���Ƿ��ںϷ���Χ
*��ڲ�������
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��27�� 13:13:16
*********************************************************************/
u8 IsMasterUser(u8 UserID)
{
    if ((UserID == 1) || (UserID == 2) || (UserID == 3)) return TRUE;
			else  return FALSE;
}	 

/*********************************************************************
*��    �ܣ�ʹģ���������״̬������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��26�� 16:55:47
*********************************************************************/
u8 setLpMode(USART_TypeDef* USARTx)
{
  u8 m;
	
	gTxBuf[0] = CMD_LP_MODE;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
	  return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ�����/��ȡָ�����ģʽ������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�����com:  0:�����µ����ģʽ 1����ȡ��ǰ���ģʽ
					 mode: 0:�����ظ�  1:��ֹ�ظ�
					��ȡ�����ģʽ������gRsBuf[3]
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��26�� 16:51:01
*********************************************************************/
u8 setAndReadMode(USART_TypeDef* USARTx,u8 com,u8 mode)
{
  u8 m;	
	gTxBuf[0] = CMD_SET_READ_ADDMODE;
	gTxBuf[1] = 0;
	gTxBuf[2] = mode;
	gTxBuf[3] = com;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
	  return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ����ָ�ƣ�����/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�����userNum:�û���[1:4095]
		  lim:Ȩ��[1:3]
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��26�� 21:30:43
*********************************************************************/
u8 addUser(USART_TypeDef* USARTx,u16 userNum,u8 lim)
{
	u8 m;
	u16 userCount;	
	getUserCount(USARTx);
	userCount = (receive_pack.pack.Q1<<8)+receive_pack.pack.Q2;
	if (userCount >= USER_MAX_CNT)
		return ACK_FULL;
	
	gTxBuf[0] = CMD_ADD_1;
	gTxBuf[1] = userNum >> 8;		//�û��Ÿ߰�λ
	gTxBuf[2] = userNum & 0xFF;		//�û��ŵͰ�λ
	gTxBuf[3] = lim;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 200);
	
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
	    gTxBuf[0] = CMD_ADD_2;
			m = TxAndRsCmd(USARTx,5, 8, 200);
			if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
			{
					gTxBuf[0] = CMD_ADD_3;
					m = TxAndRsCmd(USARTx,5, 8, 200);
					if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
					{
							return ACK_SUCCESS;
					}
					else
						return ACK_FAIL;
			}
			else
				return ACK_FAIL;
	}
	else
		return ACK_FAIL;
}

/*********************************************************************
*��    �ܣ�ɾ��ָ���û�������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�����userNum:�û���[1:4095]
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��27�� 13:00:42
*********************************************************************/
u8 deleteOneUser(USART_TypeDef* USARTx,u16 userNum)
{
	u8 m;
	
	gTxBuf[0] = CMD_DEL_ONE;
	gTxBuf[1] = userNum >> 8;		//�û��Ÿ߰�λ
	gTxBuf[2] = userNum & 0xFF;		//�û��ŵͰ�λ
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
		return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ�ɾ�������û�������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��27�� 13:03:02
*********************************************************************/
u8 deleteAllUser(USART_TypeDef* USARTx)
{
 	u8 m;
	
	gTxBuf[0] = CMD_DEL_ALL;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;
	
	m = TxAndRsCmd(USARTx,5, 8, 50);
	
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{	    
		return ACK_SUCCESS;
	}
	else
	{
		return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ�ȡ�û�����������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
		  �û����Ͱ�λ������gRsBuf[3]���߰�λ������gRsBuf[2]
*ʱ�䣺2015��6��27�� 13:06:52
*********************************************************************/
u8 getUserCount(USART_TypeDef* USARTx)
{
	u8 m;
	gTxBuf[0] = CMD_USER_CNT;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{	    
		return ACK_SUCCESS;
	}
	else
	{
		return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ��ȶ� 1:1������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�����userNum:�û���[1:4095]
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��27�� 13:11:24
*********************************************************************/
u8 matchOne(USART_TypeDef* USARTx,u8 userNum)
{
  u8 m;
	
	gTxBuf[0] = CMD_MATCH_ONE;
	gTxBuf[1] = userNum >> 8;		//�û��Ÿ߰�λ
	gTxBuf[2] = userNum & 0xFF;		//�û��ŵͰ�λ
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{	    
		return ACK_SUCCESS;
	}
	else
	{
		return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ��ȶ� 1:N������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�������
*���ڲ�����״̬
		  �û��ŵͰ�λ������gRsBuf[3]���߰�λ������gRsBuf[2]��Ȩ�޴�����gRsBuf[4]
*ʱ�䣺2015��6��27�� 13:14:39
*********************************************************************/
u8 matchN(USART_TypeDef* USARTx)
{
	u8 m;
	
	gTxBuf[0] = CMD_MATCH_N;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;
	
	m = TxAndRsCmd(USARTx,5, 8, 150);
	
	if ((m == ACK_SUCCESS) && (IsMasterUser(receive_pack.pack.Q3) == TRUE))
	{	
		return ACK_SUCCESS;
	}
	else if(receive_pack.pack.Q3 == ACK_NO_USER)
	{
		return ACK_NO_USER;
	}
	else if(receive_pack.pack.Q3 == ACK_TIMEOUT)
	{
		return ACK_TIMEOUT;
	}
	else
	{
		return ACK_GO_OUT;
	}
}


/*********************************************************************
*��    �ܣ��ȶ� 1:N������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�������
*���ڲ�����״̬
		  �û��ŵͰ�λ������gRsBuf[3]���߰�λ������gRsBuf[2]��Ȩ�޴�����gRsBuf[4]
*ʱ�䣺2015��6��27�� 13:14:39
*********************************************************************/
void fastMatchN(USART_TypeDef* USARTx)
{
	TxByte(USARTx,CMD_HEAD);
	TxByte(USARTx,CMD_MATCH_N);
	TxByte(USARTx,0x00);
	TxByte(USARTx,0x00);
	TxByte(USARTx,0x00);
	TxByte(USARTx,0x00);
	TxByte(USARTx,0x0C);
	TxByte(USARTx,CMD_TAIL);
}

/*********************************************************************
*��    �ܣ�ȡ�û�Ȩ�ޣ�����/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�����userNum���û���
		  ��ȡ�û�Ȩ�޴�����gRsBuf[4]
*���ڲ������ɹ���ʧ��
*ʱ�䣺2015��6��26�� 16:51:01
*********************************************************************/
u8 getLim(USART_TypeDef* USARTx,u16 userNum)
{
  u8 m;
	
	gTxBuf[0] = CMD_GETLIM;
	gTxBuf[1] = userNum >> 8;
	gTxBuf[2] = userNum & 0xFF;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
	  return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ�ȡ DSP ģ��汾�ţ�����Ϊ 8 �ֽ�/Ӧ��>8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
		  �汾�Ŵ�����:gRsBuf[9]:gRsBuf[17]
*ʱ�䣺2015��6��27�� 17:00:25
*********************************************************************/
u8 getVer(USART_TypeDef* USARTx)
{
	u8 m;
	
	gTxBuf[0] = CMD_GET_VER;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 20, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ�����/��ȡ�ȶԵȼ�������/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�����cmd   0:�����µıȶԵȼ� 1:��ȡ��ǰ�ȶԵȼ�
		  level 0:�±ȶԵȼ� 1:��ǰ�ȶԵȼ�
*���ڲ������ɹ���ʧ��
		  ��ȡ�ıȶԵȼ�������:gRsBuf[3]
*ʱ�䣺2015��6��27�� 16:59:58
*********************************************************************/
u8 setAndReadLevel(USART_TypeDef* USARTx,u8 cmd,u8 level)
{
	u8 m;
	
	gTxBuf[0] = CMD_COM_LEV;
	gTxBuf[1] = 0;
	gTxBuf[2] = level;
	gTxBuf[3] = cmd;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ��ɼ�ͼ����ȡ����ֵ�ϴ�������Ϊ 8 �ֽ�/Ӧ��>8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
		  ��ȡ�ıȶԵȼ�������:gRsBuf[12]:gRsBuf[204]
*ʱ�䣺2015��6��27�� 16:59:58
*********************************************************************/
u8 getEigen(USART_TypeDef* USARTx)
{
	u8 m;
	
	gTxBuf[0] = CMD_GET_EIGEN;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 207, 100);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}


/*********************************************************************
*��    �ܣ��´�ָ������ֵ�� DSP ģ�����ݿ�ָ�Ʊȶ� 1�� 1������>8 �ֽ�/Ӧ��Ϊ 8 �ֽڣ�
*��ڲ���������ֵ
*���ڲ������ɹ���ʧ��
		  �û��Ÿ�λ������:gRsBuf[2]
		  �û��ŵ�λ������:gRsBuf[3]
		  �û�Ȩ�޴�����:gRsBuf[4]
*ʱ�䣺2015��9��6�� 8:47:39
*********************************************************************/
u8 downloadAndMatchONE(USART_TypeDef* USARTx,u16 userNum,u8 *eigen)
{
	u8 m;
	u16 i;
	
	gTxBuf[0] = CMD_DOWN_MATCH_ONE;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0xC4;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	gTxBuf[5] = 0x86;	
	gTxBuf[6] = CMD_TAIL;	
	gTxBuf[7] = CMD_HEAD;	
	gTxBuf[8] = userNum>>8;
	gTxBuf[9] = userNum&0xFF;
	gTxBuf[10] = 0;	
	for(i = 11;i<=204;i++)
	{
		gTxBuf[i] = eigen[i-11];
	}
	m = TxAndRsCmd(USARTx,204, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}


/*********************************************************************
*��    �ܣ��´�ָ������ֵ�� DSP ģ�����ݿ�ָ�Ʊȶ� 1�� N������>8 �ֽ�/Ӧ��Ϊ 8 �ֽڣ�
*��ڲ���������ֵ
*���ڲ������ɹ���ʧ��
		  �û��Ÿ�λ������:gRsBuf[2]
		  �û��ŵ�λ������:gRsBuf[3]
		  �û�Ȩ�޴�����:gRsBuf[4]
*ʱ�䣺2015��6��29�� 13:39:39
*********************************************************************/
u8 downloadAndMatchN(USART_TypeDef* USARTx,u8 *eigen)
{
	u8 m;
	u16 i;
	
	gTxBuf[0] = CMD_DOWN_MATCH_N;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0xC4;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	gTxBuf[5] = 0x87;	
	gTxBuf[6] = CMD_TAIL;	
	gTxBuf[7] = CMD_HEAD;	
	gTxBuf[8] = 0;
	gTxBuf[9] = 0;
	gTxBuf[10] = 0;	
	for(i = 11;i<=204;i++)
	{
		gTxBuf[i] = eigen[i-11];
	}
	m = TxAndRsCmd(USARTx,204, 8, 10);
		
	if (m == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else if(receive_pack.pack.Q3 == ACK_NO_USER)
	{
		return ACK_NO_USER;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ��ϴ� DSP ģ�����ݿ���ָ���û�����ֵ������Ϊ 8 �ֽ�/Ӧ��>8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
		  �ϴ�������ֵ������:gRsBuf[12]:gRsBuf[204]
		  �û��Ÿ�λ������:gRsBuf[9]
		  �û��ŵ�λ������:gRsBuf[10]
		  �û�Ȩ�޴�����:gRsBuf[11]
*ʱ�䣺2015��6��27�� 19:38:55
*********************************************************************/
u8 uploadEigen(USART_TypeDef* USARTx,u16 userNum)
{
	u8 m;
	
	gTxBuf[0] = CMD_UP_EIGEN;
	gTxBuf[1] = userNum >> 8;
	gTxBuf[2] = userNum & 0xFF;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 207, 100);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ��´�����ֵ����ָ���û��Ŵ��� DSP ģ�����ݿ⣨����>8 �ֽ�/Ӧ��Ϊ 8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
		  ��ȡ�ıȶԵȼ�������:gRsBuf[12]:gRsBuf[204]
		  �û��Ÿ�λ������:gRsBuf[9]
		  �û��Ÿ�λ������:gRsBuf[10]
		  �û�Ȩ�޴�����:gRsBuf[11]
*ʱ�䣺2015��6��27�� 19:38:55
*********************************************************************/
u8 downloadAddUser(USART_TypeDef* USARTx,u16 userNum,u8 lim,u8 *eigen)
{
	u8 m;
	u16 i,userCount;	//,delay = 2000
	
	getUserCount(USARTx);	//ȡ�û�����
	userCount = (receive_pack.pack.Q1<<8)+receive_pack.pack.Q2;
	if (userCount >= USER_MAX_CNT)
		return ACK_FULL;
	
	ResetFingerFlag();	//ָ��ģ���־λ��λ
	gTxBuf[0] = CMD_DOWN_ADD;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0xc4;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	gTxBuf[5] = 0x85;	
	gTxBuf[6] = CMD_TAIL;	
	gTxBuf[7] = CMD_HEAD;	
	gTxBuf[8] = userNum >> 8;
	gTxBuf[9] = userNum & 0xFF;
	gTxBuf[10] = lim;	
	for(i = 11;i<204;i++)
	{
		gTxBuf[i] = eigen[i-11];
	}
	ResetFingerFlag();	//ָ��ģ���־λ��λ
	m = TxAndRsCmd(USARTx,204, 8, 10);
			
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ�ȡ�ѵ�¼�����û��û��ż�Ȩ�ޣ�����Ϊ 8 �ֽ�/Ӧ��>8 �ֽڣ�
*��ڲ�������
*���ڲ������ɹ���ʧ��
		  �û�����λ������:gRsBuf[9]
		  �û�����λ������:gRsBuf[10]
		  �û�Ȩ�޴�����:gRsBuf[11]
		  �Ժ�ÿ���ֽڷֱ𴢴�:�û��Ÿ߰�λ���û��ŵͰ�λ��Ȩ��
*ʱ�䣺2015��6��27�� 20:09:55
*********************************************************************/
u8 getAllUser(USART_TypeDef* USARTx)
{
	u8 m,num;
	
	getUserCount(USARTx);
	num = (receive_pack.pack.Q1<<8)+receive_pack.pack.Q2;
	gTxBuf[0] = CMD_GET_USERNUM;
	gTxBuf[1] = 0;
	gTxBuf[2] = 0;
	gTxBuf[3] = 0;
	gTxBuf[4] = 0;	
	m = TxAndRsCmd(USARTx,5, (3*num+2)+11, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

/*********************************************************************
*��    �ܣ�����/��ȡָ�Ʋɼ��ȴ���ʱʱ�䣨����/Ӧ���Ϊ 8 �ֽڣ�
*��ڲ�����cmd 0:�����µĳ�ʱʱ��1:��ȡ��ǰ��ʱʱ��
		  timeOut:[0:255] ����ֵΪ 0������ָ�ư�ѹ��ָ�Ʋɼ����̽�һֱ����
					����ֵ��0����tout*T0 ʱ��������ָ�ư�ѹ��ϵͳ����ʱ�˳���
*���ڲ������ɹ���ʧ��
		  ��ǰ��ʱʱ�䴢����:gRsBuf[3]
*ʱ�䣺2015��6��27�� 20:09:55
*********************************************************************/
u8 setAndGetTimeOut(USART_TypeDef* USARTx,u8 cmd,u8 timeOut)
{
	u8 m;
	
	gTxBuf[0] = CMD_TIMEOUT;
	gTxBuf[1] = 0;
	gTxBuf[2] = timeOut;
	gTxBuf[3] = cmd;
	gTxBuf[4] = 0;	
	
	m = TxAndRsCmd(USARTx,5, 8, 10);
		
	if (m == ACK_SUCCESS && receive_pack.pack.Q3 == ACK_SUCCESS)
	{
		return ACK_SUCCESS;
	}
	else
	{
	 	return ACK_FAIL;
	}
}

void Get_Showdata()
{
	sprintf(show_text.HEAD,"0x%X",receive_pack.pack.HEAD);
	sprintf(show_text.CMD,"0x%X",receive_pack.pack.CMD);
	sprintf(show_text.Q1,"0x%X",receive_pack.pack.Q1);
	sprintf(show_text.Q2,"0x%X",receive_pack.pack.Q2);
	sprintf(show_text.Q3,"0x%X",receive_pack.pack.Q3);
	sprintf(show_text.NONE,"0x%X",receive_pack.pack.NONE);
	sprintf(show_text.CHK,"0x%X",receive_pack.pack.CHK);
	sprintf(show_text.TAIL,"0x%X",receive_pack.pack.TAIL);
}

unsigned char CheckSum = 0,ret = 0;

unsigned char Transmit_Data(void)
{
	u16 i;
	CheckSum = receive_pack.pack.CMD;
	CheckSum ^= receive_pack.pack.Q1;
	CheckSum ^= receive_pack.pack.Q2;
	CheckSum ^= receive_pack.pack.Q3;
	CheckSum ^= receive_pack.pack.NONE;						//����У��ֵ
	if((CheckSum == receive_pack.pack.CHK) 
		  && (receive_pack.pack.HEAD == 0xF5) 
	    && (receive_pack.pack.TAIL == 0xF5))	{ret = 1;}
	else										{ret = 0;}	//�ж��Ƿ�Ϊ�Ϸ�����
	if(    (receive_pack.pack.CMD == 0x26) 					//ȡģ��汾��
		  || (receive_pack.pack.CMD == 0x24) 				//�ɼ�ͼ���ϴ�
	    || (receive_pack.pack.CMD == 0x23) 					//�ɼ�ͼ����ȡ����ֵ�ϴ�
			|| (receive_pack.pack.CMD == 0x31) 				//�ϴ� DSP ģ�����ݿ���ָ���û�����ֵ
			|| (receive_pack.pack.CMD == 0x2B)) 			//ȡ�ѵ�¼�����û��û��ż�Ȩ��
	{
		receiveMode = 1;									//׼�����ܽ�����������
		receive_len=0;										//����������ݼ�����
		u3receive_len=0;									//����������ݼ�����
		dataLen = receive_pack.pack.Q2 + (receive_pack.pack.Q3 << 8);//��ȡ���������ݳ���
		for(i = 0;i < dataLen + 3;i++) {receiveMore[i] = 0;} //������ջ���

	}
	return ret;
}

//����2��ʼ��
 void uart2_init(u32 bound)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//ʹ��USART2��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��GPIOAʱ��
 	USART_DeInit(USART2);  //��λ����2
	
	//USART1_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART1_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3

   //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART2, &USART_InitStructure); //��ʼ������
	USART_ClearFlag(USART2, USART_FLAG_TC);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
}

//����2�жϷ�����
void USART2_IRQHandler(void)                					//����2�жϷ������
{
	u16 i;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		if((receiveMode == 0) && (receive_short_ok == 0))		//����Ϊ8���ֽ�
		{
			receive_pack.buf[receive_len++] = USART2->DR & 0xFF;//���մ�������
			if((receive_pack.pack.HEAD != 0xF5) || (receive_pack.pack.NONE != 0))
			{
				receive_len = 0;
				for(i = 0;i < 8;i++)							//��ս��ջ���
				{
					receive_pack.buf[i] = 0;
				}
			}
			if(receive_pack.pack.CMD == 0xF5)	{receive_len--;}//�������β������ͷ���ܣ�����֮	
			if(receive_len >= 8)								//������8���ֽ�
			{
				if(Transmit_Data() == 1){receive_short_ok = 1;}
				else					{receive_short_ok = 0;}	//ͨ��У��֮����λ������ɱ�־
				Get_Showdata();
				//receive_len=0;									//����������ݼ�����
			}
		}else	if(receive_long_ok ==0)							//����ΪdataLen���ֽ�
		{
			receiveMore[receive_len++] = USART2->DR & 0xFF;		//���մ�������
			if(receiveMore[0] != 0xF5)							//�жϰ�ͷ
			{
				receive_len = 0;
				receiveMode = 0;
				for(i = 0;i < dataLen + 3;i++) {receiveMore[i] = 0;}//������ջ���
			}
			if(receive_len >= dataLen + 3)						//�ж��Ƿ���չ�����
			{
				CheckSum = 0;
				for(i = 1; i <= dataLen;i++)
				{
					CheckSum ^= receiveMore[i];
				}												//����У���
				if((receiveMore[dataLen + 2] == 0xF5) 
						&& (receiveMore[dataLen + 1] == CheckSum)){receive_long_ok = 1;}
				else											  {receive_long_ok = 0;}//�ж��յ��������Ƿ�Ϸ�
				//receive_len=0;
				receiveMode = 0;								//���ؽ���8�ֽ�ģʽ
			}
		}
	} 
} 


//����3��ʼ��
void uart3_init(u32 bound)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//ʹ��USART2��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��GPIOAʱ��
 	USART_DeInit(USART3);  //��λ����3
	
	//USART3_TX   PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB.10
   
    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //��ʼ��PB.11

   //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART3, &USART_InitStructure); //��ʼ������
	USART_ClearFlag(USART3, USART_FLAG_TC);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ��� 
}


//����3�жϷ�����
void USART3_IRQHandler(void)                					//����3�жϷ������
{
	u16 i;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		if((receiveMode == 0) && (u3receive_short_ok == 0))		//����Ϊ8���ֽ�
		{
			receive_pack.buf[u3receive_len++] = USART3->DR & 0xFF;//���մ�������
			if((receive_pack.pack.HEAD != 0xF5) || (receive_pack.pack.NONE != 0))
			{
				u3receive_len = 0;
				for(i = 0;i < 8;i++)							//��ս��ջ���
				{
					receive_pack.buf[i] = 0;
				}
			}
			if(receive_pack.pack.CMD == 0xF5)	{u3receive_len--;}//�������β������ͷ���ܣ�����֮	
			if(u3receive_len >= 8)								//������8���ֽ�
			{
				if(Transmit_Data() == 1){u3receive_short_ok = 1;}
				else					{u3receive_short_ok = 0;}	//ͨ��У��֮����λ������ɱ�־
				Get_Showdata();
			}
		}else	if(u3receive_long_ok ==0)							//����ΪdataLen���ֽ�
		{
			receiveMore[u3receive_len++] = USART3->DR & 0xFF;		//���մ�������
			if(receiveMore[0] != 0xF5)							//�жϰ�ͷ
			{
				u3receive_len = 0;
				receiveMode = 0;
				for(i = 0;i < dataLen + 3;i++) {receiveMore[i] = 0;}//������ջ���
			}
			if(u3receive_len >= dataLen + 3)						//�ж��Ƿ���չ�����
			{
				CheckSum = 0;
				for(i = 1; i <= dataLen;i++)
				{
					CheckSum ^= receiveMore[i];
				}												//����У���
				if((receiveMore[dataLen + 2] == 0xF5) 
						&& (receiveMore[dataLen + 1] == CheckSum)){u3receive_long_ok = 1;}
				else											  {u3receive_long_ok = 0;}//�ж��յ��������Ƿ�Ϸ�
				//receive_len=0;
				receiveMode = 0;								//���ؽ���8�ֽ�ģʽ
			}
		}
	} 
} 

