#ifndef CST816T_H   
#define CST816T_H

#include "main.h"

/* functions define */
#define CST816T_RST_0 HAL_GPIO_WritePin(TP_RST_GPIO_Port, TP_RST_Pin, GPIO_PIN_RESET)
#define CST816T_RST_1 HAL_GPIO_WritePin(TP_RST_GPIO_Port, TP_RST_Pin, GPIO_PIN_SET)

/* 设备地址 */
#define CST816T_Device_Addr 	0x15

/* 触摸屏寄存器 */
#define CST816T_GestureID 		     	0x01  /* 手势ID，0x00=无手势 0x01=下滑 0x02=上滑 0x03=左滑 0x04=右滑 0x05=单击 0x0B=双击 0x0C=长按 */
#define CST816T_FingerNum 		    	0x02  /* 触摸手指数量，0x00=无触摸 0xFF=睡眠状态 */
#define CST816T_XposH 					0x03  /* X坐标高4位（低半字节有效） */
#define CST816T_XposL 					0x04  /* X坐标低8位 */
#define CST816T_YposH 					0x05  /* Y坐标高4位（低半字节有效） */
#define CST816T_YposL 					0x06  /* Y坐标低8位 */
#define CST816T_ChipID 					0xA7  /* 芯片ID寄存器 */
#define CST816T_SleepMode 			    0xE5  /* 睡眠模式控制，0x03=进入睡眠*/
#define CST816T_DoubleClickInspect 		0xEC  /* 双击检测使能寄存器，0x01=使能 */
#define CST816T_IrqPluseWidth 	        0xED  /* 中断低脉冲宽度配置，单位0.1ms */
#define CST816T_NorScanPer 			    0xEE  /* 正常快速检测周期，单位10ms，范围1~30 */
#define CST816T_MotionSlAngle 	        0xEF  /* 手势滑动分区角度，Angle=tan(c)*10 */
#define CST816T_LpAutoWakeTime 	        0xF4  /* 低功耗自动唤醒时间 */
#define CST816T_LpScanTH 					0xF5  /* 低功耗扫描唤醒门限，越小越灵敏，默认48 */
#define CST816T_LpScanWin 			    0xF6  /* 低功耗扫描窗口 */
#define CST816T_LpScanFreq 			    0xF7  /* 低功耗扫描频率 */
#define CST816T_LpScanIdac 			    0xF8  /* 低功耗扫描电流配置 */
#define CST816T_AutoSleepTime 	        0xF9  /* 自动休眠时间，单位秒，时间内无触摸进入低功耗 */
#define CST816T_IrqCtl 					0xFA  /* 中断控制，bit4=手势使能 bit5=坐标变化使能 bit6=触摸使能 */
#define CST816T_AutoReset 			    0xFB  /* 自动复位配置 */
#define CST816T_LongPressTime 		    0xFC  /* 长按时间配置 */
#define CST816T_IOCtl 					0xFD  /* IO控制寄存器 */
#define CST816T_DisAutoSleep 		    0xFE  /* 禁用自动休眠 */

/* 触摸屏坐标结构体 */
typedef struct
{
	uint16_t X_Pos;
	uint16_t Y_Pos;

} CST816T_Info;

/* 手势ID识别选项 */
typedef enum
{
    NOGESTURE   = 0x00,   // 无手势
    UPGLIDE     = 0x01,   // 上滑
    DOWNGLIDE   = 0x02,   // 下滑
    LEFTGLIDE   = 0x03,   // 左滑
    RIGHTGLIDE  = 0x04,   // 右滑
    CLICK       = 0x05,   // 单击
    DOUBLECLICK = 0x0B,   // 双击
    LONGPRESS   = 0x0C,   // 长按
    PALM        = 0xAA    // 大手掌/大面积触摸
} CST816T_GestureID_TypeDef;


typedef enum
{
	CST816T_INIT_OK,
	CST816T_INIT_ERROR,
	CST816T_STATUS_NONE = 4,
	CST816T_STATUS_PRESS,
	CST816T_STATUS_RELEASE,
	CST816T_STATUS_HOLDMOVE
} CST816T_Status_TypeDef;

extern CST816T_Info CST816T_Data;

void CST816T_GPIO_Init(void);
void CST816T_RESET(void);
void CST816T_Get_XY_AXIS(void);
uint8_t CST816T_Get_FingerNum(void);
uint8_t CST816T_Get_ChipID(void);
void CST816T_Config_DoubleClick(uint8_t enable);
CST816T_GestureID_TypeDef CST816T_Get_Gesture(void);
void CST816T_Config_AutoSleepTime(uint8_t time);
void CST816T_Config_DisableAutoSleep(void);
void CST816T_Sleep(void);
void CST816T_Wakeup(void);
void CST816T_Config_MotionSlAngle(uint8_t x_right_y_up_angle);
void CST816T_Config_NorScanPer(uint8_t Period);
void CST816T_Config_IrqPluseWidth(uint8_t Width);
void CST816T_Config_LpScanTH(uint8_t TH);
CST816T_Status_TypeDef CST816T_Get_Status(void);
void CST816T_Config_PressTime(uint8_t time);
CST816T_Status_TypeDef CST816T_Init(void);
#endif


