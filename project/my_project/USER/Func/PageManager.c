#include "PageManager.h"

PageStack_t g_Page_PageStack = {0};

Page_t Page_Home;
Page_t Page_Menu;
Page_t Page_A;


/*
 * @brief 初始化页面栈
 * @param PageStack 页面栈指针
 * @return None
 **/
static void PageStack_Init(PageStack_t *PageStack)
{
    PageStack->PageStackTop = 0;
}

/*
 * @brief 压页面栈
 * @param PageStack 页面栈指针
 * @param Page 页面
 * @return 压栈成功 true 压栈失败 false
 **/
static bool PageStack_Push(PageStack_t *PageStack, Page_t *Page)
{
    if(PageStack->PageStackTop >= MAX_STACK_DEPTH - 1)
    {
        return false;
    }
    PageStack->PageStack[PageStack->PageStackTop++] = Page;
    return true;
}

/*
 * @brief 弹页面栈
 * @param PageStack 页面栈指针
 * @return 弹栈成功 true 弹栈失败 false
 **/
static bool PageStack_Pop(PageStack_t *PageStack)
{
    if(PageStack->PageStackTop <= 0) //栈底不可pop
    {
        return false;
    }
    PageStack->PageStackTop--;
    PageStack->PageStack[PageStack->PageStackTop]->DeInit();
    return true;
}

/*
 * @brief 判断页面栈是否为空
 * @param PageStack 页面栈指针
 * @return 1: 空 0: 不为空
 **/
static uint8_t PageStackIsEmpty(PageStack_t *PageStack)
{
    return PageStack->PageStackTop == 0; 
}

/*
 * @brief 获取页面栈顶部页面指针
 * @param PageStack 页面栈指针
 * @return 栈顶页面指针
 **/
static Page_t* get_top_page(PageStack_t *PageStack)
{
    if(PageStackIsEmpty(PageStack)) return NULL;
    return (PageStack->PageStack[PageStack->PageStackTop - 1]);
}

/*
 * @brief 获取当前页面指针
 * @param None
 * @return 当前页面指针
 **/
Page_t* Page_Get_NowPage(void)
{
    return get_top_page(&g_Page_PageStack);
}

/*
 * @brief 返回上一页
 * @param None
 * @return None
 **/
void Page_Back(void)
{
    if(PageStackIsEmpty(&g_Page_PageStack)) return;

    if(PageStack_Pop(&g_Page_PageStack) == false)
    {
        PageStack_Push(&g_Page_PageStack, &Page_Home);
        PageStack_Push(&g_Page_PageStack, &Page_Menu);
        g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->Init();
        lv_scr_load_anim(*g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->page_obj, 
                         LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);
                        
    }
    else
    {
        g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->Init();
        lv_scr_load_anim(*g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->page_obj,
                         LV_SCR_LOAD_ANIM_MOVE_LEFT, 100, 0, true);
    }
}
/*
 * @brief 直接返回到首页
 * @param None
 * @return None
 **/
void Page_BackToBottom(void)
{
    if(PageStackIsEmpty(&g_Page_PageStack)) return;
    while(g_Page_PageStack.PageStackTop > 1)
    {
        PageStack_Pop(&g_Page_PageStack);
    }
    g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->Init();
    lv_scr_load_anim(*g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->page_obj,
                     LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);
}

/*
 * @brief 加载页面
 * @param Page 页面指针
 * @return None
 **/
void Page_Load(Page_t *Page)
{
    if(g_Page_PageStack.PageStackTop >= MAX_STACK_DEPTH - 1) return;

    //如果堆栈非空，反初始化当前页面
    if(g_Page_PageStack.PageStackTop > 0)
    {
        g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->DeInit();
    }
    //压栈
    PageStack_Push(&g_Page_PageStack, Page);
    //初始化新页面
    g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->Init();
    //加载新页面    
    lv_scr_load_anim(*g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->page_obj, 
                     LV_SCR_LOAD_ANIM_MOVE_LEFT, 100, 0, true);
}
//======================================================
static void HomePage_Init(void)
{
    static lv_obj_t* home_page; 
    home_page = lv_obj_create(NULL);
    Page_Home.page_obj = &home_page;
    lv_obj_set_size(*Page_Home.page_obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(*Page_Home.page_obj, lv_color_hex(0xff0000), 0);  //红色
    lv_obj_t *label_home = lv_label_create(*Page_Home.page_obj);
    lv_label_set_text(label_home, "home");
    lv_obj_center(label_home);
    lv_obj_set_style_text_font(label_home, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_home, lv_color_white(), 0);

}
static void MenuPage_Init(void)
{
    static lv_obj_t* menu_page; 
    menu_page = lv_obj_create(NULL);
    Page_Menu.page_obj = &menu_page;
    lv_obj_set_size(*Page_Menu.page_obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(*Page_Menu.page_obj, lv_color_hex(0x0000ff), 0); //蓝色
    lv_obj_t *label_menu = lv_label_create(*Page_Menu.page_obj);
    lv_label_set_text(label_menu, "menu");
    lv_obj_center(label_menu);
    lv_obj_set_style_text_font(label_menu, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_menu, lv_color_white(), 0);
}
static void APage_Init(void)
{
    static lv_obj_t* A_page; 
    A_page = lv_obj_create(NULL);
    Page_A.page_obj = &A_page;
    lv_obj_set_size(*Page_A.page_obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(*Page_A.page_obj, lv_color_hex(0xFFFF80), 0); //黄色
    lv_obj_t *label_A = lv_label_create(*Page_A.page_obj);
    lv_label_set_text(label_A, "AAAAAAAAA");
    lv_obj_center(label_A);
    lv_obj_set_style_text_font(label_A, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_A, lv_color_white(), 0);
}

static void DeInit(void)
{

}
//======================================================
/*
 * @brief 初始化页面栈并加载首页
 * @param None
 * @return None
 **/
void Pages_Init(void)
{
    Page_Menu.Init = MenuPage_Init;
    Page_Menu.DeInit = DeInit;
    Page_Home.Init = HomePage_Init;
    Page_Home.DeInit = DeInit;
    Page_A.Init = APage_Init;
    Page_A.DeInit = DeInit;
    PageStack_Init(&g_Page_PageStack);
    //压栈
    PageStack_Push(&g_Page_PageStack, &Page_Home);
    //初始化新页面
    g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->Init();
    //加载新页面
    lv_scr_load(*g_Page_PageStack.PageStack[g_Page_PageStack.PageStackTop -1]->page_obj);
}


