#include "timer.h"
#include "lcd.h"
#include "wdg.h"


/*********************************************************************
*��    �ܣ���ʱ��6��ʼ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Tim6_Init(void)
{

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);	 	//ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��			��ʱ5s
	TIM_TimeBaseStructure.TIM_Period = 49999; 				 	//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =7199; 					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	 	//����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); 			//����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;  			//TIM6�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  		//�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  							//��ʼ��NVIC�Ĵ���

	TIM_ITConfig(TIM6,TIM_IT_Update,DISABLE ); 					//ʹ��ָ����TIM6�ж�,��������ж�
	TIM_Cmd(TIM6, DISABLE);  									//ʹ��TIMx					 
//	TIM6->DIER&=~0x0001;		 								//ʧ��ָ����TIM6�ж�,��������ж�
//	TIM6->CR1&=~0x0001;		 									//ʧʹ��TIMx	

}

/*********************************************************************
*��    �ܣ���ʱ��6�жϷ������
*��ڲ�����
*���ڲ�����
*********************************************************************/
void TIM6_IRQHandler(void)   									//TIM3�ж�
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)  		//���TIM3�����жϷ������
		{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update  ); 			//���TIMx�����жϱ�־ 
		LCD_LED = 0; 											//�رձ���
		TIM6->DIER&=~0x0001;		 							//ʧ��ָ����TIM6�ж�,��������ж�
		TIM6->CR1&=~0x0001;										//ʧʹ��TIMx	
		}
}




/*********************************************************************
*��    �ܣ���ʱ��7��ʼ��
*��ڲ�����
*���ڲ�����
*********************************************************************/
void Tim7_Init(void)
{

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);	 	//ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��			��ʱ5s
	TIM_TimeBaseStructure.TIM_Period = 49999; 				 	//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =7199; 					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	 	//����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); 			//����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;  			//TIM6�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  		//�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  							//��ʼ��NVIC�Ĵ���

	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE ); 					//ʹ��ָ����TIM7�ж�,��������ж�
	TIM_Cmd(TIM7, DISABLE);  									//ʹ��TIMx					 
}

/*********************************************************************
*��    �ܣ���ʱ��7�жϷ������
*��ڲ�����
*���ڲ�����
*********************************************************************/
void TIM7_IRQHandler(void)   									
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)  		//���TIM7�����жϷ������
	{
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  ); 			//���TIMx�����жϱ�־ 
											
		IWDG_Feed();//ι��
		
		
	}
}









