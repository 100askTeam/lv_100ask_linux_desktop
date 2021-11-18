#
# Makefile
#
# ⭐全速编译命令： make clean && make -j$(nproc) RUN_JOBS=-j$(nproc)

#CC ?= gcc
export CC := arm-buildroot-linux-gnueabihf-gcc
export LVGL_DIR_NAME ?= lvgl
export LVGL_DIR ?= ${shell pwd}
export CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/  -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare \
	-I ~/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot/usr/include -I ~/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot/usr/lib/dbus-1.0/include
export LDFLAGS ?= -lm -ldbus-1 -lpthread
BIN = 100ask_lvgl_Main


# 从make传递的参数获取子应用的编译速度设置，示例：make -j16 RUN_JOBS=-j16
RUN_JOBS = 

#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/lv_lib_png/lv_lib_png.mk
include $(LVGL_DIR)/lv_100ask_modules/lv_100ask_modules.mk
#include $(LVGL_DIR)/lv_100ask_app/lv_100ask_app.mk


OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES
SRC_DIR  +=  lv_100ask_app/src/general_app/general_file_manager\
			 lv_100ask_app/src/general_app/general_2048_game\
			 lv_100ask_app/src/general_app/general_game_memory\
			 lv_100ask_app/src/general_app/general_game_snake\
			 lv_100ask_app/src/general_app/general_game_tiles\
			 lv_100ask_app/src/general_app/general_calc\
			 lv_100ask_app/src/general_app/general_widgets\
			 lv_100ask_app/src/general_app/general_benchmark\
			 lv_100ask_app/src/general_app/general_music_player\
			 lv_100ask_app/src/general_app/general_about\
			 lv_100ask_app/src/imx6ull_app/imx6ull_set_lcd_brightness\
			 lv_100ask_app/src/imx6ull_app/imx6ull_set_time\
			 lv_100ask_app/src/imx6ull_app/imx6ull_set_wlan\
			 lv_100ask_app/src/imx6ull_app/imx6ull_set_lan

all: default
	$(foreach  dir,$(SRC_DIR),make $(RUN_JOBS) -C $(dir);)

%.o: %.c
	$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)
	mkdir -p $(LVGL_DIR)/obj $(LVGL_DIR)/bin
	mv *.o $(LVGL_DIR)/obj/
	mv $(BIN) $(LVGL_DIR)/bin/

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ) ./bin/* ./obj/*
	$(foreach  dir,$(SRC_DIR),make -C $(dir) clean;)

