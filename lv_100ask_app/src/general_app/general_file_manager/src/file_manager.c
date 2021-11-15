/**
 ******************************************************************************
 * @file    file_manager.c
 * @author  百问科技
 * @version V1.0
 * @date    2021-11-03
 * @brief	文件管理器
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2021-11-03     zhouyuebiao     First version
 ******************************************************************************
 * @attention
 *
 * Copyright (C) 2008-2021 深圳百问网科技有限公司<https://www.100ask.net/>
 * All rights reserved
 *
 ******************************************************************************
 */


/*********************
 *      INCLUDES
 *********************/
//#include "../../lv_100ask_demo.h"

#if 1 //LV_USE_100ASK_DEMO_FILE_MANAGER

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "file_manager.h"

#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define WIN_HEADER_HEIGHT       (60)
#define WIN_BTN_WIDTH           (50)


#define MENU_WIDTH  (22)
#define SHOW_WIDTH  (100 - MENU_WIDTH)

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * g_path_label;
static lv_obj_t * g_file_list;
static char * file_browser_dir[256] = {0};
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void win_event_handler(lv_event_t * e);
static void click_file_event_handler(lv_event_t * e);
static void show_dir(char * path);
static void strip_ext(char *fname);
static void shell_opt_handle(const char * cmd, char * result);
static void click_menu_item_event_handler(lv_event_t * e);



/* 去除最后的换行符 */
static void trim_string(char *str)
{
	int len = strlen(str);
 
	//去掉最后的换行符
	if(str[len-1] == '\n')
	{
		len--;		//字符串长度减一
		str[len] = 0;	//给字符串最后一个字符赋0，即结束符
	}
}


// 去掉最后的后缀名
static void strip_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end >= fname && *end != '/') {
        --end;
    }

    if (end > fname) {
        *end = '\0';
    }
    else if (end == fname) {
        *(end+1) = '\0';
    }

}


static void win_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    LV_LOG_USER("Button %d clicked", (int)lv_obj_get_index(obj));
}

static void click_menu_item_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    char * buff = lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        char * file_name[256];

        //LV_LOG_USER("Button %s clicked", buff);
        if((strcmp(buff, "..") == 0) && (strcmp(buff, "/") != 0))
        {
            strip_ext(file_browser_dir); // 去掉最后的 /路径
            lv_snprintf(file_name, sizeof(file_name), "%s", file_browser_dir);
            show_dir(file_name);
        }
        else show_dir(buff);
    }
}


static void click_file_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_CLICKED) {
        struct stat stat_buf;
        char * file_name[256];
        
        memset(file_name, 0, sizeof(file_name));      
        //trim_string(file_browser_dir); // 去掉最后的 \n
        if((strcmp(lv_list_get_btn_text(g_file_list, obj), "..") == 0) && (strcmp(file_browser_dir, "/") != 0))
        {
            strip_ext(file_browser_dir); // 去掉最后的 /路径
            lv_snprintf(file_name, sizeof(file_name), "%s", file_browser_dir);
        }
        else
        {
            if (strcmp(file_browser_dir, "/") == 0)
                lv_snprintf(file_name, sizeof(file_name), "%s%s", file_browser_dir, lv_list_get_btn_text(g_file_list, obj));
            else
                lv_snprintf(file_name, sizeof(file_name), "%s/%s", file_browser_dir, lv_list_get_btn_text(g_file_list, obj));
        }

        if((stat(file_name, &stat_buf) == -1))
		{
			LV_LOG_USER("stat error");
		}
		if(S_ISDIR(stat_buf.st_mode))
        {
            show_dir(file_name);
        }
    }
}


static void shell_opt_handle(const char * cmd, char * result)
{
    FILE * fp;
    char buffer[128] = {0};
    //char str_ret_buff[1024] = {0};

    fp=popen(cmd, "r");
    while (fgets(buffer, 128, fp) != NULL)
    {
        //printf("%s", buffer); 
        strcat(result, buffer);
    }

    if (pclose(fp) == -1)
    {
        LV_LOG_USER("close failed!");
        return NULL;
        //exit(1); // return -1;
    }
}


