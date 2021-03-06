#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "uart5.h"
#include "xinlvxueya.h"
#include "lcd.h"
#include "wifi.h"
#include "string.h"
#include "key.h"
#include "led.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "stmflash.h"
#include "adc.h"
#include "touch.h"
#include "common.h"
#include "stdarg.h"

u8 wifista_ssid_buf[100]; 				//ssid缓冲.wifi模块将接收到的wifi名称储存到这里
u8 wifista_pswd_buf[100];					//pswd缓存.wifi模块将接收到的wifi密码储存到这里
u8 wifista_ipnb_buf[100];					//ipnb缓存.wifi模块将接收到的服务器ip储存到这里
u8 wifista_port_buf[50];					//port缓存.wifi模块将接收到的服务器端口号储存到这里

u8 get_wifista_ssid_buf[100]; 		//用来存储flash中读出来的wifi名称
u8 get_wifista_pswd_buf[100];			//用来存储flash中读出来的wifi密码
u8 get_server_ipnb[100];					//用来存储flash中读出来的IP地址.
u8 get_server_port[50];						//用来存储flash中读出来的端口号.


//wifi模块接收手机端发送数据的标志位
u16 ssid_sended_flag=0;
u16 pswd_sended_flag=0;
u16 ipnb_sended_flag = 0;
u16 port_sended_flag = 0;


u16 esp8266_connect_flag=0;
u16 USART2_RX_OVER = 0;

u16 wifi_state = 0;    //wifi模块连接wifi的步骤过程标志
int wifi_ok = 0;
int wifi_changed = 0;			

u16 wk_up = 0;
u16 sending = 0;
u16 out_set = 0;

//wifi LCD列表显�
u16 WRITE_WIFI[1000];
u16 position[20];  //储存+号位置的数组
u16 write_num=0;
u16 apnb=0;
u16 page1_flag=0;
u16 page2_flag=0;
u16 wifi_choose_flag=0;
//预先开设的10个储存wifi名称的数组
u8 ssid1[30],ssid2[30],ssid3[30],ssid4[30],ssid5[30];
u8 ssid6[30],ssid7[30],ssid8[30],ssid9[30],ssid10[30];

u8 getssid[30];

u16 wifi_lcd = 0;
//数据打包
SDK_CMD_INFO sdk_cmd_info;
u8 SN[4]={0x19,0x07,0x14,0x01};


char *p_gprs_buf; 
char *p_gprs_rd = NULL;
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		


u16 USART2_RX_STA=0;  

int i=0;
int wifi_reload = 0;
//usart2初始化
void usart2_init(u32 bound)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能GPIOA时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
		USART_DeInit(USART2); 	
		//TX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		//RX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
		GPIO_Init(GPIOA, &GPIO_InitStructure);  
		
//		RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//复位串口2
//		RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止复位
		
		USART_InitStructure.USART_BaudRate = bound;//波特率设置
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
		
		USART_Init(USART2, &USART_InitStructure); ; //初始化串口
		
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //使能串口2中断
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //先占优先级2级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //从优先级2级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
		NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	 
		USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//开启中断
		 
		USART_Cmd(USART2, ENABLE);                    //使能串口 
		USART_ClearFlag(USART2, USART_FLAG_TC);
		TIM4_Init(999,7199);		//10ms中断
		USART2_RX_STA=0;		//清零
		TIM4_Set(0);			//关闭定时器4
}
//usart2中断服务程序
void USART2_IRQHandler(void)               	
{
		u8 res;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
		{
		res =USART_ReceiveData(USART2);		
		if(USART2_RX_STA<USART2_MAX_RECV_LEN)		//还可以接收数据
		{
			TIM_SetCounter(TIM4,0);//计数器清空        				 
			if(USART2_RX_STA==0)TIM4_Set(1);	 	//使能定时器4的中断 
			USART2_RX_BUF[USART2_RX_STA]=res;		//记录接收到的值	
			USART2_RX_STA++;
		}else 
		{
			USART2_RX_STA|=1<<15;					//强制标记接收完成
			USART2_RX_OVER = 1;
		} 
		}			
} 

   
void TIM4_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		USART2_RX_STA|=1<<15;	//标记接收完成
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志    
		TIM4_Set(0);			//关闭TIM4  
	}	    
}
//设置TIM4的开关
//sta:0，关闭;1,开启;
void TIM4_Set(u8 sta)
{
	if(sta)
	{
		TIM_SetCounter(TIM4,0);//计数器清空
		TIM_Cmd(TIM4, ENABLE);  //使能TIMx	
	}else TIM_Cmd(TIM4, DISABLE);//关闭定时器4	   
}
//通用定时器中断初始化
//这里始终选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数		 
void TIM4_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能//TIM4时钟使能    
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //使能指定的TIM4中断,允许更新中断

	 	  
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}
/*******************************************************************************
* 函数名  : UART2_SendString
* 描述    : USART2发送字符串
* 输入    : *s字符串指针
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void UART2_SendString(char* s)
{
	while(*s != '\0')//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET); 
		USART_SendData(USART2 ,*s++);//发送当前字符
	}
}

//发送数据
void Send_DATA(u8 *buf,u32 len)
{
	u16 t;
	for(t=0;t<len ;t++)		//循环发送数据
	{
	  	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束		
    	USART_SendData(USART2,buf[t]); //发送数据	
		printf("%c",buf[t]);
	}	 

	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束	
}

//检测是否接收到指定的字符串
static u8 Find(const char *a)
{ 
  if(strstr((const char*)USART2_RX_BUF,(const char*)a)!=NULL)
	{
	    return 1;
	}

	else
	{
			return 0;
	}
}

/*******************************************************************************
* 函数名 : CLR_GPRS_Buf
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
static void CLR_GPRS_Buf(void)
{
	u16 k;
	for(k=0;k<USART2_MAX_RECV_LEN;k++)      //将缓存内容清零
	{
		USART2_RX_BUF[k] = 0x00;
	}
		USART2_RX_STA=0;
}

/*******************************************************************************
* 函数名 : send_at_cmd_noack
* 描述   : 发送AT指令函数
* 输入   : 发送数据的指针、发送等待时间(单位：S)
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/

void send_at_cmd_noack(char *s_at_cmd,u16 cmd_len)         
{
	#if 1
	u16 i;
	CLR_GPRS_Buf(); 
	for (i=0; *s_at_cmd!='\0' && i < cmd_len;s_at_cmd++,i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
		USART_SendData(USART2,*s_at_cmd);
	}
	delay_ms(500); 
	return;
	#endif 
}

/*******************************************************************************
* 函数名 : revs
* 描述   : u16字节序转换
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
u16 revs(u16 number)
{
	u32 temp = 0;
	temp = ((number&0xff00) >> 8) + ((number&0x00ff) << 8);
	return temp;
}

///*******************************************************************************
//* 函数名 : revl
//* 描述   : u32字节序转换
//* 输入   : 
//* 输出   : 
//* 返回   : 
//* 注意   : 
//*******************************************************************************/
u32 revl(u32 number)
{
	u32 temp = 0;
	temp = ((number&0x000000ff) << 24) + ((number&0x0000ff00) << 8) + 
	((number&0x00ff0000) >> 8) + ((number&0xff000000) >> 24);
	return temp;
}

