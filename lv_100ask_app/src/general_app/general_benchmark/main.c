#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "lv_lib_png/lv_png.h"
#include "lv_100ask_modules/lv_100ask_modules.h"

#include "lv_demos/lv_demo.h"

#define DISP_BUF_SIZE (1024 * 600)

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 1024;
    disp_drv.ver_res    = 600;
    lv_disp_drv_register(&disp_drv);

	/* Linux input device init */
    evdev_init();
	
    /* Initialize and register a display input driver */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);      /*Basic initialization*/

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;   //lv_gesture_dir_t lv_indev_get_gesture_dir(const lv_indev_t * indev)
    lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv); 

    // 支持png
    lv_png_init();

    // Set Image Cache size
    lv_img_cache_set_size(2);
    
    // 调用进程间通信管理初始化函数
	lv_100ask_dbus_handler_init("net.ask100.lvgl.Benchmark", "/net/ask100/lvgl/Benchmark");

    // 让桌面暂停运行
    dbus_method_call("net.ask100.lvgl.Main", "/net/ask100/lvgl/Main", "net.ask100.lvgl.Main", "states", 0, 0);

    /*Create a Demo*/
    //music_player_init();
    lv_demo_benchmark();

    // 悬浮球控制初始化
    assistive_touch_init();

    while(1) {
        lv_task_handler();
        //lv_tick_inc(5*1000);
        usleep(5000);
    }
    return 0;
}


/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
