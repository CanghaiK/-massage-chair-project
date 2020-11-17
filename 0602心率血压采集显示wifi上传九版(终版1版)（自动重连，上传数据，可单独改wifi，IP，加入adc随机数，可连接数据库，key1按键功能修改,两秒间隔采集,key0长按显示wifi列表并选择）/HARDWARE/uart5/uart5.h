#ifndef __UART5_H
#define __UART5_H	 
#include "sys.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//����5��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

#define UART5_MAX_RECV_LEN		400					//�����ջ����ֽ���
#define UART5_MAX_SEND_LEN		400					//����ͻ����ֽ���
#define UART5_RX_EN 			1					//0,������;1,����.

extern u8  UART5_RX_BUF[UART5_MAX_RECV_LEN]; 		//���ջ���,���UART5_MAX_RECV_LEN�ֽ�
extern u8  UART5_TX_BUF[UART5_MAX_SEND_LEN]; 		//���ͻ���,���UART5_MAX_SEND_LEN�ֽ�
extern u16 UART5_RX_STA;   						//��������״̬
extern u16 Get_data;


void uart5_init(u32 bound);				//����5��ʼ�� 
void u5_printf(char* fmt,...);
void TIM5_Set(u8 sta);
void TIM5_Init(u16 arr,u16 psc);
void TIM3_Init(u16 arr,u16 psc);
//void UART3_DMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar);
//void UART3_DMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u8 len);
#endif













