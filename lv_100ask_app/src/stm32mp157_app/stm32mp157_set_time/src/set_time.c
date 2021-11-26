/**
 ******************************************************************************
 * @file    set_time.c
 * @author  百问科技
 * @version V1.0
 * @date    2021-11-25
 * @brief	时间设置
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2021-11-25     zhouyuebiao     First version
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

#if 1 //LV_USE_100ASK_DEMO_GAME_2048

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "set_time.h"

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

/*********************
 *      DEFINES
 *********************/
#include <time.h>

LV_IMG_DECLARE(watch_bg);
LV_IMG_DECLARE(hour);
LV_IMG_DECLARE(minute);
LV_IMG_DECLARE(second);

lv_obj_t * lvMinute;
lv_obj_t * lvHour;
lv_obj_t * lvSecond ;

lv_obj_t * g_obj_hour;
lv_obj_t * g_obj_minute;

uint8_t Minute = 24;
uint8_t Hour = 8;
uint8_t Second= 2 ;


///////////////////////////// 日历
static lv_obj_t * g_calendar;

static void date_event_cb(lv_event_t * e);
static void calendar_event_cb(lv_event_t * e);


static void update_time_value(void)
{
    time_t rawtime;
    struct tm *info;

    time( &rawtime );
    info = localtime( &rawtime );

    Hour = info->tm_hour;
    Minute = info->tm_min;
    Second = info->tm_sec;

    lv_img_set_angle(lvHour, (Hour * 300 + Minute / 12 % 12 * 60)); // Set Hour angle
    lv_img_set_angle(lvMinute, (Minute*60));                        // Set Minute angles
    lv_img_set_angle(lvSecond, (Second * 60));                      // Set second angle
}

static void update_time(void *arg)
{
    char buf[4];
    char strftime_buf[64];

    if(Second++>60)
    {
       Second=0;
        if(Minute++>60)
        {
            Minute=0;
           if(Hour++>12) Hour=0;
        }
    }
    if(lvHour !=NULL)
    {

       // Hour,Minute,Second;
        lv_img_set_angle(  lvHour, Hour*30*10);
        lv_img_set_angle(  lvMinute, Minute*6*10);
        lv_img_set_angle(  lvSecond, Second*6*10);
    }

    lv_snprintf(buf, sizeof(buf), "%02d", Hour);
    lv_textarea_set_text(g_obj_hour, buf);  // 在文本区域展示日期信息

    lv_snprintf(buf, sizeof(buf), "%02d", Minute);
    lv_textarea_set_text(g_obj_minute, buf);  // 在文本区域展示日期信息
}


void analog(lv_obj_t *parent)
{
    lv_obj_t * img = lv_img_create(parent);
    //lv_obj_center(img);
    lv_img_set_src(img, &watch_bg);
    lv_obj_set_size(img, 200, 200);
    lv_obj_set_align(img, LV_ALIGN_CENTER);

    // Hour
    lvHour = lv_img_create(img);
    lv_img_set_src(lvHour, &hour);
    lv_obj_set_align(lvHour, LV_ALIGN_CENTER);
    lv_img_set_angle(lvHour, (Hour * 300 + Minute / 12 % 12 * 60)); // Set Hour angle

    // Minute
    lvMinute = lv_img_create(img);
    lv_img_set_src(lvMinute, &minute);
    lv_obj_set_align(lvMinute, LV_ALIGN_CENTER);
    lv_img_set_angle(lvMinute, (Minute*60));                        // Set Minute angle

    // Second
    lvSecond = lv_img_create(img);
    lv_img_set_src( lvSecond, &second);
    lv_obj_set_align(lvSecond, LV_ALIGN_CENTER);
    lv_img_set_angle(lvSecond, (Second * 60));                      // Set second angle

    lv_timer_t * timer = lv_timer_create(update_time, 1000, NULL);  // Update clock timer
    lv_timer_ready(timer);
}