static void show_dir(char * path)
{
    //uint32_t i;
    lv_obj_t * btn;
    struct stat stat_buf;
    struct dirent **namelist;
    char str_file_path_name[256];
	int32_t file_n;

	file_n = scandir(path, &namelist, 0, alphasort);
    if (file_n < 0)
    {
        LV_LOG_USER("Not found!");
        return;
    }
    else
        lv_obj_clean(g_file_list); // 清空列表，删除对象的所有子项（但不是对象本身）
    
	for(uint16_t i = 0; i < file_n; i++) {
        // 获取文件的： 路径+名称
        memset(str_file_path_name, 0, sizeof(str_file_path_name));
        lv_snprintf(str_file_path_name, sizeof(str_file_path_name), "%s/%s", path, namelist[i]->d_name); 

        if((stat(str_file_path_name, &stat_buf) == -1))
		{
			LV_LOG_USER("stat error");
            continue;
		}

        // 识别并展示文件		
        if ((strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".png") == 0) || (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".PNG") == 0) ||\
            (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".jpg") == 0) || (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".JPG") == 0) ||\
            (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".bmp") == 0) || (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".BMP") == 0) ||\
            (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".gif") == 0) || (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".GIF") == 0))
            btn = lv_list_add_btn(g_file_list, LV_SYMBOL_IMAGE, namelist[i]->d_name);
        else if ((strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".mp3") == 0) || (strcmp((namelist[i]->d_name + (strlen(namelist[i]->d_name) - 4)) , ".MP3") == 0))
            btn = lv_list_add_btn(g_file_list, LV_SYMBOL_AUDIO, namelist[i]->d_name);
        else if(S_ISDIR(stat_buf.st_mode))
            btn = lv_list_add_btn(g_file_list, LV_SYMBOL_DIRECTORY, namelist[i]->d_name);
        else
            btn = lv_list_add_btn(g_file_list, LV_SYMBOL_FILE, namelist[i]->d_name);

        lv_obj_add_event_cb(btn, click_file_event_handler, LV_EVENT_CLICKED, NULL);
	}
    free(namelist);

    memset(file_browser_dir, 0, sizeof(file_browser_dir));
    strcpy(file_browser_dir, path);
    
    // 更新路径信息
    lv_label_set_text_fmt(g_path_label, LV_SYMBOL_EYE_OPEN" %s", path);
}


