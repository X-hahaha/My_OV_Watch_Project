#include "delay.h"
#include "lcd.h"
#include "main.h"
#include "tim.h"
#include "lcd_font.h"

static LCD_CallbackFunc_t lcd_ready_cb = NULL;

/**
 * @brief 初始化LCD GPIO
 * 
 */
static void LCD_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure = {0};
	
	GPIO_InitStructure.Pin = LCD_RST_Pin|LCD_CS_Pin|LCD_DC_Pin;	 
 	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 		   //推挽输出
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  //速度50MHz
 	HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStructure);	           //初始化GPIOB
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin|LCD_CS_Pin|LCD_DC_Pin, GPIO_PIN_SET);
}

static void LCD_WR_COMMAND(uint8_t command)
{
    LCD_DC_Clr();
    LCD_Writ_SPIBus(command);
    LCD_DC_Set();   //这里把DC引脚拉高，后续就可以直接写数据了，而且对于写多个数据LCD_WR_DATA8就不用反复拉高了

}
/*
 * @brief 写入8位数据
 * @param data: 要写入的数据
 */
void LCD_WR_DATA8(uint8_t data)
{
    LCD_Writ_SPIBus(data);  
}
/*
 * @brief 写入16位数据
 * @param dat: 要写入的数据
 */
void LCD_WR_DATA16(uint16_t dat)
{
	uint8_t temp[2];
	temp[0]=(dat >> 8) & 0xff;
	temp[1]=dat & 0xff;
	HAL_SPI_Transmit(&hspi1, temp, 2, 1);
}

/**
 * @brief 设置显示区域
 * @param x1: 左上角横坐标
 * @param y1: 左上角纵坐标
 * @param x2: 右下角横坐标
 * @param y2: 右下角纵坐标
 */
void LCD_ShowArea_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	y1 = y1 + LCD_Show_Offset;
	y2 = y2 + LCD_Show_Offset;

	LCD_WR_COMMAND(ST7789_CASET);//列地址设置
	LCD_WR_DATA16(x1);
	LCD_WR_DATA16(x2);
	LCD_WR_COMMAND(ST7789_RASET);//行地址设置
	LCD_WR_DATA16(y1);
	LCD_WR_DATA16(y2);
	LCD_WR_COMMAND(ST7789_RAMWR);//显存写入
}

/*
 * @brief 设置背光亮度
 * @param dc: 背光亮度，范围2-100
 */
void LCD_Set_Light(uint8_t dc)
{
	if(dc >= 2 && dc <= 100)
	{
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, (dc * PWM_PERIOD) / 100);
	}
}

/*
 * @brief 关闭背光
 */
void LCD_Close_Light(void)
{
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3,0);
}

/*
 * @brief 打开背光
 */
void LCD_Open_Light(void)
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

/*
 * @brief 进入睡眠模式
 */
void LCD_SleepIn(void)
{
	LCD_WR_COMMAND(ST7789_SLPIN);
	delay_ms(100);
}

/**
 * @brief 退出睡眠模式
 */
void LCD_SleepOut(void)
{
	LCD_WR_COMMAND(ST7789_SLPOUT);
	delay_ms(100);
}

/**
 * @brief 初始化LCD
 * 
 */
