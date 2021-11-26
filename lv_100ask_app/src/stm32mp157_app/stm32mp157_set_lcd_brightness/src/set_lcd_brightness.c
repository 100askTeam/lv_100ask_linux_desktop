/**
 ******************************************************************************
 * @file    set_lcd_brightness.c
 * @author  百问科技
 * @version V1.0
 * @date    2021-11-25
 * @brief	IMX6ULL屏幕背光设置
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
#include "set_lcd_brightness.h"

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

/*********************
 *      DEFINES
 *********************/

static void backlight_control(int32_t brightness)
{
        char ch_led_state[2];
        char * led_brightness = "/sys/class/backlight/panel-backlight/brightness";;
        FILE *p_led_control = NULL;

        if ((p_led_control = fopen(led_brightness, "r+")) != NULL)
        {
                // The specified LED's brightness file is open for reading and writing.
                // Convert led_state to a char, write the result to the brightness file,
                // and close the file.

                lv_snprintf(ch_led_state, sizeof(ch_led_state), "%d", brightness);
                fwrite(ch_led_state, sizeof(char), 1, p_led_control);
                fclose(p_led_control);
        }
}


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
static char get_backlight_brightenss(void)
{
        char ch_led_state[2];
        char * led_brightness = "/sys/class/backlight/panel-backlight/brightness";;
        FILE *p_led_control = NULL;

        if ((p_led_control = fopen(led_brightness, "r+")) != NULL)
        {
                size_t ret = fread(ch_led_state, sizeof(*ch_led_state), ARRAY_SIZE(ch_led_state), p_led_control);
                if (ret != ARRAY_SIZE(ch_led_state)) {
               fprintf(stderr, "fread() failed: %zu\n", ret);
               exit(EXIT_FAILURE);
           }
                fclose(p_led_control);
        }

        printf("%c\n", ch_led_state[0]);

        return ch_led_state[0];
}



#if 0
static lv_obj_t * slider_label;
static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    backlight_control(lv_slider_get_value(slider));
}
#endif


static void slider_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t *s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    } else if(code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(obj));

            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, &lv_font_montserrat_20, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(&bg_area, dsc->clip_area, &rect_dsc);

            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = &lv_font_montserrat_20;
            lv_draw_label(&txt_area, dsc->clip_area, &label_dsc, buf, NULL);

            backlight_control(lv_slider_get_value(obj));
        }
    }
}


void set_lcd_brightness_init(void)
{
    /*Create a transition*/
    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
    static lv_style_transition_dsc_t transition_dsc;
    lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);

    static lv_style_t style_main;
    static lv_style_t style_indicator;
    static lv_style_t style_knob;
    static lv_style_t style_pressed_color;
    lv_style_init(&style_main);
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

    lv_style_init(&style_indicator);
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&style_indicator, &transition_dsc);

    lv_style_init(&style_knob);
    lv_style_set_bg_opa(&style_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&style_knob, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_border_color(&style_knob, lv_palette_darken(LV_PALETTE_ORANGE, 3));
    lv_style_set_border_width(&style_knob, 2);
    lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&style_knob, 6); /*Makes the knob larger*/
    lv_style_set_transition(&style_knob, &transition_dsc);

    lv_style_init(&style_pressed_color);
    lv_style_set_bg_color(&style_pressed_color, lv_palette_darken(LV_PALETTE_ORANGE, 2));

    /*Create a slider and add the style*/
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_obj_remove_style_all(slider);        /*Remove the styles coming from the theme*/
    //lv_obj_set_size(slider, (LV_HOR_RES / 4 * 3), (LV_VER_RES / 20));
    lv_obj_set_width(slider, LV_PCT(60));  // lv_pct(x) can be used to set the size and position in percentage
    lv_bar_set_range(slider, 0, 2);   // 最大亮度等级设置
    lv_slider_set_value(slider, (get_backlight_brightenss() - '0'), LV_ANIM_ON);
    //lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_add_style(slider, &style_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED);

    lv_obj_center(slider);

    lv_obj_t * title_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_40, 0);
    lv_label_set_text(title_label, "BRIGHTNESS");
    lv_obj_align_to(title_label, slider, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
}


#endif /* LV_USE_100ASK_DEMO_GAME_2048 */
