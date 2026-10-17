/**
 * @file lv_port_disp_templ.h
 *
 */

 /*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"
#include "lcd.h"

/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES LCD_W_pixel
#define MY_DISP_VER_RES LCD_H_pixel

#define DISP_BUF_LINES  24
/**********************
 *      TYPEDEFS
 **********************/
extern lv_disp_drv_t * g_disp_drv;
extern lv_disp_t * g_disp;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_disp_init(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
