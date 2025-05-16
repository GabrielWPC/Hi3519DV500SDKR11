ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

INIT_FILE := ../../../init/$(OSTYPE)/src/piris_init.c
INIT_INC := ../../../init/$(OSTYPE)/include/ot_piris_mod_init.h
SRCS = piris.c
SRCS += $(INIT_FILE)
SRCS += $(ARCH_DIR)/piris_hal.c

INC := -I$(ISP_ROOT)/ext_inc
INC += -I$(ISP_ROOT)/kernel/mkp/include
INC += -I$(ISP_ROOT)/kernel/arch/$(OT_ARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/arch/$(OT_ARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/include
INC += -I$(ISP_ROOT)/ext_drv/piris/$(ARCH_DIR)/
INC += -I$(DRV_ROOT)/extdrv/$(EXTDRVVER)/piris/arch/$(ARCH_DIR)/
INC += -I$(CBB_ROOT)/init/$(OSTYPE)/include
EXTRA_CFLAGS += $(INC)

#*************************************************************************
TARGET := $(KO_PREFIX)_piris
EXTDRV_KO := y

#*************************************************************************
# compile linux or liteos
include $(MAKE_DRV_FILE)
