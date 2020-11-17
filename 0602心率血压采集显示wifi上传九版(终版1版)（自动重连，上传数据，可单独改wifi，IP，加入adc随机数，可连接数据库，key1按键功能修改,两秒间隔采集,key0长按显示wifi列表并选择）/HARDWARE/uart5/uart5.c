#include "delay.h"
#include "uart5.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 
//#include "timer.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//串口5驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

//串口接收缓存区 	
u8 UART5_RX_BUF[UART5_MAX_RECV_LEN]; 			//接收缓冲,最大UART5_MAX_RECV_LEN个字节.
u8 UART5_TX_BUF[UART5_MAX_SEND_LEN]; 			  //发送缓冲,最大UART5_MAX_SEND_LEN字节

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
u16 UART5_RX_STA=0;   	
u16 Get_data=0;

void UART5_IRQHandler(void)
{
	u8 res;	      
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(UART5);	
//        res = UART5->DR;		
		if((UART5_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(UART5_RX_STA<UART5_MAX_RECV_LEN)	//还可以接收数据
			{
					TIM_SetCounter(TIM5,0);//计数器清空        				 
					if(UART5_RX_STA==0)
						TIM5_Set(1);	 	//使能定时器5的中断 
					UART5_RX_BUF[UART5_RX_STA++]=res;		//记录接收到的值	 
			}else 
			{
				UART5_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}  				 											 
}   


//初始化IO 串口5
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void uart5_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	// GPIOC,GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE); //串口5时钟使能

 	USART_DeInit(UART5);  //复位串口5
		 //UART5_TX   PC12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PC12

	//UART5_RX	  PD2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);  //初始化PD2
	
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(UART5, &USART_InitStructure); //初始化串口5
  

	USART_Cmd(UART5, ENABLE);                    //使能串口 
	
	//使能接收中断
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	
	TIM5_Init(99,7199);		//10ms中断
	UART5_RX_STA=0;		//清零
	TIM5_Set(0);		//关闭定时器4
	TIM3_Init(14398,7199);
}

//串口5,printf 函数
//确保一次发送数据不超过UART5_MAX_SEND_LEN字节
void u5_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)UART5_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)UART5_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(UART5,UART5_TX_BUF[j]); 
	} 
}

	//定时器5中断服务程序		    
void TIM5_IRQHandler(void)
	{ 	
		if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//是更新中断
			{	 			   
				UART5_RX_STA|=1<<15;	//标记接收完成
				TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //清除TIMx更新中断标志    
				TIM5_Set(0);			//关闭TIM4  
			}	    
	}
	//设置TIM5的开关
	//sta:0，关闭;1,开启;
void TIM5_Set(u8 sta)
	{
		if(sta)
			{
				TIM_SetCounter(TIM5,0);//计数器清空
				TIM_Cmd(TIM5, ENABLE);  //使能TIMx	
			}
			else 
				TIM_Cmd(TIM5, DISABLE);//关闭定时器5	   
	}
	//通用定时器中断初始化
	//这里始终选择为APB1的2倍，而APB1为36M
	//arr：自动重装值。
	//psc：时钟预分频数		 
void TIM5_Init(u16 arr,u16 psc)
	{	
		NVIC_InitTypeDef NVIC_InitStructure;
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //时钟使能//TIM4时钟使能    
	
		//定时器TIM5初始化
		TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
		TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
		TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //使能指定的TIM4中断,允许更新中断

	 	  
		NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	}

	void TIM3_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}
//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
	  	TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
//					TIM_SetCounter(TIM3,0);//计数器清空
			Get_data = 1;
		}				
}

 



















