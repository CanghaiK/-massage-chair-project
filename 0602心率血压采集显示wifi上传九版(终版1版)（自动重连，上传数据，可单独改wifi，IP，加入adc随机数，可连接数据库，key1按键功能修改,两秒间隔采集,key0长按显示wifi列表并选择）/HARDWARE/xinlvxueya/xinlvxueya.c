#include "xinlvxueya.h"
#include "uart5.h"
#include "wifi.h"
#include "string.h"
//#include "usart3.h"
int average1=0;
int average2=0;
int average3=0;

int high_pressure = 0;
int low_pressure = 0;
int heart_rate = 0;

u8 ct=0;
int number=0;
//u8 ci=0;
int xueya[10][3]={0};//血压数组数组
int FD[6]={0xfd,0x00,0x00,0x00,0x00,0x00};//fd 00 00 00 00 00 00初始发送数据
u8 get_xinlv(void)//get心率函数 从00开始覆盖
	{
		u8 start=0;
		u8 t;
		u8 i;
		for(i=0;i<6;i++)
		{   
			USART_SendData(UART5,FD[i]);//对串口3发送数据FD【i】串口三接收指令
			while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET);//查询串口三是否发送完成
		}
		delay_ms(100);
		printf("receive data:%s\r\n",UART5_RX_BUF);
		if(UART5_RX_STA&0x8000)//接收标志位接收数据完成（即已经接收到一次FD 00 00 00 00 00）
			{   
				//printf("\n接到一次数据---------\n");//0x8000二进制数据为8*16^3/2=4*4^6=2^14即15位，相与为一才说明数据接收成功，
//				len=UART5_RX_STA&0x3fff;//得到此次接收到的数据长度
//		     	printf("number的值：%d\n",number);//初始number值为0
				for(t=1;t<4;t++)
				{
					if(t==1)//t==1 输出高压
						{
							if(0==UART5_RX_BUF[t])//未开启连接,BUF[t]随着t==1/2/3依次对应高压低压心率而在t==1时要判断是否在连接 或者脱手
								{
									printf("连接ing\r\n");
									LCD_ShowString(30,170,200,16,16,"CONNECTING...");
									average1=0;
									average2=0;
									average3=0;
									start=0;
									break;
								}
							else if(255==UART5_RX_BUF[t])//连接超时
								{
									start=0,ct=0;
									printf("调整ing\r\n");
									LCD_Fill(30,170,230,186,WHITE);
									memset(xueya,0,sizeof(xueya));//脱手后清除数据
									average1=0;
									average2=0;
									average3=0;
									break;
									}
							else //连接上，输出高压
								{
									start=1;//连接上
									
									xueya[number][0]=UART5_RX_BUF[t];
//									printf("即时高压：%d\n",xueya[number][0]);//将串口三buf数组里第一个数据给血压number0
									average1=0;
								}
						}
					else if(t==2)//t==2输出低压
						{
							xueya[number][1]=UART5_RX_BUF[t];
//							printf("即时低压：%d\n",xueya[number][1]);//将串口三buf数组里第而个数据给血压number1
							average2=0;
						}
						
					else if(t==3)//t==3输出心率
						{
							xueya[number][2]=UART5_RX_BUF[t];
							printf("即时心率：%d\n",xueya[number][2]);//将串口三buf数组里第三个数据给血压number2
							average3=0;
							number++;//下一组数据
						}
				}
				if(start==1)//当前连接状态下，
					{
						LCD_Fill(30,170,230,186,WHITE);
						if(number>9)//到20即重新计数
								{
									ct=1;
									number=0;
									printf("CT=%d,number=%d\r\n",ct,number);
								}
						if(number<=9&&ct==0)//如果数据数量少于20并且ct==0（最初的number超过20，ct即置1）
							{
								for(i=0;i<number;i++)
								{
									average1+=xueya[i][0];
									//printf("高压低压心率求和：%d\n",average1);
									average2+=xueya[i][1];
									average3+=xueya[i][2];//高压 低压 心率 分别求和

								}
								average1=(average1/(number*1.03));
//								printf("NUMBER：%d\n",number);//数据个数
//								printf("少于20个数据的高压：%d\n",average1);
								average2=(average2/(number*1.03));
//								printf("低压：%d\n",average2);
								average3=(average3/(number*1.03));//校准算法，贴近即时值
//								printf("心率：%d\r\n",average3);
//								if(number>9)
									printf("少于20个数据的高压：%d\n,低压：%d\n,心率：%d\r\n",average1,average2,average3);
							}
							else if(number<=9&&ct==1)//如果数据数量少于20并且ct==1（已经有过最少一次number计数到20）
								{
									for(i=0;i<10;i++)
									{
										average1+=xueya[i][0];
										average2+=xueya[i][1];
										average3+=xueya[i][2];
									}
									average1=(average1/10.45);
//									printf("高压：%d\n",average1);
									average2=(average2/10.45);
//									printf("低压：%d\n",average2);
									average3=(average3/10.45);
//									printf("心率：%d\r\n",average3);		
									printf("高压：%d\n,低压：%d\n,心率：%d\r\n",average1,average2,average3);
								}
								if(average3 == 0)
								{
									average1 = average2 = average3 = 0;
								}
					}
				else
					{
						number=0,start=0,ct=0;
						memset(xueya,0,sizeof(xueya));//清除血压数组数据

					}
				UART5_RX_STA=0;//标志位清零
				sending = 1;
			}	
			delay_ms(100);
			return number;	//返不返回都可以		
	}
