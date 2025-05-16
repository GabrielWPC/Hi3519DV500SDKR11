
ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

INIT_FILE := ../../../init/$(OSTYPE)/src/sensor_i2c_init.c
INIT_INC := ../../../init/$(OSTYPE)/include/ot_sensor_i2c_mod_init.h

SRCS := sensor_i2c.c
SRCS += $(INIT_FILE)

INC += -I$(ISP_ROOT)/ext_inc
INC += -I$(ISP_ROOT)/ext_drv/sensor_i2c/$(ARCH_DIR)/
INC += -I$(ISP_ROOT)/kernel/mkp/include
INC += -I$(ISP_ROOT)/kernel/arch/$(OT_ARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/arch/$(OT_ARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/include
INC += -I$(CBB_ROOT)/init/$(OSTYPE)/include
EXTRA_CFLAGS += $(INC)

#*************************************************************************
TARGET := $(KO_PREFIX)_sensor_i2c
EXTDRV_KO := y

#*************************************************************************
# compile linux or liteos
include $(MAKE_DRV_FILE)