void LCD_Init(void)
{
	LCD_GPIO_Init();     //初始化GPIO
	LCD_CS_Clr();		 //chip select
	
	LCD_RST_Clr();		 //复位
	delay_ms(100);
	LCD_RST_Set();
	delay_ms(100);
	
	LCD_WR_COMMAND(ST7789_SLPOUT);  //退出睡眠模式
	delay_ms(120);              //等待120ms确保退出睡眠模式
	LCD_WR_COMMAND(ST7789_MADCTL);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(ST7789_MADCTL_PORTRAIT);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(ST7789_MADCTL_LANDSCAPE_180);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(ST7789_MADCTL_LANDSCAPE_MIR);
	else LCD_WR_DATA8(ST7789_MADCTL_LANDSCAPE_ALT);

	LCD_WR_COMMAND(ST7789_COLMOD);
	LCD_WR_DATA8(ST7789_COLMOD_RGB565);

	LCD_WR_COMMAND(ST7789_PORCTRL);
	LCD_WR_DATA8(ST7789_PORCTRL_NORMAL_BP);
	LCD_WR_DATA8(ST7789_PORCTRL_NORMAL_FP);
	LCD_WR_DATA8(ST7789_PORCTRL_IDLE_BP);
	LCD_WR_DATA8(ST7789_PORCTRL_PARTIAL_FP);
	LCD_WR_DATA8(ST7789_PORCTRL_IDLE_FP);

	LCD_WR_COMMAND(ST7789_GCTRL);
	LCD_WR_DATA8(ST7789_GCTRL_VGH13_VGL10);

	LCD_WR_COMMAND(ST7789_VCOMS);
	LCD_WR_DATA8(ST7789_VCOMS_OFFSET_M725);

	LCD_WR_COMMAND(ST7789_LCMCTRL);
	LCD_WR_DATA8(ST7789_LCMCTRL_DEFAULT);

	LCD_WR_COMMAND(ST7789_VDVVRHEN);
	LCD_WR_DATA8(ST7789_VDVVRHEN_ENABLE);

	LCD_WR_COMMAND(ST7789_VRHS);
	LCD_WR_DATA8(ST7789_VRHS_4P65V);

	LCD_WR_COMMAND(ST7789_VDVS);
	LCD_WR_DATA8(ST7789_VDVS_DEFAULT);

	LCD_WR_COMMAND(ST7789_FRCTRL2);
	LCD_WR_DATA8(ST7789_FRCTRL2_60HZ);

	LCD_WR_COMMAND(ST7789_PWCTRL1);
	LCD_WR_DATA8(ST7789_PWCTRL1_AVDD_6P8V);
	LCD_WR_DATA8(ST7789_PWCTRL1_AVCL_M4P8V);

	LCD_WR_COMMAND(ST7789_PVGAMCTRL);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V0);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V1);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V2);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V3);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V4);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V5);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V6);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V7);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V8);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V9);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V10);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V11);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V12);
	LCD_WR_DATA8(ST7789_PVGAMCTRL_V13);

	LCD_WR_COMMAND(ST7789_NVGAMCTRL);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V0);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V1);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V2);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V3);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V4);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V5);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V6);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V7);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V8);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V9);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V10);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V11);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V12);
	LCD_WR_DATA8(ST7789_NVGAMCTRL_V13);

	LCD_WR_COMMAND(ST7789_INVON);

	LCD_WR_COMMAND(ST7789_DISPON);  //显示画面
}


/*--------------------------------------------------*/

/**
 *
 * @brief 以指定颜色填充两对角点之间所有矩形区域的像素
 * @param xsta: 左上角横坐标[0-239]
 * @param ysta: 左上角纵坐标[0-279]
 * @param xend: 右下角横坐标[0-239]
 * @param yend: 右下角纵坐标[0-279]
 * @param color: 填充颜色
 */
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
	uint16_t i, j;
	LCD_ShowArea_Set(xsta, ysta, xend, yend);//设置显示范围
	for(i = ysta; i <= yend; i++)
	{
		for(j = xsta; j <= xend; j++)
		{
			LCD_WR_DATA16(color);
		}
	}
}

/**
 * @brief 以颜色数组填充两对角点之间所有矩形区域的像素
 * @param xsta: 左上角横坐标
 * @param ysta: 左上角纵坐标
 * @param xend: 右下角横坐标
 * @param yend: 右下角纵坐标
 * @param color_p: 指向颜色数组首元素的指针
 * @return HAL_StatusTypeDef: DMA传输状态
 * @note 该接口是以DMA方式传输颜色数组，传输完成后会调用回调lcd_ready_cb指向的函数
*/
HAL_StatusTypeDef LCD_Color_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color_p)
{
	uint16_t width, height;
	width = xend - xsta + 1;
	height = yend - ysta + 1;
	uint32_t pixel_count = width * height;
	uint32_t byte_count = pixel_count * 2U;

	if(byte_count > UINT16_MAX)
	{
		return HAL_ERROR;
	}

	LCD_ShowArea_Set(xsta, ysta, xend, yend);
	return HAL_SPI_Transmit_DMA(&hspi1, (const uint8_t*)color_p, byte_count);
}


void LCD_Set_Flush_Complete_Callback(LCD_CallbackFunc_t cb)
{
	lcd_ready_cb = cb;
}

// DMA 传输完成中断回调，最后调用lcd_ready_cb指向的函数
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1)
	{
		// 确保最后一位数据离开 STM32 的移位寄存器
		while (hspi->Instance->SR & SPI_FLAG_BSY);
		if (lcd_ready_cb)
		{
			lcd_ready_cb();
		}
	}
}