/*******************************************************************************
* 函数名 : crc16_check
* 描述   : crc16校验
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
uint16_t crc16_check(uint8_t *Pushdata,uint8_t length)
{
    uint16_t Reg_CRC=0xffff;
    uint8_t i,j;
    for( i = 0; i<length; i ++)
    {
           Reg_CRC^= *Pushdata++;
           for (j = 0; j<8; j++)
           {
            if (Reg_CRC & 0x0001)
                Reg_CRC=Reg_CRC>>1^0xA001;
            else
                Reg_CRC >>=1;
           }
    }
    return Reg_CRC;
}


//***************************************
//将wifi模块搜索到的wifi提取出来并分别放置到各个字符串中
//*****************************************
void wifi_find(void)
{
	u16 i=0;		//串口2缓存，遍历用的变量.
	u16 x=0;		//加号位置存储数组，遍历用的变量
	u16 y=0;
	u16 z=0;
	u16 m=0;
	u16 k=0;
	u16 k1=0,k2 = 0;
	
	//确定+号的位置
	for(i=0;i<sizeof(USART2_RX_BUF);i++)
	{
		if(USART2_RX_BUF[i]=='+')
		{
			position[x]=i;
			printf("i=%d\r\n",i);
			printf("plus_position=%d\r\n",position[x]);
			x++;
		}
	}
	apnb=x;
	printf("WiFi个数：%d\r\n\r\n",apnb);
	POINT_COLOR=BLUE;			//设置字体为蓝色
	Show_Str(20,50,200,16,"搜索到的WiFi数量：",16,0);
	LCD_ShowxNum(170,50,apnb,2,16,0);
	
	//根据+的位置分成不同的区块来进行储存
	for(y=0;y!=x;y++)
	{
		printf("plus_position=%d\r\n",position[y]);
		for(k=position[y];k!=position[y+1];k++)
		{
			if(USART2_RX_BUF[k]=='"')
			{	
				if(k2==0)					//第二个"的位置为0.
				{
					if(k1 == 0)
					{
						k1=k;
						printf("k1=%d\r\n",k1);
					}
					else
					{
						k2 = k;
						printf("k2=%d\r\n",k2);
					}
				}
			else break;
			}
		}
		
		printf("起始：%d\r\n",z);
		for(i=k1+1;i<k2;i++)
		{
			WRITE_WIFI[z]= USART2_RX_BUF[i];
			printf("%c",WRITE_WIFI[z]);
			switch(y)
			{
				case 0:
					ssid1[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 1:
					ssid2[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 2:
					ssid3[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 3:
					ssid4[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 4:
					ssid5[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 5:
					ssid6[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 6:
					ssid7[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 7:
					ssid8[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 8:
					ssid9[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				case 9:
					ssid10[m]=USART2_RX_BUF[i];
					m++;				
				break;
				
				default:break;
			}
			
			z++;
		}
		printf("\r\n");
		
		printf("ssid1:%s\r\n",ssid1);
		printf("ssid2:%s\r\n",ssid2);
		printf("ssid3:%s\r\n",ssid3);
		printf("ssid4:%s\r\n",ssid4);
		printf("ssid5:%s\r\n",ssid5);
		printf("ssid6:%s\r\n",ssid6);
		printf("ssid7:%s\r\n",ssid7);
		printf("ssid8:%s\r\n",ssid8);
		printf("ssid9:%s\r\n",ssid9);
		printf("ssid10:%s\r\n",ssid10);
		
		printf("结束：%d\r\n",z);
		
		z++;
		
		printf("\r\n\r\n");
		
		k1=0;
		k2=0;
		m=0;
	}
}



//加载wifi界面（尺寸为240*140）
//x,y:界面起始坐标（320*240分辨率的时候，x必须为0）
void draw_wifi_frame(u16 x,u16 y)
{
	POINT_COLOR=RED; 
//	LCD_DrawRectangle(x,y   , x+200,y+175);						   
	LCD_DrawRectangle(x,y,    x+200,y+35 );	 
	LCD_DrawRectangle(x,y+35, x+200,y+70 );
	LCD_DrawRectangle(x,y+70, x+200,y+105);	
	LCD_DrawRectangle(x,y+105,x+200,y+140);	
	LCD_DrawRectangle(x,y+140,x+200,y+175);
	
	POINT_COLOR=BLACK;
	LCD_DrawRectangle(x,y+200,    x+90,y+230 );
	LCD_DrawRectangle(x+110,y+200,x+200,y+230);
	
	Show_Str( 40,288,200,16,"上一页",16,0);
	Show_Str(150,288,200,16,"下一页",16,0);
}

void wifi_show(void)
{
	draw_wifi_frame(20,80);
	
	Show_Str(30, 90,200,16,"WiFi1:",16,0);
	Show_Str(30,125,200,16,"WiFi2:",16,0);
	Show_Str(30,160,200,16,"WiFi3:",16,0);
	Show_Str(30,195,200,16,"WiFi4:",16,0);
	Show_Str(30,230,200,16,"WiFi5:",16,0);
	
	Show_Str(80, 90,200,16,ssid1,16,0);
	Show_Str(80,125,200,16,ssid2,16,0);
	Show_Str(80,160,200,16,ssid3,16,0);
	Show_Str(80,195,200,16,ssid4,16,0);
	Show_Str(80,230,200,16,ssid5,16,0);
	
	page1_flag=1;
}


void get_touch_msg(void)
{
		if(TP_Scan(0)&&(page1_flag==1))
		{
			if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>80)&&(tp_dev.y[0]<115))	//WiFi1区域.
			{
				LCD_Clear(WHITE);
				POINT_COLOR=BLACK;
//				Show_Str(20, 120,200,16,"已选择WiFi1:",16,0);
//				Show_Str(120,120,200,16,ssid1,16,0);
//				Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
				
				printf("WiFi1：%s\r\n",ssid1);
				
				STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid1,sizeof(ssid1));	//把ssid1写进flash中.
				delay_ms(2000);
//				STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
				printf("[YZ]已选择WiFi1:%s\r\n",getssid);
				
				wifi_choose_flag=1;
			}
			else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>115)&&(tp_dev.y[0]<150))	//按WiFi2区域
			{
				LCD_Clear(WHITE);
				POINT_COLOR=BLACK;
//				Show_Str(20, 120,200,16,"已选择WiFi2:",16,0);
//				Show_Str(120,120,200,16,ssid2,16,0);
//				Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
				
				printf("WiFi2：%s\r\n",ssid2);
				
				STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid2,sizeof(ssid2));	//把ssid2写进flash中.
				delay_ms(2000);
//				STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
				printf("[YZ]已选择WiFi2:%s\r\n",getssid);
				
				wifi_choose_flag=1;
			}
			else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>150)&&(tp_dev.y[0]<185))	//按WiFi3区域
			{
				LCD_Clear(WHITE);
				POINT_COLOR=BLACK;
//				Show_Str(20, 120,200,16,"已选择WiFi3:",16,0);
//				Show_Str(120,120,200,16,ssid3,16,0);
//				Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
				
				printf("WiFi3：%s\r\n",ssid3);
				
				STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid3,sizeof(ssid3));	//把ssid3写进flash中.
				delay_ms(2000);
//				STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
				printf("[YZ]已选择WiFi3:%s",getssid);
				
				wifi_choose_flag=1;
			}
			else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>185)&&(tp_dev.y[0]<220))	//按WiFi4区域
			{
				LCD_Clear(WHITE);
//				POINT_COLOR=BLACK;
//				Show_Str(20, 120,200,16,"已选择WiFi4:",16,0);
//				Show_Str(120,120,200,16,ssid4,16,0);
//				Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
				
				printf("WiFi4：%s\r\n",ssid4);
				
				STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid4,sizeof(ssid4));	//把ssid4写进flash中.
				delay_ms(2000);
//				STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
				printf("[YZ]已选择WiFi4:%s",getssid);
				
				wifi_choose_flag=1;
			}
			else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>220)&&(tp_dev.y[0]<255))	//按WiFi5区域
			{		
				LCD_Clear(WHITE);
				POINT_COLOR=BLACK;
//				Show_Str(20, 120,200,16,"已选择WiFi5:",16,0);
//				Show_Str(120,120,200,16,ssid5,16,0);
//				Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
				
				printf("WiFi5：%s\r\n",ssid5);
				
				STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid5,sizeof(ssid5));	//把ssid4写进flash中.
				delay_ms(2000);
//				STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
				printf("[YZ]已选择WiFi5:%s",getssid);
				
				wifi_choose_flag=1;
			}
			else if((tp_dev.x[0]>130)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>280)&&(tp_dev.y[0]<310))
			{
				printf("下一页\r\n");
				
				LCD_Clear(WHITE);
				
//				Show_Str_Mid(0,15,"ESP8266 WIFI搜索程序",16,240);
//				POINT_COLOR=BLUE;			//设置字体为蓝色
//				Show_Str(20,50,200,16,"搜索到的WiFi数量：",16,0);
//				LCD_ShowxNum(170,50,apnb,2,16,0);		
				draw_wifi_frame(20,80);
				
				Show_Str(30, 90,200,16,"WiFi6:",16,0);
				Show_Str(30,125,200,16,"WiFi7:",16,0);
				Show_Str(30,160,200,16,"WiFi8:",16,0);
				Show_Str(30,195,200,16,"WiFi9:",16,0);
				Show_Str(30,230,200,16,"WiFi10:",16,0);
				
				Show_Str(80, 90,200,16,ssid6,16,0);
				Show_Str(80,125,200,16,ssid7,16,0);
				Show_Str(80,160,200,16,ssid8,16,0);
				Show_Str(80,195,200,16,ssid9,16,0);
				Show_Str(80,230,200,16,ssid10,16,0);
				
				page1_flag=0;		//页面1标志位初始化.
				page2_flag=1;		//页面2标志位置1.
				
		}
	}
	else if(TP_Scan(0)&&(page2_flag==1))
	{
		if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>80)&&(tp_dev.y[0]<115))	//WiFi6区域.
		{
			LCD_Clear(WHITE);
			POINT_COLOR=BLACK;
//			Show_Str(20, 120,200,16,"已选择WiFi6:",16,0);
//			Show_Str(120,120,200,16,ssid6,16,0);
//			Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
			
			printf("WiFi6：%s\r\n",ssid6);
			
			STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid6,sizeof(ssid6));	//把ssid6写进flash中.
			delay_ms(2000);
//			STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
			printf("[YZ]已选择WiFi6:%s",getssid);
			
			wifi_choose_flag=1;
		}
		else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>115)&&(tp_dev.y[0]<150))	//按WiFi7区域
		{
			LCD_Clear(WHITE);
			POINT_COLOR=BLACK;
//			Show_Str(20, 120,200,16,"已选择WiFi7:",16,0);
//			Show_Str(120,120,200,16,ssid7,16,0);
//			Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
			
			printf("WiFi7：%s\r\n",ssid7);
			
			STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid7,sizeof(ssid7));	//把ssid7写进flash中.
			delay_ms(2000);
//			STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
			printf("[YZ]已选择WiFi7:%s",getssid);
			
			wifi_choose_flag=1;
		}
		else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>150)&&(tp_dev.y[0]<185))	//按WiFi8区域
		{
			
			LCD_Clear(WHITE);
			POINT_COLOR=BLACK;
//			Show_Str(20, 120,200,16,"已选择WiFi8:",16,0);
//			Show_Str(120,120,200,16,ssid8,16,0);
//			Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
			
			printf("WiFi8：%s\r\n",ssid8);
			
			STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid8,sizeof(ssid8));	//把ssid8写进flash中.
			delay_ms(2000);
//			STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
			printf("[YZ]已选择WiFi8:%s",getssid);
			
			wifi_choose_flag=1;
		}
		else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>185)&&(tp_dev.y[0]<220))	//按WiFi9区域
		{
			LCD_Clear(WHITE);
			POINT_COLOR=BLACK;
//			Show_Str(20, 120,200,16,"已选择WiFi9:",16,0);
//			Show_Str(120,120,200,16,ssid9,16,0);
//			Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
			
			printf("WiFi9：%s\r\n",ssid9);
			
			STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid9,sizeof(ssid9));	//把ssid8写进flash中.
			delay_ms(2000);
//			STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
			printf("[YZ]已选择WiFi9:%s",getssid);
			
			wifi_choose_flag=1;
		}
		else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<220)&&(tp_dev.y[0]>220)&&(tp_dev.y[0]<255))	//按WiFi10区域
		{
			LCD_Clear(WHITE);
			POINT_COLOR=BLACK;
//			Show_Str(20, 120,200,16,"已选择WiFi10:",16,0);
//			Show_Str(120,120,200,16,ssid10,16,0);
//			Show_Str(20, 150,200,16,"请输入WiFi密码...",16,0);
//			
			printf("WiFi10：%s\r\n",ssid10);
			
			STMFLASH_Write(FLASH_SAVE_ADDR2,(u16 *)ssid10,sizeof(ssid10));	//把ssid10写进flash中.
			delay_ms(2000);
//			STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)getssid,sizeof(getssid));
			printf("[YZ]已选择WiFi10:%s",getssid);
			
			wifi_choose_flag=1;
		}
		else if((tp_dev.x[0]>20)&&(tp_dev.x[0]<110)&&(tp_dev.y[0]>280)&&(tp_dev.y[0]<310))
		{
			printf("上一页\r\n");
			
			LCD_Clear(WHITE);
			
//			Show_Str_Mid(0,15,"ESP8266 WIFI搜索程序",16,240);
//			POINT_COLOR=BLUE;			//设置字体为蓝色
//			Show_Str(20,50,200,16,"搜索到的WiFi数量：",16,0);
//			LCD_ShowxNum(170,50,apnb,2,16,0);		
			draw_wifi_frame(20,80);
			
			Show_Str(30, 90,200,16,"WiFi1:",16,0);
			Show_Str(30,125,200,16,"WiFi2:",16,0);
			Show_Str(30,160,200,16,"WiFi3:",16,0);
			Show_Str(30,195,200,16,"WiFi4:",16,0);
			Show_Str(30,230,200,16,"WiFi5:",16,0);
			
			Show_Str(80, 90,200,16,ssid1,16,0);
			Show_Str(80,125,200,16,ssid2,16,0);
			Show_Str(80,160,200,16,ssid3,16,0);
			Show_Str(80,195,200,16,ssid4,16,0);
			Show_Str(80,230,200,16,ssid5,16,0);
			
			page1_flag=1;		//页面1标志位置1.
			page2_flag=0;		//页面2标志位初始化.
		}
	}
}


void wifi_LCD_Display(void)
{
//		send_at_cmd_noack("AT+CWMODE=1\r\n", strlen("AT+CWMODE=1\r\n"));
//		send_at_cmd_noack("AT+RST\r\n", strlen("AT+RST\r\n"));
//		delay_ms(3000);
		while(atk_8266_send_cmd("AT+CWMODE=1","OK",200));  	//设置STA模式
		while(atk_8266_send_cmd("AT+RST","",400));
		LCD_Clear(WHITE);
		printf("搜索附近WiFi...\r\n");
		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));					//串口缓存清零.
		USART2_RX_STA=0;
		while(atk_8266_send_cmd("AT+CWLAP","OK",200));  	//搜索AP
//		send_at_cmd_noack("AT+CWLAP\r\n", strlen("AT+CWLAP\r\n"));
//	 delay_ms(50000);
	 printf("wifi如下");
		printf("%s\r\n",USART2_RX_BUF);
		wifi_find();
		wifi_show();
	
	wifi_choose_flag=0;
	
	while(1)
	{
//		get_touch();
			get_touch_msg();
			if(wifi_choose_flag==1)
			{
					wk_up = 0;
					wifi_ok = 0;
					wifi_state = 0;
					wifi_changed = 1;
					break;
			}				
	}
}

//**********************************数据打包****************************************//
//跟服务器的规定有关，对需要发送的数据进行包装，再上传，这样服务器可以对发过来的数据进行识别并储存
//************************************************************************************
void Data_packaging(u8 *buffer, u8 info ,u8 len)
{
		u8 j;
		uint16_t crc = 0;
		sdk_cmd_info.cmd_no = revs(info);///<发送到网络转为大端模式
		sdk_cmd_info.header[0] = 'H';
		sdk_cmd_info.header[1] = 'E';
		sdk_cmd_info.header[2] = 'A';
		sdk_cmd_info.header[3] = 'D';
		for(j=0;j<4;j++)
		sdk_cmd_info.sn[j]=SN[j];
		sdk_cmd_info.second = 0;
		sdk_cmd_info.second=revl(sdk_cmd_info.second);
		sdk_cmd_info.ms = 0;
		sdk_cmd_info.ms=revl(sdk_cmd_info.ms);
		sdk_cmd_info.sys_num1=Get_random_num();
		sdk_cmd_info.sys_num1=revl(sdk_cmd_info.sys_num1);
		sdk_cmd_info.sys_num2=Get_random_num();	
		sdk_cmd_info.sys_num2=revl(sdk_cmd_info.sys_num2);
		sdk_cmd_info.len = revs(sizeof(sdk_cmd_info));
		memcpy(sdk_cmd_info.data,buffer,158);
		crc = crc16_check((uint8_t*)&sdk_cmd_info,sizeof(sdk_cmd_info) -2);
		sdk_cmd_info.crc[0] = crc & 0x00ff;
		sdk_cmd_info.crc[1] = (crc & 0xff00) >> 8;
}
//****************************************
//wifi模块连接wifi以及服务器
//***************************************
void wifi_connect(void)
{
	char wifi_data[200];
	switch(wifi_state)
	{
		case 0:
				send_at_cmd_noack("AT+CWMODE=1\r\n", strlen("AT+CWMODE=1\r\n"));
				printf("%s",USART2_RX_BUF);
				wifi_state = 1;
				i = 0;
		break;
		
		case 1:
				if(i==0)
					{
					send_at_cmd_noack("AT+RST\r\n", strlen("AT+RST\r\n"));
					printf("wifi_reconnect:STA模式重启中\r\n");
					}
					delay_ms(500);
					i++;
				if(i>2)
				{
					if(wifi_changed == 1)//在开机后重新设置过wifi的名称密码
					{
						wifi_state = 2;
					}
					else
					{
						wifi_state = 3;
					}
					i = 0;
				}
		break;
		
		case 2:
			if(i == 0)
			{
				memset(get_wifista_ssid_buf,0,sizeof(get_wifista_ssid_buf));
				memset(get_wifista_pswd_buf,0,sizeof(get_wifista_pswd_buf));
				wifi_changed = 0;
				STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)get_wifista_ssid_buf,sizeof(get_wifista_ssid_buf));	//获取flash中的IP地址到get_server_ipnb.
				STMFLASH_Read(FLASH_SAVE_ADDR3,(u16*)get_wifista_pswd_buf,sizeof(get_wifista_pswd_buf));	//获取flash中的端口号到get_server_port.
				sprintf(wifi_data,"AT+CWJAP=\"%s\",\"%s\"\r\n",get_wifista_ssid_buf,get_wifista_pswd_buf);
				printf("%s",wifi_data);
				UART2_SendString(wifi_data);	
				printf("%s",USART2_RX_BUF);
				memset(wifista_ssid_buf,0,sizeof(wifista_ssid_buf));
				memset(wifista_pswd_buf,0,sizeof(wifista_pswd_buf));
				printf("wifi_connect:连接新wifi中");
			}
			delay_ms(500);
			i++;
			if(i>8)
			{
				printf("连接新wifi=%d",i);
				wifi_state = 3;
				i = 0;
			}
		break;
		
		case 3:
			STMFLASH_Read(FLASH_SAVE_ADDR2,(u16*)get_wifista_ssid_buf,sizeof(get_wifista_ssid_buf));	//获取flash中的IP地址到get_server_ipnb.
			STMFLASH_Read(FLASH_SAVE_ADDR3,(u16*)get_wifista_pswd_buf,sizeof(get_wifista_pswd_buf));	//获取flash中的端口号到get_server_port.
			send_at_cmd_noack("AT+CIPMUX=0\r\n", strlen("AT+CIPMUX=0\r\n"));
			printf("%s",USART2_RX_BUF);	
			wifi_state = 4;
		break;		
		
		case 4:
			send_at_cmd_noack("AT+CIPMODE=0\r\n", strlen("AT+CIPMODE=0\r\n"));
			printf("%s",USART2_RX_BUF);
			wifi_state = 5;
		break;
		
		case 5:
			if(i == 0)
			{
				u8 pbuf[200];
				CLR_GPRS_Buf();
				memset(get_server_ipnb,0,sizeof(get_server_ipnb));
				memset(get_server_port,0,sizeof(get_server_port));
				printf("wifi_connect:连接服务器中\r\n");
				LCD_ShowString(110,200,200,16,16,get_wifista_ssid_buf);
				LCD_ShowString(110,220,200,16,16,get_wifista_pswd_buf); 
				STMFLASH_Read(FLASH_SAVE_ADDR0,(u16*)get_server_ipnb,sizeof(get_server_ipnb));	//获取flash中的IP地址到get_server_ipnb.
				STMFLASH_Read(FLASH_SAVE_ADDR1,(u16*)get_server_port,sizeof(get_server_port));	//获取flash中的端口号到get_server_port.
				printf("当前输入的服务器IP:%s,当前输入的服务器端口：%s\r\n",get_server_ipnb,get_server_port);
				sprintf((char*)pbuf,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",get_server_ipnb,get_server_port);
				UART2_SendString((char*)pbuf);
				printf("%s",USART2_RX_BUF);
				memset(wifista_ipnb_buf,0,sizeof(wifista_ipnb_buf));
				memset(wifista_port_buf,0,sizeof(wifista_port_buf));
			}
			delay_ms(500);
			i++;
			if(i>8)
			{
				LCD_ShowString(110,240,200,16,16,get_server_ipnb);
				LCD_ShowString(110,260,200,16,16,get_server_port);	
				wifi_state = 0;
				wifi_ok = 1;
				i=0;
			}
		break;
			
	}
}
//**********************************
//通过WiFi和服务器建立TCP连接，并想服务器上传数据
//**********************************
void wifi_send(void)
{
	char str[100];
	u8 get_data[158];
	CLR_GPRS_Buf();
	printf("wifi_send:发送数据中\r\n");
	for(i=6;i<158;i++)   //这些位存储的都应该为0，但实际上不是，因此再次重置一下
	{
		get_data[i] = 0x00;
	}
	get_data[0] = 0x00;
	get_data[1] = average1;
	get_data[2] = 0x00;
	get_data[3] = average2;
	get_data[4] = 0x00;
	get_data[5] = average3;
	Data_packaging(get_data,3,25);
	sprintf(str,"AT+CIPSEND=%d\r\n",sizeof(sdk_cmd_info));
	printf("%s\r\n",str);
	printf("%s\r\n",USART2_RX_BUF);

if(1) //不这样会有重复发送两次的bug
	{
		CLR_GPRS_Buf();
		UART2_SendString(str);
		delay_ms(4000);
		printf("%s\r\n",USART2_RX_BUF);
		if(Find("ERROR"))
		{
			printf("connect failed");
			wifi_ok = 0;
		}
		else
		{
			printf("%s\r\n",USART2_RX_BUF);
			CLR_GPRS_Buf(); 
			Send_DATA ((u8 *)&sdk_cmd_info ,sizeof(sdk_cmd_info));
			delay_ms(6000);
			printf("发送数据\r\n");
			CLR_GPRS_Buf(); 
			sending = 0;
		}
	}
}

