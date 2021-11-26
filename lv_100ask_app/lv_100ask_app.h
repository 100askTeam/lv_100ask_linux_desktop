/**
 * @file lv_100ask_app.h
 *
 */

#ifndef LV_100ASK_APP_H
#define LV_100ASK_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif

#if 0
#if defined(LV_100ASK_APP_PATH)
#define __LV_TO_STR_AUX(x) #x
#define __LV_TO_STR(x) __LV_TO_STR_AUX(x)
#include __LV_TO_STR(LV_100ASK_APP_PATH)
#undef __LV_TO_STR_AUX
#undef __LV_TO_STR
#elif defined(LV_100ASK_APP_INCLUDE_SIMPLE)
#include "lv_100ask_app_conf.h"
#else
#include "../lv_100ask_app_conf.h"
#endif
#endif

#include "src/general_app/general_app.h"
#include "src/stm32mp157_app/stm32mp157_app.h"
//#include "stm32mp157_app/stm32mp157_app.h"

/*********************
 *      DEFINES
 *********************/
/*Test  lvgl version*/
#if LV_VERSION_CHECK(8, 0, 0) == 0
#error "lv_100ask_app: Wrong lvgl version"
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_100ASK_APP_H */
