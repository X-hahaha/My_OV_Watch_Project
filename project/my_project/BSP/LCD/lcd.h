#ifndef __LCD_H__
#define __LCD_H__

#include "main.h"
#include "spi.h"
#include "stdint.h"

typedef void (*LCD_CallbackFunc_t)(void);

#define PWM_PERIOD         300
#define LCD_Show_Offset    20
/* LCD使用的是ST7789T3驱动芯片 */
//=================== ST7789T3 命令寄存器 ===================
#define ST7789_NOP         0x00  // 空操作
#define ST7789_SWRESET     0x01  // 软件复位
#define ST7789_SLPIN       0x10  // 进入睡眠
#define ST7789_SLPOUT      0x11  // 退出睡眠
#define ST7789_INVON       0x21  // 打开显示反转
#define ST7789_INVOFF      0x20  // 关闭显示反转
#define ST7789_DISPOFF     0x28  // 关闭显示
#define ST7789_DISPON      0x29  // 打开显示
#define ST7789_CASET       0x2A  // 列地址设置
#define ST7789_RASET       0x2B  // 行地址设置
#define ST7789_RAMWR       0x2C  // 显存写入
#define ST7789_MADCTL      0x36  // 显示方向/镜像控制
#define ST7789_COLMOD      0x3A  // 接口像素格式
#define ST7789_PORCTRL     0xB2  // Porch时序控制
#define ST7789_GCTRL       0xB7  // 栅极控制
#define ST7789_VCOMS       0xBB  // VCOM电压设置
#define ST7789_LCMCTRL     0xC0  // LCM电源控制
#define ST7789_VDVVRHEN    0xC2  // VDV/VRH使能
#define ST7789_VRHS        0xC3  // VRH参考电压
#define ST7789_VDVS        0xC4  // VDV电压
#define ST7789_FRCTRL2     0xC6  // 帧率控制
#define ST7789_PWCTRL1     0xD0  // 电源控制1
#define ST7789_PVGAMCTRL   0xE0  // 正极性Gamma校正
#define ST7789_NVGAMCTRL   0xE1  // 负极性Gamma校正

//=================== ST7789T3 命令数据参数 ===================

// ---- MADCTL (0x36) 显示方向 ----
#define ST7789_MADCTL_PORTRAIT       0x00  // 竖屏(默认)
#define ST7789_MADCTL_LANDSCAPE_180  0xC0  // 横屏180°
#define ST7789_MADCTL_LANDSCAPE_MIR  0x70  // 横屏镜像
#define ST7789_MADCTL_LANDSCAPE_ALT  0xA0  // 横屏(行列互换)

// ---- COLMOD (0x3A) 像素格式 ----
#define ST7789_COLMOD_RGB565         0x05  // 16-bit/pixel, RGB565
#define ST7789_COLMOD_RGB666         0x06  // 18-bit/pixel, RGB666

// ---- PORCTRL (0xB2) Porch参数 ----
#define ST7789_PORCTRL_NORMAL_BP     0x0C
#define ST7789_PORCTRL_NORMAL_FP     0x0C
#define ST7789_PORCTRL_IDLE_BP       0x00
#define ST7789_PORCTRL_PARTIAL_FP    0x33
#define ST7789_PORCTRL_IDLE_FP       0x33

// ---- GCTRL (0xB7) 栅极电压 ----
#define ST7789_GCTRL_VGH13_VGL10     0x35  // VGH=13.26V, VGL=-10.43V

// ---- VCOMS (0xBB) VCOM ----
#define ST7789_VCOMS_OFFSET_M725     0x19  // VCOM偏移=-0.725V

// ---- LCMCTRL (0xC0) ----
#define ST7789_LCMCTRL_DEFAULT       0x2C

// ---- VDVVRHEN (0xC2) ----
#define ST7789_VDVVRHEN_ENABLE       0x01

// ---- VRHS (0xC3) ----
#define ST7789_VRHS_4P65V            0x12  // GVDD参考=4.65V

// ---- VDVS (0xC4) ----
#define ST7789_VDVS_DEFAULT          0x20

// ---- FRCTRL2 (0xC6) 帧率 ----
#define ST7789_FRCTRL2_60HZ          0x0F  // 正常模式≈60Hz

// ---- PWCTRL1 (0xD0) 电源电压 ----
#define ST7789_PWCTRL1_AVDD_6P8V     0xA4
#define ST7789_PWCTRL1_AVCL_M4P8V    0xA1

// ---- PVGAMCTRL (0xE0) 正极性Gamma ----
#define ST7789_PVGAMCTRL_V0          0xD0
#define ST7789_PVGAMCTRL_V1          0x04
#define ST7789_PVGAMCTRL_V2          0x0D
#define ST7789_PVGAMCTRL_V3          0x11
#define ST7789_PVGAMCTRL_V4          0x13
#define ST7789_PVGAMCTRL_V5          0x2B
#define ST7789_PVGAMCTRL_V6          0x3F
#define ST7789_PVGAMCTRL_V7          0x54
#define ST7789_PVGAMCTRL_V8          0x4C
#define ST7789_PVGAMCTRL_V9          0x18
#define ST7789_PVGAMCTRL_V10         0x0D
#define ST7789_PVGAMCTRL_V11         0x0B
#define ST7789_PVGAMCTRL_V12         0x1F
#define ST7789_PVGAMCTRL_V13         0x23