// 获取时分秒命令，安度可单独获取：时、分、秒
// date -d today +%Y%m%d%H%M%S
static void clock_init(lv_obj_t *parent)
{
    time_t rawtime;
    struct tm *info;

    time( &rawtime );
    info = localtime( &rawtime );

    Hour = info->tm_hour;
    Minute = info->tm_min;
    Second = info->tm_sec;

#if 0
    lv_obj_t * tim = lv_obj_create(lv_scr_act());

    lv_obj_add_flag(tim, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_size(tim, LV_PCT(100), LV_PCT(100));
    lv_obj_center(tim);
    //lv_obj_set_height(tim, LV_PCT(100));
    //lv_obj_set_style_max_height(tim, 300, 0);

    lv_obj_set_flex_flow(tim, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(tim, 1);
    lv_obj_add_flag(tim, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
#endif

    analog(parent);
}


static void date_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER) {
            if(g_calendar == NULL) {
                // 获取系统时间
                time_t rawtime;
                struct tm *info;
                time( &rawtime );
                info = localtime( &rawtime );

                lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);                             // 使能 lv_layer_top 点击
                g_calendar = lv_calendar_create(lv_layer_top());                                      // 在 lv_layer_top 层上创建日历对象
                lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);                              // 设置对象透明度
                lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0);     // 设置对象颜色
                lv_obj_set_size(g_calendar, 300, 300);                                                // 设置对象大小
                lv_calendar_set_today_date(g_calendar, (info->tm_year + 2000 - 100), (info->tm_mon + 1), info->tm_mday);   // 设置今天的日期
                lv_calendar_set_showed_date(g_calendar, (info->tm_year + 2000 - 100), (info->tm_mon + 1));                 // 给日历的指定打开时显示的日期
                lv_obj_align(g_calendar, LV_ALIGN_CENTER, 0, 30);                                     // 设置对象对齐、偏移
                lv_obj_add_event_cb(g_calendar, calendar_event_cb, LV_EVENT_VALUE_CHANGED, ta);       // 给对象分配事件
                lv_obj_add_event_cb(lv_layer_top(), calendar_event_cb, LV_EVENT_CLICKED, ta);     // 给对象分配事件(lv_layer_top层分配点击回调处理函数)

                lv_calendar_header_dropdown_create(g_calendar);     // 创建一个包含 2 个下拉列表的标题：一个用于年份，另一个用于月份。
            }
        }
    }
}


static void calendar_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);    // 获取事件代码
    lv_obj_t * ta = lv_event_get_user_data(e);      // 获取在对象上注册事件时传递的 user_data
    lv_obj_t * obj = lv_event_get_current_target(e);

    // 判断事件类型
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        char cmd_buf[64];
        lv_calendar_date_t d;
        lv_calendar_get_pressed_date(obj, &d);        // 获取当前选中的日期

        memset(buf, 0, sizeof(buf));
        lv_snprintf(buf, sizeof(buf), "%02d-%02d-%d", d.year, d.month, d.day);
        lv_textarea_set_text(ta, buf);  // 在文本区域展示日期信息

        lv_snprintf(cmd_buf, sizeof(cmd_buf), "date -s %s && hwclock -w && hwclock -w", buf);
        printf("%s\n", cmd_buf);
        system(cmd_buf);  // TODO
        update_time_value();

        lv_obj_del(g_calendar);           // 删除对象及其所有子对象
        lv_obj_remove_event_cb(lv_layer_top(), calendar_event_cb);   // 删除对象的事件处理函数(lv_layer_top层)
        g_calendar = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志(lv_layer_top层)
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度(lv_layer_top层)
    }
    else if ((code == LV_EVENT_CLICKED))
    {
        lv_obj_del(g_calendar);           // 删除对象及其所有子对象
        lv_obj_remove_event_cb(lv_layer_top(), calendar_event_cb);   // 删除对象的事件处理函数(lv_layer_top层)
        g_calendar = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志(lv_layer_top层)
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度(lv_layer_top层)
    }
}


// 日期
static void date_init(lv_obj_t *parent)
{
    // 获取系统时间
    char buf[32];
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);

    // 日期标题
    lv_obj_t * date_label = lv_label_create(parent);
    lv_obj_set_width(date_label, 250);              // lv_obj_get_width(parent)
    lv_label_set_text(date_label, "DATE");                  // 为标签设置新文本
    lv_obj_set_style_text_opa(date_label, LV_OPA_50, 0);

    lv_obj_t * date = lv_textarea_create(parent);
    lv_textarea_set_one_line(date, true);                      // 单行模式
    lv_snprintf(buf, sizeof(buf), "%02d-%02d-%d", (info->tm_year + 2000 - 100), (info->tm_mon + 1), info->tm_mday);
    lv_textarea_set_text(date, buf);  // 在文本区域展示日期信息
    //lv_obj_align(date, LV_ALIGN_TOP_MID, 0, 120);              // 更改对象的对齐方式并设置新坐标
    lv_obj_add_event_cb(date, date_event_cb, LV_EVENT_ALL, NULL);  // 为对象分配事件及其处理函数

    //lv_obj_align_to(date_label, date, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
}

