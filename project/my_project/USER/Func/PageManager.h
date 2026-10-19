#ifndef __PAGE_MANGER_H__
#define __PAGE_MANGER_H__

#include "lvgl.h"

#define MAX_STACK_DEPTH 6



typedef struct
{
    void (*Init)(void);
    void (*DeInit)(void);
    lv_obj_t **page_obj;
}Page_t;
extern Page_t Page_Home;
extern Page_t Page_Menu;
extern Page_t Page_A;
typedef struct
{
    Page_t* PageStack[MAX_STACK_DEPTH];
    uint8_t PageStackTop;
}PageStack_t;

Page_t* Page_Get_NowPage(void);
void Page_Back(void);
void Page_BackToBottom(void);
void Page_Load(Page_t *Page);
void Pages_Init(void);



#endif