//**********************************
//wifi模块接收手机端发送的数据  wifi名称（mode:0,不清零USART2_RX_STA;1,清零USART2_RX_STA）
//***********************************
void esp8266_ssid_sava(u8 mode)
{
	u8 *p1; 
	u16 i=0;
	memset(wifista_ssid_buf,0,sizeof(wifista_ssid_buf));
	if((USART2_RX_STA&0X8000)&&sizeof(USART2_RX_BUF)>4)		//接收到一次数据了
	{ 
		for(p1=&USART2_RX_BUF[0];*p1!=':';p1++);
		p1++;	
			
		while(*p1!='\0')
		{
			wifista_ssid_buf[i]=*p1;
			i++;
			p1++;
		}		
		STMFLASH_Write(FLASH_SAVE_ADDR2,(u16*)wifista_ssid_buf,sizeof(wifista_ssid_buf));	//把wifista_ipnb_buf中的IP地址写进flash中.
		printf("输入的路由器SSID为：%s\r\n\r\n",wifista_ssid_buf);	//发送到串口			
		if(mode)USART2_RX_STA=0;	
		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));					//串口缓存清零.
		delay_ms(100);		
		LCD_Fill(110,200,310,216,WHITE);
		delay_ms(300);
		LCD_Fill(30,280,230,296,WHITE);
		LCD_ShowString(110,200,200,16,16,wifista_ssid_buf);
		ssid_sended_flag=1;

	} 
} 
	