// ---- NVGAMCTRL (0xE1) 负极性Gamma ----
#define ST7789_NVGAMCTRL_V0          0xD0
#define ST7789_NVGAMCTRL_V1          0x04
#define ST7789_NVGAMCTRL_V2          0x0C
#define ST7789_NVGAMCTRL_V3          0x11
#define ST7789_NVGAMCTRL_V4          0x13
#define ST7789_NVGAMCTRL_V5          0x2C
#define ST7789_NVGAMCTRL_V6          0x3F
#define ST7789_NVGAMCTRL_V7          0x44
#define ST7789_NVGAMCTRL_V8          0x51
#define ST7789_NVGAMCTRL_V9          0x2F
#define ST7789_NVGAMCTRL_V10         0x1F
#define ST7789_NVGAMCTRL_V11         0x1F
#define ST7789_NVGAMCTRL_V12         0x20
#define ST7789_NVGAMCTRL_V13         0x23

//=====================================================================
#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W_pixel           240
#define LCD_X_MAX             (240 - 1)
#define LCD_Y_MAX             (280 - 1)
#define LCD_H_pixel           280

#else
#define LCD_W_pixel           280
#define LCD_X_MAX             (280 - 1)
#define LCD_Y_MAX             (240 - 1)
#define LCD_H_pixel           240
#endif
//=====================================================================

#define LCD_TP_SCLK_Clr()        HAL_GPIO_WritePin(TP_SCL_GPIO_Port,TP_SCL_Pin,GPIO_PIN_RESET)//SCL=SCLK
#define LCD_TP_SCLK_Set()        HAL_GPIO_WritePin(TP_SCL_GPIO_Port,TP_SCL_Pin,GPIO_PIN_SET)

#define LCD_MOSI_Clr()           HAL_GPIO_WritePin(TP_SDA_GPIO_Port,TP_SDA_Pin,GPIO_PIN_RESET)//SDA=MOSI
#define LCD_MOSI_Set()           HAL_GPIO_WritePin(TP_SDA_GPIO_Port,TP_SDA_Pin,GPIO_PIN_SET)

#define LCD_TP_RST_Clr()         HAL_GPIO_WritePin(TP_RST_GPIO_Port,TP_RST_Pin,GPIO_PIN_RESET)//TP_RST
#define LCD_TP_RST_Set()         HAL_GPIO_WritePin(TP_RST_GPIO_Port,TP_RST_Pin,GPIO_PIN_SET)

#define LCD_RST_Clr()            HAL_GPIO_WritePin(LCD_RST_GPIO_Port,LCD_RST_Pin,GPIO_PIN_RESET)//LCD_RST
#define LCD_RST_Set()            HAL_GPIO_WritePin(LCD_RST_GPIO_Port,LCD_RST_Pin,GPIO_PIN_SET)

#define LCD_DC_Clr()             HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_RESET)//DC
#define LCD_DC_Set()             HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET)

#define LCD_CS_Clr()             HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_RESET)//CS
#define LCD_CS_Set()             HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_SET)

#define LCD_Writ_SPIBus(data)    HAL_SPI_Transmit(&hspi1, &data, 1, 1)


//颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE           	 0x001F
#define BRED             0XF81F
#define GRED 			       0XFFE0
#define GBLUE			       0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			     0XBC40 //棕色
#define BRRED 			     0XFC07 //棕红色
#define GRAY  			     0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			     0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

typedef enum
{
	LCD_FONT_6X12,
	LCD_FONT_8X16,
	LCD_FONT_12X24,
	LCD_FONT_16X32,
}LCD_FONT_t;

typedef enum
{
	LCD_FONT_MODE_NORMAL = 0,
	LCD_FONT_MODE_INVERT = 1,
}LCD_FONT_MODE_t;


/*
 * @brief 写入8位数据
 * @param data: 要写入的数据
 */
void LCD_WR_DATA8(uint8_t data);

/*
 * @brief 写入16位数据
 * @param dat: 要写入的数据
 */
void LCD_WR_DATA16(uint16_t dat);

/**
 * @brief 设置显示区域
 * @param x1: 左上角横坐标
 * @param y1: 左上角纵坐标
 * @param x2: 右下角横坐标
 * @param y2: 右下角纵坐标
 */
void LCD_ShowArea_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);

/*
 * @brief 设置背光亮度
 * @param dc: 背光亮度，范围2-100
 */

void LCD_Set_Light(uint8_t dc);

/*
 * @brief 关闭背光
 */
void LCD_Close_Light(void);

/*
 * @brief 打开背光
 */
void LCD_Open_Light(void);

/*
 * @brief 进入睡眠模式
 */
void LCD_SleepIn(void);

/**
 * @brief 退出睡眠模式
 */
void LCD_SleepOut(void);

/**
 * @brief 初始化LCD
 * 
 */
void LCD_Init(void);




void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);//指定区域填充颜色
HAL_StatusTypeDef LCD_Color_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color_p);
void LCD_Set_Flush_Complete_Callback(LCD_CallbackFunc_t cb);
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);//在指定位置画一个点
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);//在指定位置画一条线
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);//在指定位置画一个矩形
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);//在指定位置画一个圆
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t ch, uint16_t fc, uint16_t bc, LCD_FONT_t FONT, LCD_FONT_MODE_t mode);

#endif
