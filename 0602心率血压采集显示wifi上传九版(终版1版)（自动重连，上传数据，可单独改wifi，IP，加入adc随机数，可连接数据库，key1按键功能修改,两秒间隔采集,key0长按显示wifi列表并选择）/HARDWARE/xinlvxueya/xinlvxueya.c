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
int xueya[10][3]={0};//Ѫѹ��������
int FD[6]={0xfd,0x00,0x00,0x00,0x00,0x00};//fd 00 00 00 00 00 00��ʼ��������
u8 get_xinlv(void)//get���ʺ��� ��00��ʼ����
	{
		u8 start=0;
		u8 t;
		u8 i;
		for(i=0;i<6;i++)
		{   
			USART_SendData(UART5,FD[i]);//�Դ���3��������FD��i������������ָ��
			while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET);//��ѯ�������Ƿ������
		}
		delay_ms(100);
		printf("receive data:%s\r\n",UART5_RX_BUF);
		if(UART5_RX_STA&0x8000)//���ձ�־λ����������ɣ����Ѿ����յ�һ��FD 00 00 00 00 00��
			{   
				//printf("\n�ӵ�һ������---------\n");//0x8000����������Ϊ8*16^3/2=4*4^6=2^14��15λ������Ϊһ��˵�����ݽ��ճɹ���
//				len=UART5_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
//		     	printf("number��ֵ��%d\n",number);//��ʼnumberֵΪ0
				for(t=1;t<4;t++)
				{
					if(t==1)//t==1 �����ѹ
						{
							if(0==UART5_RX_BUF[t])//δ��������,BUF[t]����t==1/2/3���ζ�Ӧ��ѹ��ѹ���ʶ���t==1ʱҪ�ж��Ƿ������� ��������
								{
									printf("����ing\r\n");
									LCD_ShowString(30,170,200,16,16,"CONNECTING...");
									average1=0;
									average2=0;
									average3=0;
									start=0;
									break;
								}
							else if(255==UART5_RX_BUF[t])//���ӳ�ʱ
								{
									start=0,ct=0;
									printf("����ing\r\n");
									LCD_Fill(30,170,230,186,WHITE);
									memset(xueya,0,sizeof(xueya));//���ֺ��������
									average1=0;
									average2=0;
									average3=0;
									break;
									}
							else //�����ϣ������ѹ
								{
									start=1;//������
									
									xueya[number][0]=UART5_RX_BUF[t];
//									printf("��ʱ��ѹ��%d\n",xueya[number][0]);//��������buf�������һ�����ݸ�Ѫѹnumber0
									average1=0;
								}
						}
					else if(t==2)//t==2�����ѹ
						{
							xueya[number][1]=UART5_RX_BUF[t];
//							printf("��ʱ��ѹ��%d\n",xueya[number][1]);//��������buf������ڶ������ݸ�Ѫѹnumber1
							average2=0;
						}
						
					else if(t==3)//t==3�������
						{
							xueya[number][2]=UART5_RX_BUF[t];
							printf("��ʱ���ʣ�%d\n",xueya[number][2]);//��������buf��������������ݸ�Ѫѹnumber2
							average3=0;
							number++;//��һ������
						}
				}
				if(start==1)//��ǰ����״̬�£�
					{
						LCD_Fill(30,170,230,186,WHITE);
						if(number>9)//��20�����¼���
								{
									ct=1;
									number=0;
									printf("CT=%d,number=%d\r\n",ct,number);
								}
						if(number<=9&&ct==0)//���������������20����ct==0�������number����20��ct����1��
							{
								for(i=0;i<number;i++)
								{
									average1+=xueya[i][0];
									//printf("��ѹ��ѹ������ͣ�%d\n",average1);
									average2+=xueya[i][1];
									average3+=xueya[i][2];//��ѹ ��ѹ ���� �ֱ����

								}
								average1=(average1/(number*1.03));
//								printf("NUMBER��%d\n",number);//���ݸ���
//								printf("����20�����ݵĸ�ѹ��%d\n",average1);
								average2=(average2/(number*1.03));
//								printf("��ѹ��%d\n",average2);
								average3=(average3/(number*1.03));//У׼�㷨��������ʱֵ
//								printf("���ʣ�%d\r\n",average3);
//								if(number>9)
									printf("����20�����ݵĸ�ѹ��%d\n,��ѹ��%d\n,���ʣ�%d\r\n",average1,average2,average3);
							}
							else if(number<=9&&ct==1)//���������������20����ct==1���Ѿ��й�����һ��number������20��
								{
									for(i=0;i<10;i++)
									{
										average1+=xueya[i][0];
										average2+=xueya[i][1];
										average3+=xueya[i][2];
									}
									average1=(average1/10.45);
//									printf("��ѹ��%d\n",average1);
									average2=(average2/10.45);
//									printf("��ѹ��%d\n",average2);
									average3=(average3/10.45);
//									printf("���ʣ�%d\r\n",average3);		
									printf("��ѹ��%d\n,��ѹ��%d\n,���ʣ�%d\r\n",average1,average2,average3);
								}
								if(average3 == 0)
								{
									average1 = average2 = average3 = 0;
								}
					}
				else
					{
						number=0,start=0,ct=0;
						memset(xueya,0,sizeof(xueya));//���Ѫѹ��������

					}
				UART5_RX_STA=0;//��־λ����
				sending = 1;
			}	
			delay_ms(100);
			return number;	//�������ض�����		
	}
