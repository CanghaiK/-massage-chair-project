#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "uart5.h"
#include "xinlvxueya.h"
#include "lcd.h"
#include "time.h"




#define USART2_MAX_RECV_LEN		1024				//最大接收缓存字节数
#define USART2_MAX_SEND_LEN		1024				//最大发送缓存字节数
#define USART2_RX_EN 			1					//0,不接收;1,接收.
#define CMD_HEAD "HEAD"
#define CMD_INFO_LEN sizeof(SDK_CMD_INFO)

typedef struct
{
	u8 header[4];///数据头 48 45 41 44    "HEAD"assic码
	u16 len;///<数据长度 包括头
	u16 cmd_no;///<命令号
	u8 sn[4];  ///设备编号
	u32 second;
	u32 ms;
	u32 sys_num1;
	u32 sys_num2;
	u8 data[158];///<命令附加的数据
	u8 crc[2];///<crc16校验位  crc[0]低字节 crc[1] 高字节
}SDK_CMD_INFO;

extern SDK_CMD_INFO sdk_cmd_info;
extern int wifi_ok;
extern int wifi_changed;
extern int wifi_reload;
extern u8 get_wifista_ssid_buf[100]; 				//ssid缓冲.
extern u8 get_wifista_pswd_buf[100];					//pswd缓存.
extern u8 get_server_ipnb[100];		//用来存储flash中读出来的IP地址.
extern u8 get_server_port[50];			//用来存储flash中读出来的端口号.
extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//接收缓冲,最大USART2_MAX_RECV_LEN字节
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//发送缓冲,最大USART2_MAX_SEND_LEN字节
extern u16 USART2_RX_STA;   						//接收数据状态
extern u16 wifi_state;
extern u16 sending;
extern u16 wifi_lcd;

void u2_printf(char* fmt,...);
void TIM4_Set(u8 sta);
void TIM4_Init(u16 arr,u16 psc);
void usart2_init(u32 bound);
void USART2_IRQHandler(void);
void UART2_SendString(char* s);
void wifi_change(void);
void wifi_LCD_Display(void);
static u8 Find(const char *a);
static void CLR_GPRS_Buf(void);
void send_at_cmd_noack(char *s_at_cmd,u16 cmd_len);
void wifi_connect(void);
void wifi_send(void);
void mode_set(void);
void esp8266_ipnb_sava(u8 mode);