//**********************************
//wifi模块接收手机端发送的数据  wifi密码（mode:0,不清零USART2_RX_STA;1,清零USART2_RX_STA）
//***********************************
void esp8266_pwd_sava(u8 mode)
{
		u8 *p1;
	u16 i=0;
	
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		
		for(p1=&USART2_RX_BUF[0];*p1!=':';p1++);
		p1++;
		
		while(*p1!='\0')
		{
			wifista_pswd_buf[i]=*p1;
			i++;
			p1++;
		}		
		STMFLASH_Write(FLASH_SAVE_ADDR3,(u16*)wifista_pswd_buf,sizeof(wifista_pswd_buf));	//把wifista_ipnb_buf中的IP地址写进flash中.
		printf("输入的路由器Password为：%s\r\n\r\n",wifista_pswd_buf);	//发送到串口			
		if(mode)USART2_RX_STA=0;	
		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));			//串口缓存清零.
		delay_ms(100);
		LCD_Fill(110,220,310,236,WHITE);
		delay_ms(300);
		LCD_Fill(30,280,230,296,WHITE);
		LCD_ShowString(110,220,200,16,16,wifista_pswd_buf); 
		pswd_sended_flag=1;
	} 
}
//**********************************
//wifi模块接收手机端发送的数据  服务器ip（mode:0,不清零USART2_RX_STA;1,清零USART2_RX_STA）
//***********************************
void esp8266_ipnb_sava(u8 mode)
{
	u8 *p1;
	u16 i=0;
	
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		
		for(p1=&USART2_RX_BUF[0];*p1!=':';p1++);
		p1++;
		
		while(*p1!='\0')
		{
			wifista_ipnb_buf[i]=*p1;
			i++;
			p1++;
		}	
		
		STMFLASH_Write(FLASH_SAVE_ADDR0,(u16*)wifista_ipnb_buf,sizeof(wifista_ipnb_buf));	//把wifista_ipnb_buf中的IP地址写进flash中.
		printf("输入的服务器IP地址为：%s\r\n\r\n",wifista_ipnb_buf);	//发送到串口			
		if(mode)USART2_RX_STA=0;
		
		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));								//串口缓存清零.
		USART2_RX_STA=0;
		delay_ms(100);
		LCD_Fill(110,240,310,256,WHITE);
		delay_ms(300);
		LCD_Fill(30,280,230,296,WHITE);
		LCD_ShowString(110,240,200,16,16,wifista_ipnb_buf);
		ipnb_sended_flag=1;
	} 
}

