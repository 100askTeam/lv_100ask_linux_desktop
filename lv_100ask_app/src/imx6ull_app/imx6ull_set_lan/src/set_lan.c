/**
 ******************************************************************************
 * @file    set_lan.c
 * @author  百问科技
 * @version V1.0
 * @date    2021-11-03
 * @brief	有线(LAN)网络设置
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

#if 1

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "set_lan.h"

#include "lvgl/lvgl.h"

// LAN信息结构体
static struct Lan_info {
    char device[8];         // 网卡设备
	char cable[16];		    // 连接状态
	char dhcp[8];           // DHCPC模式
    char mac[24];           // MAC地址
    char ipaddr[16];        // IP地址
    char gateway[16];       // 网关
    char netmask[16];       // 子网掩码
} Lan_info = {"0", "0", "0", "0", "0", "0"};

static lv_obj_t * g_btn_run_dhcp;               // 刷新网卡句柄
static lv_obj_t * g_table_lan_info;             // LAN信息表格句柄
static lv_timer_t * g_wifi_scan_task_handle; 	// 定时器任务句柄

static bool get_lan_info(void);                                     // 获取LAN信息 
static void set_cell_value(lv_obj_t * table);                       // 展示LAN信息
static void run_dhcpc_event_handler(lv_event_t * e);                // DHCPC按钮回调函数
static void lv_task_lan_cable_scan(lv_timer_t * task);              // 定时扫描连接状态任务
static char * shell_exec_handle(const char * cmd, char * result);   // shell命令执行函数


static void run_dhcpc_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        //LV_LOG_USER("Clicked");
        get_lan_info();
        set_cell_value(g_table_lan_info);
    }
}


// 定时扫描连接状态
static void lv_task_lan_cable_scan(lv_timer_t * task)
{
    get_lan_info();
    set_cell_value(g_table_lan_info);
}


static char * shell_exec_handle(const char * cmd, char * result)
{
    FILE * fp;
    char buffer[128] = {0};

    fp=popen(cmd, "r");
    while (fgets(buffer, 128, fp) != NULL)
    {
        printf("%s", buffer); 
        strcat(result, buffer);
    }

    if (pclose(fp) == -1)
    {
        printf("close failed!");
        //return NULL;
    }
}


static bool get_lan_info(void)
{
    char cmd_buff[128];
    char result[128];

    // 检查网线连接状态
    memset(cmd_buff, 0, sizeof(cmd_buff));
    memset(result, 0, sizeof(result));
    lv_snprintf(cmd_buff, sizeof(cmd_buff), "ip link show | grep eth0: | awk '{print $9}'");
    shell_exec_handle(cmd_buff, result);
    lv_snprintf(Lan_info.cable, sizeof(Lan_info.cable), result);
    if (strstr(Lan_info.cable, "DOWN") != NULL)
    {
        memset(cmd_buff, 0, sizeof(cmd_buff));
        memset(result, 0, sizeof(result));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "ip link show | grep eth1: | awk '{print $9}'");
        shell_exec_handle(cmd_buff, result);
        lv_snprintf(Lan_info.cable, sizeof(Lan_info.cable), result);
        if (strstr(Lan_info.cable, "DOWN") != NULL)
        {
            LV_LOG_USER("Neither eth1 nor eth0 are connected!\n");
            memset(Lan_info.dhcp, 0, sizeof(Lan_info.dhcp));
            memset(Lan_info.mac, 0, sizeof(Lan_info.mac));
            memset(Lan_info.ipaddr, 0, sizeof(Lan_info.ipaddr));
            memset(Lan_info.gateway, 0, sizeof(Lan_info.gateway));
            memset(Lan_info.netmask, 0, sizeof(Lan_info.netmask));
            return 0;
        }
        else lv_snprintf(Lan_info.device, sizeof(Lan_info.device), "eth1");
    }
    else
    {
        lv_snprintf(Lan_info.device, sizeof(Lan_info.device), "eth0");
    }

    // 检查DHCP状态
    lv_snprintf(Lan_info.dhcp, sizeof(Lan_info.dhcp), "YES");

    // 获取MAC地址
    memset(cmd_buff, 0, sizeof(cmd_buff));
    memset(result, 0, sizeof(result));
    //lv_snprintf(cmd_buff, sizeof(cmd_buff), "ifconfig %s | grep \"ether \" | awk '{print $2}'", Lan_info.device);
    lv_snprintf(cmd_buff, sizeof(cmd_buff), "ifconfig %s | grep \"HWaddr \" | awk '{print $5}'", Lan_info.device); // ifconfig eth0 | grep "HWaddr " | awk '{print $5}'
    shell_exec_handle(cmd_buff, result);
    lv_snprintf(Lan_info.mac, sizeof(Lan_info.mac), result);

    // 获取IP地址
    memset(cmd_buff, 0, sizeof(cmd_buff));
    memset(result, 0, sizeof(result));
    //lv_snprintf(cmd_buff, sizeof(cmd_buff), "ifconfig %s | grep \"inet \" | awk '{print $2}'", Lan_info.device);
    lv_snprintf(cmd_buff, sizeof(cmd_buff), "ifconfig %s | grep \"inet \" | awk -F '[:]'  '{print $2}' | awk  '{print $1}'", Lan_info.device);  // ifconfig eth0 | grep "inet " | awk -F '[:]'  '{print $2}' | awk  '{print $1}'
    shell_exec_handle(cmd_buff, result);
    lv_snprintf(Lan_info.ipaddr, sizeof(Lan_info.ipaddr), result);
    
    // 获取网关
    memset(cmd_buff, 0, sizeof(cmd_buff));
    memset(result, 0, sizeof(result));
    lv_snprintf(cmd_buff, sizeof(cmd_buff), "ip route show  | grep \"default via \" | awk '{print $3}'");  // ip route show  | grep "default via " | awk '{print $3}'
    shell_exec_handle(cmd_buff, result);
    lv_snprintf(Lan_info.gateway, sizeof(Lan_info.gateway), result);

    // 获取子网掩码
    memset(cmd_buff, 0, sizeof(cmd_buff));
    memset(result, 0, sizeof(result));
    //lv_snprintf(cmd_buff, sizeof(cmd_buff), "ifconfig %s | grep \"netmask \" | awk '{print $4}'", Lan_info.device);  
    lv_snprintf(cmd_buff, sizeof(cmd_buff), "ifconfig %s | grep \"Mask\" | awk -F '[:]'  '{print $4}'", Lan_info.device);// ifconfig eth0 | grep "Mask" | awk -F '[:]'  '{print $4}'
    shell_exec_handle(cmd_buff, result);
    lv_snprintf(Lan_info.netmask, sizeof(Lan_info.netmask), result);

    memset(cmd_buff, 0, sizeof(cmd_buff));
    memset(result, 0, sizeof(result));
    
    return 1;
}


static void set_cell_value(lv_obj_t * table)
{
    // Fill the first column
    lv_table_set_cell_value(table, 0, 0, "Cable");
    lv_table_set_cell_value(table, 1, 0, "DHCP");
    lv_table_set_cell_value(table, 2, 0, "MAC");
    lv_table_set_cell_value(table, 3, 0, "IP-address");
    lv_table_set_cell_value(table, 4, 0, "Gateway");
    lv_table_set_cell_value(table, 5, 0, "Netmask");

    // Fill the second column
    lv_table_set_cell_value(table, 0, 1, "Connected "LV_SYMBOL_OK); // LV_SYMBOL_OK LV_SYMBOL_WARNING
    lv_table_set_cell_value(table, 1, 1, Lan_info.dhcp);
    lv_table_set_cell_value(table, 2, 1, Lan_info.mac);
    lv_table_set_cell_value(table, 3, 1, Lan_info.ipaddr);
    lv_table_set_cell_value(table, 4, 1, Lan_info.gateway);
    lv_table_set_cell_value(table, 5, 1, Lan_info.netmask);

    if (strstr(Lan_info.cable, "DOWN") != NULL)
    {
        lv_obj_add_state(g_btn_run_dhcp, LV_STATE_DISABLED|LV_STATE_CHECKED);
        lv_label_set_text(lv_obj_get_child(g_btn_run_dhcp, 0), LV_SYMBOL_WARNING" The network cable is not connected!");
        lv_table_set_cell_value(table, 0, 1, "Not Connected "LV_SYMBOL_WARNING);    // LV_SYMBOL_OK LV_SYMBOL_WARNING
    }
    else
    {
        lv_obj_clear_state(g_btn_run_dhcp, LV_STATE_DISABLED|LV_STATE_CHECKED);
        lv_label_set_text(lv_obj_get_child(g_btn_run_dhcp, 0), LV_SYMBOL_REFRESH" Retrieve IP address(DHCPC)");
    }
}

void imx6ull_set_lan_init(void)
{
    lv_obj_t * obj_layout;
    static lv_style_t obj_layout_style;   // 容器的样式

    get_lan_info();     // 先获取网络信息

    lv_style_init(&obj_layout_style);
    //lv_style_set_pad_all(&obj_layout_style, 0);
    //lv_style_set_bg_opa(&obj_layout_style, 0);
    //lv_style_set_shadow_opa(&obj_layout_style, 0);
    lv_style_set_pad_row(&obj_layout_style, 20);
    lv_style_set_border_opa(&obj_layout_style, 0);

    /* Layout Init */
    obj_layout = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj_layout,  LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(obj_layout, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(obj_layout, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(obj_layout);
    lv_obj_add_style(obj_layout, &obj_layout_style, 0);

    /* Create title */
    lv_obj_t * label_title = lv_label_create(obj_layout);
    lv_obj_set_width(label_title, LV_PCT(100));
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_align(label_title, LV_TEXT_ALIGN_CENTER, 0);
    //lv_label_set_recolor(label_title, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label_title, "LAN");
    lv_obj_align(label_title, LV_ALIGN_CENTER, 0, 0); //lv_obj_center(label_title);

    /* Get IP Address(DHCPC) Button */
    g_btn_run_dhcp = lv_btn_create(obj_layout);
    lv_obj_set_width(g_btn_run_dhcp, LV_PCT(100));
    lv_obj_add_event_cb(g_btn_run_dhcp, run_dhcpc_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label = lv_label_create(g_btn_run_dhcp);
    lv_label_set_text(label, LV_SYMBOL_REFRESH" Retrieve IP address(DHCPC)");
    lv_obj_center(label);

    /* NET info */
    g_table_lan_info = lv_table_create(obj_layout);
    lv_obj_set_width(g_table_lan_info, LV_PCT(100));
    lv_obj_set_style_pad_all(g_table_lan_info, 0, 0);
    lv_obj_set_style_border_opa(g_table_lan_info, 0, 0);
    //lv_obj_set_style_text_font(g_table_lan_info, &lv_font_simsun_16_cjk, 0);  // 中文

    lv_table_set_col_width(g_table_lan_info, 0, 770);  // 150
    lv_table_set_col_width(g_table_lan_info, 1, 200);  // 800

    // Fill the table
    set_cell_value(g_table_lan_info);

    /*Add an event callback to to apply some custom drawing*/
    //lv_obj_add_event_cb(table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    g_wifi_scan_task_handle = lv_timer_create(lv_task_lan_cable_scan, 5000, NULL);  // 10秒定时器任务
   
}

#endif /* LV_USE_100ASK_DEMO_GAME_2048 */