// 时
static void set_hour_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);    // 获取事件代码
    lv_obj_t * obj = lv_event_get_target(e);        // 获取触发事件的对象
    lv_obj_t * ta = lv_event_get_user_data(e);      // 获取在对象上注册事件时传递的 user_data

    // 判断事件类型
    if(code == LV_EVENT_SHORT_CLICKED) {
        char hour_buf[4];
        char cmd_buf[64];
        lv_roller_get_selected_str(obj, hour_buf, sizeof(hour_buf));        // 获取当前选中的时间(hour)

        lv_textarea_set_text(ta, hour_buf);  // 在文本区域展示日期信息

        lv_obj_del(obj);           // 删除对象及其所有子对象
        lv_obj_remove_event_cb(lv_layer_top(), set_hour_event_cb);   // 删除对象的事件处理函数(lv_layer_top层)
        obj = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志(lv_layer_top层)
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度(lv_layer_top层)

        lv_snprintf(cmd_buf, sizeof(cmd_buf), "date -s %s:00:01 && hwclock -w && hwclock -w", hour_buf);
        printf("%s\n", cmd_buf);
        system(cmd_buf);
        update_time_value();
    }
    else if ((code == LV_EVENT_CLICKED))
    {
        lv_obj_del(obj);           // 删除对象及其所有子对象
        lv_obj_remove_event_cb(lv_layer_top(), set_hour_event_cb);   // 删除对象的事件处理函数(lv_layer_top层)
        obj = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志(lv_layer_top层)
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度(lv_layer_top层)
    }

}


static void opt_hour_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER) {
            if(g_calendar == NULL) {
                const char * opts = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24";
                uint16_t selected = atoi(lv_label_get_text(lv_obj_get_child(ta, 0))) - 1; // 获取当前时间，并设为默认选中项

                // 制造对话框效果
                lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);
                lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0);

                lv_obj_t *roller= lv_roller_create(lv_layer_top());
                lv_roller_set_options(roller, opts, LV_ROLLER_MODE_NORMAL); // 设置选中的选项 TODO
                lv_roller_set_visible_row_count(roller, 8);
                lv_obj_set_width(roller, 80);
                lv_obj_set_style_text_font(roller, &lv_font_montserrat_22, 0);
                lv_obj_align_to(roller, ta, LV_ALIGN_CENTER, 0, 0);
                lv_roller_set_selected(roller, selected, LV_ANIM_OFF);

                lv_obj_add_event_cb(roller, set_hour_event_cb, LV_EVENT_SHORT_CLICKED, ta);       // 给对象分配事件
                lv_obj_add_event_cb(lv_layer_top(), set_hour_event_cb, LV_EVENT_CLICKED, ta);     // 给对象分配事件(lv_layer_top层分配点击回调处理函数)

            }
        }
    }
}


// 分
static void set_minute_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);    // 获取事件代码
    lv_obj_t * obj = lv_event_get_target(e);        // 获取触发事件的对象
    lv_obj_t * ta = lv_event_get_user_data(e);      // 获取在对象上注册事件时传递的 user_data

    // 判断事件类型
    if(code == LV_EVENT_SHORT_CLICKED) {
        char min_buf[4];
        char cmd_buf[64];
        lv_roller_get_selected_str(obj, min_buf, sizeof(min_buf));        // 获取当前选中的时间(hour)

        lv_textarea_set_text(ta, min_buf);  // 在文本区域展示日期信息

        lv_obj_del(obj);           // 删除对象及其所有子对象
        lv_obj_remove_event_cb(lv_layer_top(), set_minute_event_cb);   // 删除对象的事件处理函数(lv_layer_top层)
        obj = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志(lv_layer_top层)
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度(lv_layer_top层)

        lv_snprintf(cmd_buf, sizeof(cmd_buf), "date -s 01:%s:01 && hwclock -w && hwclock -w", min_buf);
        printf("%s\n", cmd_buf);
        system(cmd_buf);
        update_time_value();
    }
    else if ((code == LV_EVENT_CLICKED))
    {
        lv_obj_del(obj);           // 删除对象及其所有子对象
        lv_obj_remove_event_cb(lv_layer_top(), set_minute_event_cb);   // 删除对象的事件处理函数(lv_layer_top层)
        obj = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志(lv_layer_top层)
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度(lv_layer_top层)
    }

}