//**********************************
//wifi模块接收手机端发送的数据  服务器端口号（mode:0,不清零USART2_RX_STA;1,清零USART2_RX_STA）
//***********************************
void esp8266_port_sava(u8 mode)
{
	u8 *p1;
	u16 i=0;
	
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		for(p1=&USART2_RX_BUF[0];*p1!=':';p1++);
		p1++;
		
		while(*p1!='\0')
		{
			wifista_port_buf[i]=*p1;
			i++;
			p1++;
		}
		
		STMFLASH_Write(FLASH_SAVE_ADDR1,(u16*)wifista_port_buf,sizeof(wifista_port_buf));//把wifista_port_buf中的端口号写进flash中.
		printf("输入的服务器端口号为：%s\r\n\r\n",wifista_port_buf);	//发送到串口.			
		if(mode)USART2_RX_STA=0;	
		
		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));								//串口缓存清零.
		USART2_RX_STA=0;
		delay_ms(100);
		LCD_Fill(110,260,310,276,WHITE);
		delay_ms(300);
		LCD_Fill(30,280,230,296,WHITE);
		LCD_ShowString(110,260,200,16,16,wifista_port_buf);	
		port_sended_flag=1;
	} 
}

//*************************************
//根据接收到的字符串，跳转到需要具体更改的模式
//************************************
void GETnewWIFI(void)
{
	while(1)
	{
		delay_ms(300);
		LCD_Fill(30,280,230,296,WHITE);
		if(Find("wifi"))
		{	
			delay_ms(100);
			CLR_GPRS_Buf();	
			printf("请发送路由器的SSID...\r\n");
			while(1)
			{
				LCD_ShowString(30,280,200,16,16,"wifi setting");	
				delay_ms(300);
				LCD_Fill(30,280,230,296,WHITE);
				delay_ms(300);
				esp8266_ssid_sava(1);
				if(ssid_sended_flag==1)break;
			}	
		}
		else if(Find("pswd"))
			{
				delay_ms(100);
				CLR_GPRS_Buf();	
					
				printf("请发送路由器的Password...\r\n");
				while(1)
				{
					LCD_ShowString(30,280,200,16,16,"password setting");	
					delay_ms(300);
					LCD_Fill(30,280,230,296,WHITE);
					delay_ms(300);
					esp8266_pwd_sava(1);
					if(pswd_sended_flag==1)break;
				}
			}
	
		else	if(wk_up == 1&&Find("ipad"))
				{
					delay_ms(100);
					CLR_GPRS_Buf();	
						
					printf("请发送服务器的IP地址...\r\n");
					while(1)
					{
						LCD_ShowString(30,280,200,16,16,"ip addr setting");	
						delay_ms(300);
						LCD_Fill(30,280,230,296,WHITE);
						delay_ms(300);
						esp8266_ipnb_sava(1);
						if(ipnb_sended_flag==1)break;
					}
				}
			
		else if(wk_up==1&&Find("port"))
					{
						delay_ms(100);
						CLR_GPRS_Buf();	
						printf("请发送服务器的端口号...\r\n");
						while(1)
						{
							LCD_ShowString(30,280,200,16,16,"ip port setting");
							delay_ms(300);
							LCD_Fill(30,280,230,296,WHITE);
							delay_ms(300);
							esp8266_port_sava(1);
							if(port_sended_flag==1)break;
						}
					}
		else if(Find("over")) break;
		else if(Find("CLOSED"))	break;		
		else 
		{
				CLR_GPRS_Buf();	
				delay_ms(300);
				LCD_ShowString(30,280,200,16,16,"select one to set");
				printf("请输入正确指令");

		}
	}
	
	ssid_sended_flag = 0;
	pswd_sended_flag = 0;
	ipnb_sended_flag = 0;
	port_sended_flag = 0;
	wk_up = 0;
	wifi_ok = 0;
	wifi_state = 0;
	wifi_changed = 1;

}

