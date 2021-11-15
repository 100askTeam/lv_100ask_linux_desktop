/**
 ******************************************************************************
 * @file    set_wlan.c
 * @author  百问科技
 * @version V1.0
 * @date    2021-10-29
 * @brief	IMX6ULL屏幕背光设置
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2021-10-29     zhouyuebiao     First version
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
#include <string.h>
#include "set_wlan.h"

#include "lvgl/lvgl.h"
//#include "lv_100ask_modules/lv_100ask_modules.h"

#define USE_TIMER_UPDATE_LIST 0

static lv_obj_t * g_wifi_conn_info_panel;
static lv_obj_t * g_wifi_conn_list; // 已连接的WiFi列表
static lv_obj_t * g_wifi_scan_list; // 扫描到的wifi列表
static lv_obj_t * g_pwd_ta; // 密码框

static lv_timer_t * g_wifi_scan_task_handle; 	// 定时器任务句柄



static void wifi_list_event_handler(lv_event_t * e);

// 1 : 匹配到字符串  
// 0 ：匹配不到字符串
static bool shell_exec_handle(const char * cmd, const char * result)
{
    FILE * fp;
    char buffer[512] = {0};
    char *str_ret;
    bool ret = 0;

    fp = popen(cmd, "r");
    while (fgets(buffer, 512, fp) != NULL)
    {
        printf("%s", buffer); 
        if (result != NULL)
        {
            str_ret = strstr(buffer, result);
            if (str_ret != NULL)
            {
                printf("str_ret != NULL: %s\n", str_ret);
                ret = 1;
            }
        }
    }

    if (pclose(fp) == -1)
    {
        printf("close failed!");
        //exit(1); // return -1;
        ret = 0;
    }
    return ret;
}


static void shell_opt_handle(const char * cmd, char * result)
{
    FILE * fp;
    char buffer[512] = {0};
    //char str_ret_buff[1024] = {0};

    fp = popen(cmd, "r");
    while (fgets(buffer, 512, fp) != NULL)
    {
        printf("%s", buffer); 
        strcat(result, buffer);
    }

    if (pclose(fp) == -1)
    {
        printf("close failed!");
        //return NULL;
        //exit(1); // return -1;
    }
    //printf("[shell_opt_handle]str_ret_buff: %s\n", str_ret_buff);
    //return str_ret_buff;
}


#if 1
// 定时扫描热点任务
static void lv_task_wlan_scan(lv_timer_t * task)
{
    static int count = 0;
    lv_obj_t * btn;
    char res_buffer[1024] = {0};
    char *token;

    printf("\n\n------------------lv_task_wlan_scan---------------------\n\n");
    if (count > 0)
    {
        shell_opt_handle("iw dev wlan0 scan | grep SSID: | awk '{print $2}'", res_buffer);
        if (strlen(res_buffer) <= 1)
        {
            printf("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR]Buffer NULL!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        }
        
        lv_obj_clean(g_wifi_scan_list); // 清空列表，删除对象的所有子项（但不是对象本身）
        lv_list_add_text(g_wifi_scan_list, "OTHER NETWORKS");
        token = strtok(res_buffer, "\n");
        do{
            btn = lv_list_add_btn(g_wifi_scan_list, LV_SYMBOL_WIFI, token);
            lv_obj_add_event_cb(btn, wifi_list_event_handler, LV_EVENT_CLICKED, NULL);
        }while((token = strtok(NULL, "\n")));

        memset(res_buffer, 0, sizeof(res_buffer));   //清空字符串数组 
    }
    else
    {
        count = 2;
    }
}
#endif

// 1 : 正常  
// 0 ：异常
static bool open_wifi(void)
{
    if(shell_exec_handle("ifconfig wlan0 up", "SIOCGIFFLAGS") == 1)
        return 0;
#if USE_TIMER_UPDATE_LIST
    // 如果未创建则创建，如果已经创建过了直接恢复定时器
    if(g_wifi_scan_task_handle == NULL)
    {
        printf("\n\n------------------g_wifi_scan_task_handle---------------------\n\n");
        g_wifi_scan_task_handle = lv_timer_create(lv_task_wlan_scan, 20000, NULL);  // 10秒定时器任务
        if(g_wifi_scan_task_handle) lv_timer_ready(g_wifi_scan_task_handle);  // 就绪，马上开始
    }
    else
    {
        if(g_wifi_scan_task_handle) lv_timer_resume(g_wifi_scan_task_handle);
    }
#endif     
    return 1;
}


// wifi 开关
static void switch_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * btn = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        //printf("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if (lv_obj_has_state(obj, LV_STATE_CHECKED))
        {
            //open_wifi();
            if (1 == open_wifi());
            {
                lv_obj_clear_state(btn, LV_STATE_DISABLED);
                lv_event_send(btn, LV_EVENT_CLICKED, NULL);
            }
        }
        else
        {
            lv_obj_add_state(btn, LV_STATE_DISABLED);
#if USE_TIMER_UPDATE_LIST            
            if(g_wifi_scan_task_handle)
                lv_timer_pause(g_wifi_scan_task_handle);  // 暂停
            if (g_wifi_scan_list)
                lv_obj_clean(g_wifi_scan_list); // 清空列表，删除对象的所有子项（但不是对象本身）
#endif                
        }
    }
}



// 刷新wifi列表
static void refresh_wifi_list_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    //lv_obj_t * obj = lv_event_get_target(e);
    
    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * btn;
        char res_buffer[1024] = {0};
        char *token;

        if(g_wifi_scan_task_handle) lv_timer_pause(g_wifi_scan_task_handle);  // 暂停

        printf("\n\n------------------lv_task_wlan_scan---------------------\n\n");
        shell_opt_handle("iw dev wlan0 scan | grep SSID: | awk '{print $2}'", res_buffer);
        if (strlen(res_buffer) <= 1)
        {
            printf("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR]Buffer NULL!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
            if(g_wifi_scan_task_handle) lv_timer_resume(g_wifi_scan_task_handle);  // 恢复定时器任务
            return;
        }
        
        lv_obj_clean(g_wifi_scan_list); // 清空列表，删除对象的所有子项（但不是对象本身）
        lv_list_add_text(g_wifi_scan_list, "OTHER NETWORKS");
        token = strtok(res_buffer, "\n");
        do{
            btn = lv_list_add_btn(g_wifi_scan_list, LV_SYMBOL_WIFI, token);
            lv_obj_add_event_cb(btn, wifi_list_event_handler, LV_EVENT_CLICKED, NULL);
        }while((token = strtok(NULL, "\n")));

        if(g_wifi_scan_task_handle) lv_timer_resume(g_wifi_scan_task_handle);  // 恢复定时器任务
    }
}


static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);    // 获取事件代码
    lv_obj_t * ta = lv_event_get_target(e);         // 获取事件最初指向的对象
    lv_obj_t * kb = lv_event_get_user_data(e);      // 获取在对象上注册事件时传递的 user_data

    // 判断事件类型(当焦点在键盘上时)
    if(code == LV_EVENT_FOCUSED) {
        //获取输入设备的类型
        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);                                   // 为键盘分配一个文本区域
            lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 3, 0);             // 设置键盘的最高宽度为屏幕高度的一半
            lv_obj_align_to(g_wifi_conn_info_panel, kb, LV_ALIGN_OUT_TOP_MID, 0, 0);   // 设置对象的高度，当唤出键盘时应该偏移背景
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);                          // 清除标志
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);                   // 滚动到一个对象，直到它在其父对象上可见。对父母的父母做同样的事情，依此类推。即使对象具有嵌套的可滚​​动父对象，它也会滚动到视图中
        }
    }
    // 判断事件类型(当焦点不在键盘上时)
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_center(g_wifi_conn_info_panel);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    // 判断事件类型(当键盘点击确定或取消键)
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_center(g_wifi_conn_info_panel);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);            // 标志为隐藏对象(使对象隐藏，就像它根本不存在一样)
        lv_obj_clear_state(e->target, LV_STATE_FOCUSED);    // 删除对象的一种或多种状态。其他状态位将保持不变。
        lv_indev_reset(NULL, e->target);                    // 忘记最后一次点击的对象，使其再次成为可关注的对象
    }
}


// mode: 0: 连接历史wifi 1：连接新的wifi
static bool conn_wifi(const char * name, char * passwd, int mode)
{
    char cmd_buff[256];
    char res_buffer[512] = {0};
    int count = 0, overrun = 3;

    if (1 == mode)
    {
        // [0] 重新装载驱动 //TODO 
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "rmmod 8723bu.ko && modprobe 8723bu && ifconfig wlan0 up");
        shell_exec_handle(cmd_buff, "NULL");

        // [1] 配置连接参数
        memset(cmd_buff, 0, sizeof(cmd_buff));
        // rm /etc/wpa_supplicant.conf && wpa_passphrase Programmers 100asktech >> /etc/wpa_supplicant.conf
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "rm /etc/wpa_supplicant.conf && wpa_passphrase %s %s >> /etc/wpa_supplicant.conf", name, passwd);
        printf("%s\n", cmd_buff);
        shell_exec_handle(cmd_buff, "NULL");
        //sleep(1);
        
        // [2] 判断是否后台已经运行 wpa_supplicant (ps | grep "wpa_supplicant")
        //memset(cmd_buff, 0, sizeof(cmd_buff));
        //lv_snprintf(cmd_buff, sizeof(cmd_buff), "ps | grep \"wpa_supplicant\"", "wpa_supplicant");
        //shell_exec_handle(cmd_buff, NULL);
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "killall -9 wpa_supplicant");  // 连接之前先kill掉wpa_supplicant (这里不检查是否已运行wpa_supplicant)
        printf("%s\n", cmd_buff);
        shell_exec_handle(cmd_buff, "NULL");
        //sleep(1);
        
        // [3] 连接wifi设备
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "wpa_supplicant -B -iwlan0 -c /etc/wpa_supplicant.conf");
        printf("%s\n", cmd_buff);
        shell_exec_handle(cmd_buff, "NULL");
        //sleep(1);

        // [4] 查看连接状态
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "iw wlan0 link");
        while(shell_exec_handle(cmd_buff, "bitrate") == 1)
        {
            printf("iw wlan0 link: %s\n", cmd_buff);
            if (count >= 5)
            {
                memset(cmd_buff, 0, sizeof(cmd_buff));
                // rmmod 8723bu && insmod /lib/modules/4.9.88/extra/8723bu.ko && ifconfig wlan0 up
                lv_snprintf(cmd_buff, sizeof(cmd_buff), \
                    "rmmod 8723bu.ko && modprobe 8723bu &&\
                    ifconfig wlan0 up && killall wpa_supplicant &&\
                    wpa_supplicant -B -iwlan0 -c /etc/wpa_supplicant.conf && iw wlan0 link");
                
                count = 0;
                continue;
            }
            if(0 == overrun)
            {
                LV_LOG_USER("iw wlan0 link fail!");
                return 0;
            }

            memset(cmd_buff, 0, sizeof(cmd_buff));
            lv_snprintf(cmd_buff, sizeof(cmd_buff), "iw wlan0 link");
            count++;
            overrun--;
            //sleep(1);
        }

        // [5] 获取ip地址
        memset(cmd_buff, 0, sizeof(cmd_buff));
        printf("%s\n", cmd_buff);
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "udhcpc -i wlan0");
        if(1 == shell_exec_handle(cmd_buff, "adding dns"))
            printf("\n\nudhcpc -i wlan0 ok!\n\n");
        memset(cmd_buff, 0, sizeof(cmd_buff));

    }
    else if (0 == mode)
    {
        char ipaddr[16];
        char wifi_name[32];
        char btn_name[128];

        // 查看连接状态
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "ifconfig wlan0");
        //if (shell_exec_handle(cmd_buff, "Device not found") != 0)
        if (shell_exec_handle(cmd_buff, "information") == 0)
        {
            memset(wifi_name, 0, sizeof(wifi_name));
            memset(ipaddr, 0, sizeof(ipaddr));
            memset(btn_name, 0, sizeof(btn_name));
            shell_opt_handle("ifconfig wlan0 | grep \"inet \" | awk -F '[:]'  '{print $2}' | awk  '{print $1}'", ipaddr); // 获取IP地址
            shell_opt_handle("cat /etc/wpa_supplicant.conf | grep ssid | awk -F= '{print $2}' | awk -F '[\"]' '{print $2}'", wifi_name); // 获取wifi名称
            if((strlen(ipaddr) > 1) && (strlen(wifi_name) > 1))
            {
                ipaddr[strlen(ipaddr) - 1] = '\0';
                wifi_name[strlen(wifi_name) - 1] = '\0';
                lv_obj_clean(g_wifi_conn_list); // 清空列表，删除对象的所有子项（但不是对象本身）
                //shell_opt_handle("ifconfig wlan0 | grep \"inet \" | awk -F '[:]'  '{print $2}' | awk  '{print $1}'", ipaddr); // 获取IP地址
                //shell_opt_handle("cat /etc/wpa_supplicant.conf | grep ssid | awk -F= '{print $2}' | awk -F '[\"]' '{print $2}'", wifi_name); // 获取wifi名称
                lv_snprintf(btn_name, sizeof(btn_name), "%s  %s", wifi_name, ipaddr);
                lv_list_add_btn(g_wifi_conn_list, LV_SYMBOL_WIFI, btn_name);
                return 1;
            }
        }


        // [0] 重新装载驱动 //TODO 
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "rmmod 8723bu.ko && modprobe 8723bu && ifconfig wlan0 up && killall -9 wpa_supplicant");
        shell_exec_handle(cmd_buff, "NULL");

        // 连接wifi设备
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "wpa_supplicant -B -iwlan0 -c /etc/wpa_supplicant.conf");
        printf("%s\n", cmd_buff);
        shell_exec_handle(cmd_buff, "NULL");
        //sleep(1);

        // 查看连接状态
        memset(cmd_buff, 0, sizeof(cmd_buff));
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "iw wlan0 link");
        if (shell_exec_handle(cmd_buff, "bitrate") == 0)
        {
            LV_LOG_USER("iw wlan0 link fail!");
            return 0;
        }
        
        // [5] 获取ip地址
        memset(cmd_buff, 0, sizeof(cmd_buff));
        printf("%s\n", cmd_buff);
        lv_snprintf(cmd_buff, sizeof(cmd_buff), "udhcpc -i wlan0");
        if(1 == shell_exec_handle(cmd_buff, "adding dns"))
        {
            printf("\n\nudhcpc -i wlan0 ok!\n\n");
        }
        memset(cmd_buff, 0, sizeof(cmd_buff));
          
        memset(wifi_name, 0, sizeof(wifi_name));
        memset(ipaddr, 0, sizeof(ipaddr));
        memset(btn_name, 0, sizeof(btn_name));
        shell_opt_handle("ifconfig wlan0 | grep \"inet \" | awk -F '[:]'  '{print $2}' | awk  '{print $1}'", ipaddr); // 获取IP地址
        shell_opt_handle("cat /etc/wpa_supplicant.conf | grep ssid | awk -F= '{print $2}' | awk -F '[\"]' '{print $2}'", wifi_name); // 获取wifi名称
        if((strlen(ipaddr) > 1) && (strlen(wifi_name) > 1))
        {
            ipaddr[strlen(ipaddr) - 1] = '\0';
            wifi_name[strlen(wifi_name) - 1] = '\0';
            lv_obj_clean(g_wifi_conn_list); // 清空列表，删除对象的所有子项（但不是对象本身）
            lv_list_add_text(g_wifi_conn_list, "CONNECTED");
            lv_snprintf(btn_name, sizeof(btn_name), "%s  %s", wifi_name, ipaddr);
            lv_list_add_btn(g_wifi_conn_list, LV_SYMBOL_WIFI, btn_name);
        }
        
    }
    
    return 1;
}



static void conn_btn_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);    // 获取事件代码类型
    lv_obj_t * obj = lv_event_get_target(e);        // 获取触发的对象
    const char * wifi_name = lv_event_get_user_data(e);   // 获取在对象上注册事件时传递的 user_data

    if(code == LV_EVENT_CLICKED) {
        if(0 == strcmp(wifi_name, "back"))
        {
            lv_obj_del(g_wifi_conn_info_panel);                         // 删除对象及其所有子对象
            lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志
            lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度
            if(g_wifi_scan_task_handle) lv_timer_resume(g_wifi_scan_task_handle);  // 恢复定时器任务
            printf("Back\n");
        }
        else
        {            
            if(0 == conn_wifi(wifi_name, lv_textarea_get_text(g_pwd_ta), 1))
            {
                printf("[ERROR] Connetc fail!!!\n");
            }
            else
            {
                char ipaddr_buff[16] = {0};
                char btn_name[128] = {0};
                lv_obj_clean(g_wifi_conn_list); // 清空列表，删除对象的所有子项（但不是对象本身）
                lv_list_add_text(g_wifi_conn_list, "CONNECTED");
                //shell_opt_handle("ifconfig wlan0 | grep \"inet \" | awk '{print $2}'", ipaddr_buff); // 获取IP地址
                shell_opt_handle("ifconfig wlan0 | grep \"inet \" | awk -F '[:]'  '{print $2}' | awk  '{print $1}'", ipaddr_buff); // 获取IP地址
                ipaddr_buff[strlen(ipaddr_buff) - 1] = '\0';
                lv_snprintf(btn_name, sizeof(btn_name), "%s  %s", wifi_name, ipaddr_buff);
                lv_obj_t * btn = lv_list_add_btn(g_wifi_conn_list, LV_SYMBOL_WIFI, btn_name);
                lv_obj_add_event_cb(btn, wifi_list_event_handler, LV_EVENT_CLICKED, NULL);
                printf("wifi_name: %s, ipaddr_buff: %s\n\n\n\n" ,wifi_name ,ipaddr_buff);
            }
            lv_obj_del(g_wifi_conn_info_panel);                         // 删除对象及其所有子对象
            lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志
            lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度
            if(g_wifi_scan_task_handle) lv_timer_resume(g_wifi_scan_task_handle);  // 恢复定时器任务
            //lv_timer_resume(g_wifi_scan_task_handle);  // 恢复定时器任务
            printf("%s\n", wifi_name);

            //dbus_method_call("net.ask100.lvgl.Main", "/net/ask100/lvgl/Main", "net.ask100.lvgl.Main", "states", 1, getpid());
        }
    }
}


static void wifi_conn_info(char * wifi_name)
{
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);

    //  创建键盘
    lv_obj_t * kb = lv_keyboard_create(lv_layer_top());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);    // 标志为隐藏对象(使对象隐藏，就像它根本不存在一样)

    // 主区域布局容器
    g_wifi_conn_info_panel = lv_obj_create(lv_layer_top());
    lv_obj_set_size(g_wifi_conn_info_panel, LV_PCT(40), LV_PCT(40));
    lv_obj_set_layout(g_wifi_conn_info_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_align(g_wifi_conn_info_panel, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_flow(g_wifi_conn_info_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(g_wifi_conn_info_panel, 0, 0);
    lv_obj_center(g_wifi_conn_info_panel);

    // 输入区域布局容器
    lv_obj_t * input_panel = lv_obj_create(g_wifi_conn_info_panel);
    lv_obj_set_size(input_panel, 280, 100);
    lv_obj_set_style_bg_opa(input_panel, 0, 0);
    lv_obj_set_style_border_opa(input_panel, 0, 0);
    lv_obj_set_layout(input_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(input_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(input_panel, 0, 0);

    lv_obj_t * pwd_label = lv_label_create(input_panel);
    lv_label_set_text(pwd_label, "Password: ");                  // 为标签设置新文本

    g_pwd_ta = lv_textarea_create(input_panel);
    lv_textarea_set_one_line(g_pwd_ta, true);                       // 单行模式
    lv_textarea_set_password_mode(g_pwd_ta, true);                  // 密码模式
    //lv_textarea_set_placeholder_text(g_pwd_ta, "Please input wifi password...");    // 设置文本区域的占位符文本(输入框提示文本内容)
    lv_obj_align(g_pwd_ta, LV_ALIGN_CENTER, 0, 0);                // 更改对象的对齐方式并设置新坐标
    lv_obj_add_event_cb(g_pwd_ta, ta_event_cb, LV_EVENT_ALL, kb);   // 为对象分配事件及其处理函数

    pwd_label = lv_label_create(input_panel);
    lv_label_set_long_mode(pwd_label, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_style_text_font(pwd_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(pwd_label, "Please wait for 15s after clicking \"join\"...");                  // 为标签设置新文本

    // 选择域布局容器
    lv_obj_t * opt_panel = lv_obj_create(g_wifi_conn_info_panel);
    lv_obj_set_size(opt_panel, 280, 45);
    lv_obj_set_style_bg_opa(opt_panel, 0, 0);
    lv_obj_set_style_border_opa(opt_panel, 0, 0);
    lv_obj_set_layout(opt_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_align(opt_panel, LV_FLEX_ALIGN_SPACE_EVENLY  , LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_flow(opt_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(opt_panel, 0, 0);

    // 退出按钮，也可以点击空白区域退出
    lv_obj_t * btn_cancel = lv_btn_create(opt_panel);
    lv_obj_add_event_cb(btn_cancel, conn_btn_event_handler, LV_EVENT_CLICKED, "back");
    //lv_obj_add_event_cb(lv_layer_top(), conn_btn_event_handler, LV_EVENT_CLICKED, "back");

    // 连接wifi按钮
    lv_obj_t * btn_conn = lv_btn_create(opt_panel); 
    lv_obj_add_event_cb(btn_conn, conn_btn_event_handler, LV_EVENT_CLICKED, wifi_name);  // wifi_name

    lv_obj_t * label_conn = lv_label_create(btn_conn);
    lv_label_set_text(label_conn, "Join");
    lv_obj_center(label_conn);


    lv_obj_t * label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, "Back");
    lv_obj_center(label_cancel);
}



static void wifi_list_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_CLICKED) {
        printf("Clicked: %s\n", lv_list_get_btn_text(g_wifi_scan_list, obj));
        if (g_wifi_scan_task_handle != NULL)
            lv_timer_pause(g_wifi_scan_task_handle);  // 暂停
        wifi_conn_info(lv_list_get_btn_text(g_wifi_scan_list, obj));
    }
}


void imx6ull_set_wlan_init(void)
{
    lv_obj_t * obj_layout;
    lv_obj_t * ctr_btn_layout;
    static lv_style_t obj_layout_style;   // 容器的样式
    static lv_style_t ctr_btn_layout_style;   // 容器的样式

    lv_style_init(&obj_layout_style);
    //lv_style_set_pad_all(&obj_layout_style, 0);
    //lv_style_set_bg_opa(&obj_layout_style, 0);
    //lv_style_set_shadow_opa(&obj_layout_style, 0);
    lv_style_set_border_opa(&obj_layout_style, 0);

    lv_style_init(&ctr_btn_layout_style);
    lv_style_set_pad_all(&ctr_btn_layout_style, 0);
    //lv_style_set_bg_opa(&ctr_btn_layout_style, 0);
    //lv_style_set_shadow_opa(&ctr_btn_layout_style, 0);
    lv_style_set_border_opa(&ctr_btn_layout_style, 0);


    /* Layout Init */
    obj_layout = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj_layout,  LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(obj_layout, LV_LAYOUT_FLEX);
    lv_obj_set_style_base_dir(obj_layout, LV_BASE_DIR_RTL, 0);
    lv_obj_set_flex_flow(obj_layout, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(obj_layout);
    lv_obj_add_style(obj_layout, &obj_layout_style, 0);

    /* Create title */
    lv_obj_t * label_title = lv_label_create(obj_layout);
    lv_obj_set_width(label_title, LV_PCT(100));
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_align(label_title, LV_TEXT_ALIGN_CENTER, 0);
    //lv_label_set_recolor(label_title, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label_title, "Wi-Fi");
    lv_obj_align(label_title, LV_ALIGN_CENTER, 0, 0); //lv_obj_center(label_title);

    ctr_btn_layout = lv_obj_create(obj_layout);
    lv_obj_set_size(ctr_btn_layout,  LV_PCT(100), LV_PCT(7));
    lv_obj_set_layout(ctr_btn_layout, LV_LAYOUT_FLEX);
    lv_obj_set_style_base_dir(ctr_btn_layout, LV_BASE_DIR_LTR, 0);
    lv_obj_set_flex_flow(ctr_btn_layout, LV_FLEX_FLOW_ROW);
    lv_obj_center(ctr_btn_layout);
    lv_obj_add_style(ctr_btn_layout, &ctr_btn_layout_style, 0);

    /* Create wifi switch */
    lv_obj_t * sw = lv_switch_create(ctr_btn_layout); 

    lv_obj_t * btn1 = lv_btn_create(ctr_btn_layout);
    lv_obj_add_state(btn1, LV_STATE_DISABLED);
    lv_obj_add_event_cb(btn1, refresh_wifi_list_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label = lv_label_create(btn1);
    lv_label_set_text(label, LV_SYMBOL_REFRESH" Refresh");
    lv_obj_center(label);

    lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_ALL, btn1);

    /*Create wifi connected list*/
    g_wifi_conn_list = lv_list_create(obj_layout);
    //lv_obj_set_style_text_align(g_wifi_conn_list, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_size(g_wifi_conn_list, LV_PCT(100), LV_PCT(12));
    lv_obj_center(g_wifi_conn_list);

    /*Add buttons to the list*/
    lv_list_add_text(g_wifi_conn_list, "CONNECTED");
    //lv_obj_t * btn = lv_list_add_btn(g_wifi_conn_list, LV_SYMBOL_SETTINGS "  " LV_SYMBOL_WIFI, "www.100ask.net");
    lv_obj_t * btn = lv_list_add_btn(g_wifi_conn_list, LV_SYMBOL_WIFI, "www.100ask.net");
    lv_obj_add_event_cb(btn, wifi_list_event_handler, LV_EVENT_CLICKED, NULL);


    /*Create wifi scan list*/
    g_wifi_scan_list = lv_list_create(obj_layout);
    //lv_obj_set_style_text_align(g_wifi_scan_list, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_size(g_wifi_scan_list, LV_PCT(100), LV_PCT(68));
    lv_obj_center(g_wifi_scan_list);

    lv_list_add_text(g_wifi_scan_list, "OTHER NETWORKS");

#if 1
    if(shell_exec_handle("ifconfig wlan0", "Device not found") == 0)
    {
        open_wifi();
        if(conn_wifi(NULL, NULL, 0))
        {
            lv_obj_add_state(sw, LV_STATE_CHECKED);
            lv_obj_clear_state(btn1, LV_STATE_DISABLED);
            lv_event_send(btn1, LV_EVENT_CLICKED, NULL);
        }
    }
#endif 
}



#endif /* LV_USE_100ASK_DEMO_GAME_2048 */