/**
 * @brief 在指定位置画点
 * @param x: 点的横坐标[0-239]
 * @param y: 点的纵坐标[0-279]
 * @param color: 点的颜色
 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_ShowArea_Set(x, y, x, y);//设置光标位置
	LCD_WR_DATA16(color);
}

/**
 * @brief 画直线
 * @param x1: 线的起始横坐标[0-239]
 * @param y1: 线的起始纵坐标[0-279]
 * @param x2: 线的结束横坐标[0-239]
 * @param y2: 线的结束纵坐标[0-279]
 * @param color: 线的颜色
 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	// Bresenham算法
	int delta_x = x2 - x1;
	int delta_y = y2 - y1;
	int x_incre, y_incre;
	int x_error = 0, y_error = 0;

	if(delta_x < 0)
	{
		delta_x = -delta_x;
		x_incre = -1;
	}
	else if(delta_x > 0)
	{
		delta_x = delta_x;
		x_incre = 1;
	}
	else
	{
		x_incre = 0;
	}

	if(delta_y < 0)
	{
		delta_y = -delta_y;
		y_incre = -1;
	}
	else if(delta_y > 0)
	{
		delta_y = delta_y;
		y_incre = 1;
	}
	else
	{
		y_incre = 0;
	}

	uint16_t distance = (delta_x >= delta_y) ? delta_x : delta_y;

	x_error = y_error = distance /2;
	for(uint16_t i = 0; i < distance; i++)
	{
		LCD_DrawPoint(x1, y1, color);
		
		x_error += delta_x;
		if(x_error >= distance)
		{
			x_error -= distance;
			x1 += x_incre;
		}
		
		y_error += delta_y;
		if(y_error >= distance)
		{
			y_error -= distance;
			y1 += y_incre;
		}
	}
}

/**
 * @brief 画矩形
 * @param x1: 矩形的左上角横坐标[0-239]
 * @param x2: 矩形的右下角横坐标[0-239]
 * @param y1: 矩形的左上角纵坐标[0-279]
 * @param y2: 矩形的右下角纵坐标[0-279]
 * @param color: 矩形线框的颜色
 */
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1, y1, x2, y1, color);
	LCD_DrawLine(x1, y1, x1, y2, color);
	LCD_DrawLine(x1, y2, x2, y2, color);
	LCD_DrawLine(x2, y1, x2, y2, color);	
}

/**
 * @brief 画圆
 * @param x: 圆心的横坐标[0-239]
 * @param y: 圆心的纵坐标[0-279]
 * @param radius: 圆的半径
 * @param color: 圆线框的颜色
 */
void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
	int a, b;
	a = 0;
	b = radius;
	while(a <= b)
	{
		LCD_DrawPoint(x + a, y + b, color);
		LCD_DrawPoint(x - a, y + b, color);
		LCD_DrawPoint(x + a, y - b, color);
		LCD_DrawPoint(x - a, y - b, color);
		LCD_DrawPoint(x + b, y + a, color);
		LCD_DrawPoint(x - b, y + a, color);
		LCD_DrawPoint(x + b, y - a, color);
		LCD_DrawPoint(x - b, y - a, color);

		a++;
		if(a * a + b * b > radius * radius)
		{
			b--;
		}
	}
}

void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
	while(*p!='\0')
	{
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}
}



void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t ch, uint16_t fc, uint16_t bc, LCD_FONT_t FONT, LCD_FONT_MODE_t mode)
{
	uint8_t temp, sizex = 0, t = 0, m=0;
	uint16_t i, TypefaceNum;//一个字符所占字节大小
	uint16_t x0 = x;

	switch(FONT)
	{
		case LCD_FONT_6X12:
			sizex=6;
			break;
		case LCD_FONT_8X16:
			sizex=8;
			break;
		case LCD_FONT_12X24:
			sizex=12;
			break;
		case LCD_FONT_16X32:
			sizex=16;
			break;
	}
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * (sizex * 2);
	ch = ch - ' ';    //得到偏移后的值
	LCD_ShowArea_Set(x ,y, x + sizex - 1, y + 2 * sizex - 1);  //设置光标位置

	for(i = 0; i < TypefaceNum; i++)
	{
		if(FONT==LCD_FONT_6X12) temp = ascii_1206[ch][i];		       //调用6x12字体
		else if(FONT==LCD_FONT_8X16) temp = ascii_1608[ch][i];		 //调用8x16字体
		else if(FONT==LCD_FONT_12X24) temp = ascii_2412[ch][i];		 //调用12x24字体
		else if(FONT==LCD_FONT_16X32) temp = ascii_3216[ch][i];		 //调用16x32字体
		else return;
		for(t = 0; t < 8; t++)
		{
			if(mode == LCD_FONT_MODE_NORMAL)//非叠加模式
			{
				if(temp & (0x01 << t))LCD_WR_DATA16(fc);
				else LCD_WR_DATA16(bc);
				m++;
				if(m % sizex==0)
				{
					m = 0;
					break;
				}
			}
			else if(mode == LCD_FONT_MODE_INVERT)//叠加模式
			{
				if(temp & (0x01 << t))LCD_DrawPoint(x, y, fc);//画一个点
				x++;
				if((x - x0) == sizex)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}
}