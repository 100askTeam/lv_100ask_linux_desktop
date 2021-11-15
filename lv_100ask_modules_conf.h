/**
 * @file lv_100ask_modules_conf.h
 * Configuration file for v1.0.0
 *
 */
/*
 * COPY THIS FILE AS lv_100ask_modules_conf.h
 */

/* clang-format off */
#if 1 /*Set it to "1" to enable the content*/

#ifndef LV_100ASK_MODULES_CONF_H
#define LV_100ASK_MODULES_CONF_H


/*******************
 * GENERAL SETTING
 *******************/

/*********************
 * DEMO USAGE
 *********************/

/* [General] APP通用初始化组件 */
#define LV_USE_100ASK_APP_GENERAL_TEMPLATE  0

/* [General] 通用DBus 通信组件 */
#define LV_USE_100ASK_DBUS_HANDLER          1

/* [General] APP通用辅助触控组件 */
#define LV_USE_100ASK_DEMO_ASSISTIVE_TOUCH  1

/* [Main desktop usage] 开机动画 */
#define LV_USE_100ASK_BOOT_ANIMATION        1

/* [Main desktop usage] 主桌面初始化组件 */
#define LV_USE_100ASK_DEMO_INIT_ICON        1

/* [Main desktop usage] 主桌面消息分发组件 */
#define LV_USE_100ASK_DEMO_MSG_DISP         1


#endif /*LV_100ASK_MODULES_CONF_H*/

#endif /*End of "Content enable"*/