static void opt_minute_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER) {
            if(g_calendar == NULL) {
                const char * opts = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n\
                                     21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n\
                                     41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59";
                uint16_t selected = atoi(lv_label_get_text(lv_obj_get_child(ta, 0))) - 1; // 获取当前时间，并设为默认选中项

                // 制造对话框效果
                lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);
                lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0);

                lv_obj_t *roller= lv_roller_create(lv_layer_top());
                lv_roller_set_options(roller, opts, LV_ROLLER_MODE_NORMAL); // 设置选中的选项 TODO
                lv_roller_set_visible_row_count(roller, 8);
                lv_obj_set_width(roller, 80);
                lv_obj_set_style_text_font(roller, &lv_font_montserrat_22, 0);
                lv_obj_align_to(roller, ta, LV_ALIGN_CENTER, 0, 0);
                lv_roller_set_selected(roller, selected, LV_ANIM_OFF);

                lv_obj_add_event_cb(roller, set_minute_event_cb, LV_EVENT_SHORT_CLICKED, ta);       // 给对象分配事件
                lv_obj_add_event_cb(lv_layer_top(), set_minute_event_cb, LV_EVENT_CLICKED, ta);     // 给对象分配事件(lv_layer_top层分配点击回调处理函数)

            }
        }
    }
}

// 时间
static void time_init(lv_obj_t *parent)
{
    // 获取系统时间
    char buf[32];
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);

    // 时标题
    lv_obj_t * hour_label = lv_label_create(parent);
    lv_obj_set_width(hour_label, 250);              // lv_obj_get_width(parent)
    lv_label_set_text(hour_label, "HOUR");                  // 为标签设置新文本
    lv_obj_set_style_text_opa(hour_label, LV_OPA_50, 0);

    g_obj_hour = lv_textarea_create(parent);
    lv_textarea_set_one_line(g_obj_hour, true);                      // 单行模式
    lv_snprintf(buf, sizeof(buf), "%02d", info->tm_hour);
    lv_textarea_set_text(g_obj_hour, buf);  // 在文本区域展示日期信息
    lv_obj_add_event_cb(g_obj_hour, opt_hour_event_cb, LV_EVENT_ALL, NULL);  // 为对象分配事件及其处理函数

    // 分标题
    lv_obj_t * minute_label = lv_label_create(parent);
    lv_obj_set_width(minute_label, 250);              // lv_obj_get_width(parent)
    lv_label_set_text(minute_label, "MINUTE");                  // 为标签设置新文本
    lv_obj_set_style_text_opa(minute_label, LV_OPA_50, 0);

    g_obj_minute = lv_textarea_create(parent);
    lv_textarea_set_one_line(g_obj_minute, true);                      // 单行模式
    lv_snprintf(buf, sizeof(buf), "%02d", info->tm_min);
    lv_textarea_set_text(g_obj_minute, buf);  // 在文本区域展示日期信息
    lv_obj_add_event_cb(g_obj_minute, opt_minute_event_cb, LV_EVENT_ALL, NULL);  // 为对象分配事件及其处理函数
}



// 布局初始化
static lv_obj_t * layout_init(void)
{
    lv_obj_t * layout_obj;
    static lv_style_t cont_style;   // 容器的样式

    /* 设置容器的样式 */
        lv_style_init(&cont_style);
        lv_style_set_flex_flow(&cont_style, LV_FLEX_FLOW_ROW_WRAP);
        lv_style_set_flex_main_place(&cont_style, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_style_set_layout(&cont_style, LV_LAYOUT_FLEX);
    //lv_style_set_bg_opa(&cont_style, 0);
    lv_style_set_border_opa(&cont_style, 0);

    layout_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(layout_obj, 500, 500);
    lv_obj_center(layout_obj);
    lv_obj_add_style(layout_obj, &cont_style, 0);

    return layout_obj;
}


void set_time_init(void)
{
    lv_obj_t * layout_obj = layout_init();
    clock_init(layout_obj);
    date_init(layout_obj);
    time_init(layout_obj);
}


#endif /* LV_USE_100ASK_DEMO_GAME_2048 */