/*************************************
		更新wifi设置，转换到AP模式并设置wifi模块成为客户端
***************************************/
void wifi_change(void)
{
	char rst[]="AT+RST\r\n";
	char port[]="AT+CIPSERVER=1,8080\r\n";
	char espwifi[]="AT+CWSAP=\"JiuHengJianKang\",\"123456123456\",1,3\r\n";
	send_at_cmd_noack("AT+CWMODE=2\r\n", strlen("AT+CWMODE=2\r\n"));
	printf("wifi_change:转换AP模式\r\n");
	delay_ms(200);
	UART2_SendString(rst);
	printf("wifi_change:重启中\r\n");
	delay_ms(3000);
	send_at_cmd_noack("AT+CIPMUX=1\r\n", strlen("AT+CIPMUX=1\r\n"));
		printf("wifi_change:开启多链絓r\n");
	delay_ms(200);
	UART2_SendString(espwifi);
		printf("wifi_change:开启wifi\r\n");
	delay_ms(100);
	UART2_SendString(port);
		printf("wifi_change:设置服务端\r\n");
	delay_ms(100);
	CLR_GPRS_Buf(); 
		printf("请连接WIFI模块的SSID和IP...\r\n");
	while(esp8266_connect_flag==0) //等待区，等待手机端的连接，器件可长按key0退出此模式
	{
		LED0 = !LED0;
		delay_ms(300);
		if(KEY1 == 0)
		{
			i = 0;
			while(1)
			{
				if(KEY1 == 0)
				{
					LED0 = 0;
					delay_ms(100);
					i++;
					printf("%d",i);
					if(i>10)
					{
						out_set = 1;
						LED0 = 1;
						while(1)
						{
							if(KEY1 == 1)
							{
								delay_ms(200);
								LED0 = !LED0;
								delay_ms(200);
								LED0 = !LED0;
								delay_ms(200);
								break;
							}
						}
					}
				}
				else 
				{
					LED1 = 0;
					break;
				}
			}
		}

		if(out_set == 1)
		{
			break;
		}
		else if(USART2_RX_STA&0X8000)		//接收到数据
			{
				printf("%s\r\n",USART2_RX_BUF);
				CLR_GPRS_Buf(); 
				break;
			}
	}
	if(out_set == 0)
	{
		GETnewWIFI();
	}
	else
		out_set = 0;
	
}
//****************************
//模式选择
//WK_UP长按：进入wifi设置模式，可设置wifi名称，wifi密码，服务器ip，服务器端口，都需要手动输入
//key0长按：进入wifi选择，lcd屏幕上显示
//key1长按：进入设置模式是，长按可退出
//***************************
void mode_set(void)
{	
		int i=0;
		if(WK_UP==1)  
		{
			i = 0;
			while(1)     
			{
				if(WK_UP==1)
				{
					i++;
					delay_ms(50);
					LED0 = !LED0;
					if (i>30)
					{
							LED0 = 1;
							wk_up = 1;
							wifi_change();
					}
				}
				else 
				{
					LED0 = 0;
					break;
				}
			}
				
		}

    if(KEY0==0)
		{
			while(1)
			{
				if(KEY0==0)
				{
					delay_ms(300);
					i++;
					LED0 = !LED0;
					printf("%d\r\n",i);
					if(i>5)
					{
						i = 0;
						LED0 = 1;
						wifi_lcd = 1;
						wifi_LCD_Display();

					}
				}
				else 
				{
						LED0 = 0;
						break;
				}
			}
		}
}
