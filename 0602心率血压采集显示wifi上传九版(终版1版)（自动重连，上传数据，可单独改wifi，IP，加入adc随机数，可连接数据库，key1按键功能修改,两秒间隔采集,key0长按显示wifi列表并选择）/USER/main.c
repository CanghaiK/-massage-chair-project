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
extern const u8 gImage_image1[];//图片数据(包含信息头),存储在image1.c里面.
 int main(void)
 {		
	u16 x=0,y=0;
	u8 key,fontok=0; 
	HEADCOLOR *imginfo;

	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
	usart2_init(115200);
	uart5_init(115200);  //初始化串口2波特率为115200
	KEY_Init(); 
	LCD_Init();	 
	Adc_Init();
	LED_Init();
  
	tp_dev.init();			//触摸屏初始化
 	mem_init();				//初始化内存池	    
 	exfuns_init();			//为fatfs相关变量申请内存  
  f_mount(fs[0],"0:",1); 	//挂载SD卡  
	f_mount(fs[1],"1:",1); 	//挂载FLASH.
	key=KEY_Scan(0);  
	if(key==KEY0_PRES)		//强制校准
	{
		LCD_Clear(WHITE);	//清屏
		tp_dev.adjust();  	//屏幕校准  
		LCD_Clear(WHITE);	//清屏
	}
	fontok=font_init();		//检查字库是否OK
	if(fontok||key==KEY1_PRES)//需要更新字库（字库不存在/KEY1按下）			 
	{
		LCD_Clear(WHITE);		   	//清屏
 		POINT_COLOR=RED;			//设置字体为红色	   	   	  
		LCD_ShowString(60,50,200,16,16,"ALIENTEK STM32");
		while(SD_Initialize())		//检测SD卡
		{
			LCD_ShowString(60,70,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(60,70,200+60,70+16,WHITE);
			delay_ms(200);		    
		}								 						    
		LCD_ShowString(60,70,200,16,16,"SD Card OK");
		LCD_ShowString(60,90,200,16,16,"Font Updating...");
		key=update_font(20,110,16);//更新字库
		while(key)//更新失败		
		{			 		  
			LCD_ShowString(60,110,200,16,16,"Font Update Failed!");
			delay_ms(200);
			LCD_Fill(20,110,200+20,110+16,WHITE);
			delay_ms(200);		       
		} 		  
		LCD_ShowString(60,110,200,16,16,"Font Update Success!");
		delay_ms(1500);	
		LCD_Clear(WHITE);//清屏	       
	}  
	 
	
	delay_ms(1500);		//等待1.5秒		 
	POINT_COLOR=BLACK;
	srand(imginfo->h*imginfo->w);
	imginfo=(HEADCOLOR*)gImage_image1;	//得到文件信息
	image_display(x,y,(u8*)gImage_image1);//在指定地址显示图片
		
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
			delay_ms(1500);		//等待1.5秒		 
			POINT_COLOR=BLACK;
			srand(imginfo->h*imginfo->w);
			imginfo=(HEADCOLOR*)gImage_image1;	//得到文件信息
			image_display(x,y,(u8*)gImage_image1);//在指定地址显示图片
				
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
		TIM_SetCounter(TIM3,0);//计数器清空
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
