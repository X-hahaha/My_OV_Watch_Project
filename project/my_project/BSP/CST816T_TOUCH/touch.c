#include "IIC_hal.h"
#include "touch.h"

IIC_Bus_t CST816T_iic_bus = {
        .IIC_SCL_PORT = TP_SCL_GPIO_Port,
        .IIC_SCL_PIN = TP_SCL_Pin,
        .IIC_SDA_PORT = TP_SDA_GPIO_Port,
        .IIC_SDA_PIN = TP_SDA_Pin,  
};

CST816T_Info CST816T_Data = {0};


static void CST816T_Write_Reg(uint8_t reg, uint8_t val)
{
    IIC_Write_One_Byte(&CST816T_iic_bus, CST816T_Device_Addr, reg, val);
}
static int16_t CST816T_Read_Reg(uint8_t reg)
{
    return IIC_Read_One_Byte(&CST816T_iic_bus, CST816T_Device_Addr, reg);
}
static void CST816T_Read_MultiReg(uint8_t reg, uint8_t *buff, uint8_t len)
{
    IIC_Read_Multi_Bytes(&CST816T_iic_bus, CST816T_Device_Addr, reg, buff, len);
}


/*
 * @brief 初始化GPIO，前面已经进行过初始化GPIO了，所以这里只需要设置引脚电平就行了
*/
void CST816T_GPIO_Init(void)
{
    HAL_GPIO_WritePin(TP_RST_GPIO_Port, TP_RST_Pin, GPIO_PIN_SET); //复位脚，为1时不复位，为0时复位
    IIC_Init(&CST816T_iic_bus); //IIC脚
}

/*
 * @brief 复位芯片
*/
void CST816T_RESET(void)
{
    CST816T_RST_0;
    HAL_Delay(10);
    CST816T_RST_1;
    HAL_Delay(100);
}
/*
 * @brief 获取触摸点坐标
*/
void CST816T_Get_XY_AXIS(void)
{
    uint8_t buff[4];
    CST816T_Read_MultiReg(CST816T_XposH, buff, 4);
    CST816T_Data.X_Pos = ((buff[0] & 0x0F) << 8) | buff[1];
    CST816T_Data.Y_Pos = ((buff[2] & 0x0F) << 8) | buff[3];

}
/*
 * @brief 获取触摸点数量
 * 
 * @return uint8_t 触摸点数量
*/
uint8_t CST816T_Get_FingerNum(void)
{
	return CST816T_Read_Reg(CST816T_FingerNum);
}

/*
 * @brief 获取芯片ID
 * 
 * @return uint8_t 芯片ID
*/
uint8_t CST816T_Get_ChipID(void)
{
	return CST816T_Read_Reg(CST816T_ChipID);
}

/*
 * @brief 开启双击检测
*/
void CST816T_Config_DoubleClick(uint8_t enable)
{
    CST816T_Write_Reg(CST816T_DoubleClickInspect, enable);
}

/*
 * @brief 获取手势ID
 * 
 * @return CST816T_GestureID_TypeDef 手势ID
*/
CST816T_GestureID_TypeDef CST816T_Get_Gesture(void)
{
    return (CST816T_GestureID_TypeDef)CST816T_Read_Reg(CST816T_GestureID);
}

/*
 * @brief 配置自动睡眠时间
 * 
 * @param time 自动睡眠时间，单位：秒
*/
void CST816T_Config_AutoSleepTime(uint8_t time)
{
    //开启自动休眠
    CST816T_Write_Reg(CST816T_DisAutoSleep, 0);
    //设置自动休眠时间
    CST816T_Write_Reg(CST816T_AutoSleepTime, time);
}

void CST816T_Config_DisableAutoSleep(void)
{
    //关闭自动休眠
    CST816T_Write_Reg(CST816T_DisAutoSleep, 1);
}

/*
 * @brief 进入深度睡眠模式，只有拉低复位引脚1ms才能唤醒
 * @Note 深度睡眠模式的功耗为1uA，睡眠模式/待机模式的功耗为8uA
*/
void CST816T_Sleep(void)
{
	CST816T_Write_Reg(CST816T_SleepMode, 0x03);
}

/*
 * @brief 从深度睡眠模式唤醒
*/
void CST816T_Wakeup(void)
{
	CST816T_RESET();
}

/*
 * @brief 配置手势滑动分区角度，Angle=tan(c)*10
 * 
 * @param x_right_y_up_angle 角度值
*/
void CST816T_Config_MotionSlAngle(uint8_t x_right_y_up_angle)
{
	CST816T_Write_Reg(CST816T_MotionSlAngle, x_right_y_up_angle);
}

/*
 * @brief 配置正常扫描周期
*/
void CST816T_Config_NorScanPer(uint8_t Period)
{
	if(Period >= 30) Period = 30;
	else if(Period < 1) Period = 1;
	CST816T_Write_Reg(CST816T_NorScanPer, Period);
}

/*
 * @brief 配置中断脉冲宽度
 * 
 * @param Width 中断脉冲宽度，单位：ms，范围：1ms-5ms
*/
void CST816T_Config_IrqPluseWidth(uint8_t Width)
{
	if(Width >= 5) Width = 5;
	else if(Width == 0) Width = 1;
	CST816T_Write_Reg(CST816T_IrqPluseWidth, Width);
}

/*
 * @brief 配置待机模式唤醒阈值
 * 
 * @param TH 待机模式唤醒阈值，默认值47
*/
void CST816T_Config_LpScanTH(uint8_t TH)
{
	CST816T_Write_Reg(CST816T_LpScanTH, TH);
}
/*
 * @brief 获取触摸屏状态按下，释放，滑动
 * 
 * @return CST816T_Status_TypeDef 触摸屏状态
*/
CST816T_Status_TypeDef CST816T_Get_Status(void)
{
    uint8_t status = 0;
    
    if(CST816T_Get_FingerNum() != 0)
    {
        status = (CST816T_Read_Reg(CST816T_XposH) >> 6) & 0x03;
        switch (status)
        {
        case 0x00:
            return CST816T_STATUS_PRESS;
            break;
        case 0x01:
            return CST816T_STATUS_RELEASE;
            break;
        case 0x02:   
            return CST816T_STATUS_HOLDMOVE;
            break;  
        default:
            return CST816T_STATUS_NONE;
            break;
        }
    }
    else 
    {
        return CST816T_STATUS_NONE;
    }

}

/*
 * @brief 配置长按事件阈值时间
*/
void CST816T_Config_PressTime(uint8_t time)
{
    //设置长按事件阈值时间
    if(time < 1) time =20;
    CST816T_Write_Reg(CST816T_LongPressTime, time);
}

/*
 * @brief 初始化触摸屏
*/
CST816T_Status_TypeDef CST816T_Init(void)
{
	CST816T_GPIO_Init();
    CST816T_RESET();
    if(CST816T_Read_Reg(CST816T_ChipID) == IIC_NotACK)
    {
        return CST816T_INIT_ERROR;
    }
    CST816T_Config_DoubleClick(0x01);
	CST816T_Config_AutoSleepTime(5);
    return CST816T_INIT_OK;
}