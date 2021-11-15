#
# Makefile
#

include ./app_src.mk
include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/lv_lib_png/lv_lib_png.mk
include $(LVGL_DIR)/lv_100ask_modules/lv_100ask_modules.mk
include $(LVGL_DIR)/lv_100ask_demos/lv_100ask_demo.mk

BIN = general_Tiles


#Collect the files to compile
MAINSRC = ./main.c

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)
	mkdir -p obj
	mv *.o ./obj/
	mv $(BIN) $(LVGL_DIR)/bin/

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ) ./obj/*

