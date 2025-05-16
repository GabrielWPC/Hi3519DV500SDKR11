
ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

INIT_FILE := ../../../init/$(OSTYPE)/src/pwm_init.c
INIT_INC := ../../../init/$(OSTYPE)/include/ot_pwm_mod_init.h
SRCS = pwm.c
SRCS += $(INIT_FILE)

INC += -I$(ISP_ROOT)/ext_drv/pwm/$(ARCH_DIR)/
INC += -I$(DRV_ROOT)/extdrv/$(EXTDRVVER)/pwm/arch/$(ARCH_DIR)/
INC += -I$(CBB_ROOT)/init/$(OSTYPE)/include
EXTRA_CFLAGS += $(INC)

#*************************************************************************
TARGET := $(KO_PREFIX)_pwm
EXTDRV_KO := y

#*************************************************************************
# compile linux or liteos
include $(MAKE_DRV_FILE)
