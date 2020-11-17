#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "uart5.h"
#include "key.h"
#include "xinlvxueya.h"
#include "image2lcd.h"
#include "lcd.h"
#include "wifi.h"
#include "led.h"
#include "adc.h"

#include "touch.h"
#include "ff.h"  
#include "exfuns.h"
extern const u8 gImage_image1[];//ͼƬ����(������Ϣͷ),�洢��image1.c����.
 int main(void)
 {		
	u16 x=0,y=0;
	u8 key,fontok=0; 
	HEADCOLOR *imginfo;

	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	usart2_init(115200);
	uart5_init(115200);  //��ʼ������2������Ϊ115200
	KEY_Init(); 
	LCD_Init();	 
	Adc_Init();
	LED_Init();
  
	tp_dev.init();			//��������ʼ��
 	mem_init();				//��ʼ���ڴ��	    
 	exfuns_init();			//Ϊfatfs��ر��������ڴ�  
  f_mount(fs[0],"0:",1); 	//����SD��  
	f_mount(fs[1],"1:",1); 	//����FLASH.
	key=KEY_Scan(0);  
	if(key==KEY0_PRES)		//ǿ��У׼
	{
		LCD_Clear(WHITE);	//����
		tp_dev.adjust();  	//��ĻУ׼  
		LCD_Clear(WHITE);	//����
	}
	fontok=font_init();		//����ֿ��Ƿ�OK
	if(fontok||key==KEY1_PRES)//��Ҫ�����ֿ⣨�ֿⲻ����/KEY1���£�			 
	{
		LCD_Clear(WHITE);		   	//����
 		POINT_COLOR=RED;			//��������Ϊ��ɫ	   	   	  
		LCD_ShowString(60,50,200,16,16,"ALIENTEK STM32");
		while(SD_Initialize())		//���SD��
		{
			LCD_ShowString(60,70,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(60,70,200+60,70+16,WHITE);
			delay_ms(200);		    
		}								 						    
		LCD_ShowString(60,70,200,16,16,"SD Card OK");
		LCD_ShowString(60,90,200,16,16,"Font Updating...");
		key=update_font(20,110,16);//�����ֿ�
		while(key)//����ʧ��		
		{			 		  
			LCD_ShowString(60,110,200,16,16,"Font Update Failed!");
			delay_ms(200);
			LCD_Fill(20,110,200+20,110+16,WHITE);
			delay_ms(200);		       
		} 		  
		LCD_ShowString(60,110,200,16,16,"Font Update Success!");
		delay_ms(1500);	
		LCD_Clear(WHITE);//����	       
	}  
	 
	
	delay_ms(1500);		//�ȴ�1.5��		 
	POINT_COLOR=BLACK;
	srand(imginfo->h*imginfo->w);
	imginfo=(HEADCOLOR*)gImage_image1;	//�õ��ļ���Ϣ
	image_display(x,y,(u8*)gImage_image1);//��ָ����ַ��ʾͼƬ
		
	LCD_ShowString(30,100,200,16,16,"high_pressure:");
	LCD_ShowString(30,120,200,16,16,"low_pressure:");    					 
	LCD_ShowString(30,140,200,16,16,"heart_rate:");
		
	LCD_ShowString(30,200,200,16,16,"WIFI:");
	LCD_ShowString(30,220,200,16,16,"password:");    					 
	LCD_ShowString(30,240,200,16,16,"ip addr:");
	LCD_ShowString(30,260,200,16,16,"ip port");
	LCD_ShowString(150,300,200,16,16,"@CJLU_NYL");
	while(1)
	{
		if(wifi_lcd == 1)
		{
			delay_ms(1500);		//�ȴ�1.5��		 
			POINT_COLOR=BLACK;
			srand(imginfo->h*imginfo->w);
			imginfo=(HEADCOLOR*)gImage_image1;	//�õ��ļ���Ϣ
			image_display(x,y,(u8*)gImage_image1);//��ָ����ַ��ʾͼƬ
				
			LCD_ShowString(30,100,200,16,16,"high_pressure:");
			LCD_ShowString(30,120,200,16,16,"low_pressure:");    					 
			LCD_ShowString(30,140,200,16,16,"heart_rate:");
				
			LCD_ShowString(30,200,200,16,16,"WIFI:");
			LCD_ShowString(30,220,200,16,16,"password:");    					 
			LCD_ShowString(30,240,200,16,16,"ip addr:");
			LCD_ShowString(30,260,200,16,16,"ip port");
			LCD_ShowString(150,300,200,16,16,"@CJLU_NYL");
			LCD_ShowString(110,200,200,16,16,get_wifista_ssid_buf);
			wifi_change();
			wifi_lcd = 0;
		}
		LCD_ShowxNum(170,100,average1,4,16,0);
		LCD_ShowxNum(170,120,average2,4,16,0);
		LCD_ShowxNum(170,140,average3,4,16,0); 
		LED0 = 0;
		mode_set();
		if(Get_data == 1)
		{
			printf("dfddddddddddd");
		TIM_SetCounter(TIM3,0);//���������
		get_xinlv();
		}
		if(wifi_ok == 0)
		{
			wifi_connect();
		}
		if(Get_data ==1&&wifi_ok == 1&& average1 != 0)
		{
			wifi_send();   					 
		}
		Get_data = 0;
		delay_ms(500);
	}
 }
