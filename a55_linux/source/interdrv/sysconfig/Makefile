ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

TARGET := sys_config
#*************************************************************************
# all source file in this module
SRCS := sys_cfg.c \
        clk_cfg.c \
        sys_ctrl.c \
        pin_mux.c
#*************************************************************************
EXTRA_CFLAGS += -I./

ifeq ($(OSTYPE),liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif
EXTRA_CFLAGS += -D$(OT_FPGA)

include $(MAKE_DRV_FILE)