void file_manager_init(void)
{
    // win风格样式
    static lv_style_t win_sytle;
    lv_style_init(&win_sytle);
    //lv_style_set_bg_opa(&win_sytle, 0);
    lv_style_set_radius(&win_sytle, 0);
    //lv_style_set_text_font(&win_sytle, &lv_font_montserrat_20);
    lv_style_set_bg_color(&win_sytle, lv_color_hex(0xff813f));

    // win中btn的风格样式
    static lv_style_t win_btn;
    lv_style_init(&win_btn);
    //lv_style_set_bg_opa(&win_btn, 0);
    lv_style_set_border_opa(&win_btn, 0);
    lv_style_set_outline_opa(&win_btn, 0);
    lv_style_set_radius(&win_btn, 12);
    lv_style_set_text_font(&win_btn, &lv_font_montserrat_20);

    // win展示区域风格样式
    static lv_style_t win_cont_style;
    lv_style_init(&win_cont_style);
    //lv_style_set_bg_opa(&win_cont_style, 0);
    //lv_style_set_border_opa(&win_cont_style, 0);
    //lv_style_set_text_color(&win_cont_style, lv_color_hex(0xffffff));
    lv_style_set_pad_column(&win_cont_style, 0);
    lv_style_set_pad_row(&win_cont_style, 0);
    lv_style_set_flex_flow(&win_cont_style, LV_FLEX_FLOW_ROW);
    lv_style_set_pad_all(&win_cont_style, 0);
    lv_style_set_layout(&win_cont_style, LV_LAYOUT_FLEX);

    // 左侧菜单的风格样式
    static lv_style_t list_menu_style;
    lv_style_init(&list_menu_style);
    lv_style_set_bg_color(&list_menu_style, lv_color_hex(0x222d36));
    lv_style_set_radius(&list_menu_style, 0);
    lv_style_set_pad_row(&list_menu_style, 5);
    lv_style_set_pad_all(&list_menu_style, 0);
    lv_style_set_text_font(&list_menu_style, &lv_font_montserrat_20);
    //lv_style_set_text_opa(&list_menu_style, 0);

    // 左侧菜单按钮的风格样式
    static lv_style_t list_menu_btn;
    lv_style_init(&list_menu_btn);
    lv_style_set_bg_opa(&list_menu_btn, 0);
    lv_style_set_border_opa(&list_menu_btn, 0);
    lv_style_set_text_color(&list_menu_btn, lv_color_hex(0xffffff));

    // 右侧文件浏览区域的样式风格
    static lv_style_t file_list_style;
    lv_style_init(&file_list_style);
    //lv_style_set_bg_opa(&file_list_style, 0);
    //lv_style_set_border_opa(&file_list_style, 0);
    lv_style_set_pad_all(&file_list_style, 0);
    lv_style_set_radius(&file_list_style, 0);
    lv_style_set_text_font(&file_list_style, &lv_font_montserrat_20);
    //lv_style_set_text_color(&file_list_style, lv_color_hex(0xffffff));

    // 顶部按钮
    lv_obj_t * win = lv_win_create(lv_scr_act(), WIN_HEADER_HEIGHT);
    lv_obj_add_style(win, &win_sytle, 0);
    lv_obj_t * btn;
    btn = lv_win_add_btn(win, LV_SYMBOL_LEFT, WIN_BTN_WIDTH);
    lv_obj_add_style(btn, &win_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "..");

    btn = lv_win_add_btn(win, LV_SYMBOL_RIGHT, WIN_BTN_WIDTH);
    lv_obj_add_style(btn, &win_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, ".");

    btn = lv_win_add_btn(win, LV_SYMBOL_UP, WIN_BTN_WIDTH);
    lv_obj_add_style(btn, &win_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "..");

    btn = lv_win_add_btn(win, LV_SYMBOL_DOWN, WIN_BTN_WIDTH);
    lv_obj_add_style(btn, &win_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, ".");

    // 当前路径
    g_path_label = lv_win_add_title(win, "Path://");
    lv_label_set_text(g_path_label, "Path://www.100ask.net");

    // 展示区域
    lv_obj_t * win_cont = lv_win_get_content(win);  /*Content can be aded here*/
    lv_obj_add_style(win_cont, &win_cont_style, 0);

    // 左侧菜单
    lv_obj_t * list_menu = lv_list_create(win_cont);
    lv_obj_set_size(list_menu, LV_PCT(MENU_WIDTH), LV_PCT(100));
    lv_obj_add_style(list_menu, &list_menu_style, 0);

    lv_obj_set_style_bg_color(lv_list_add_text(list_menu, "DEVICE"), lv_palette_main(LV_PALETTE_ORANGE), 0);
    btn = lv_list_add_btn(list_menu, NULL, LV_SYMBOL_DRIVE" File System");
    lv_obj_add_style(btn, &list_menu_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "/");
    btn = lv_list_add_btn(list_menu, NULL, LV_SYMBOL_DRIVE" Mnt");
    lv_obj_add_style(btn, &list_menu_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "/mnt");

    lv_obj_set_style_bg_color(lv_list_add_text(list_menu, "PLACES"), lv_palette_main(LV_PALETTE_LIME), 0);
    btn = lv_list_add_btn(list_menu, NULL, LV_SYMBOL_HOME " HOME");
    lv_obj_add_style(btn, &list_menu_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "/root");
    btn = lv_list_add_btn(list_menu, NULL, LV_SYMBOL_AUDIO " Music");
    lv_obj_add_style(btn, &list_menu_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "/root/music");
    btn = lv_list_add_btn(list_menu, NULL, LV_SYMBOL_IMAGE " Pictures");
    lv_obj_add_style(btn, &list_menu_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "/root/pictures");
    btn = lv_list_add_btn(list_menu, NULL, LV_SYMBOL_TRASH " Trash");
    lv_obj_add_style(btn, &list_menu_btn, 0);
    lv_obj_add_event_cb(btn, click_menu_item_event_handler, LV_EVENT_CLICKED, "/root");

    /* 展示区域 */
    g_file_list = lv_list_create(win_cont);
    lv_obj_set_size(g_file_list, LV_PCT(SHOW_WIDTH), LV_PCT(100));
    lv_obj_add_style(g_file_list, &file_list_style, 0);
    lv_list_add_btn(g_file_list, LV_SYMBOL_GPS, "www.100ask.net");
    
    // 默认打开家目录
    //shell_opt_handle("pwd ~", file_browser_dir);
    show_dir("/root");
}

#endif /* LV_USE_100ASK_DEMO_FILE_MANAGER */
