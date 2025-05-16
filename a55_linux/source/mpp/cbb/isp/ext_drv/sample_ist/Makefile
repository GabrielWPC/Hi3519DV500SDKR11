
ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

INIT_FILE := ../../../init/$(OSTYPE)/src/sample_ist_init.c
INIT_INC := ../../../init/$(OSTYPE)/include/ot_sample_ist_mod_init.h
SRCS := sample_ist.c
SRCS += $(INIT_FILE)
INC += -I$(ISP_ROOT)/ext_inc
INC += -I$(ISP_ROOT)/kernel/mkp/include
INC += -I$(ISP_ROOT)/kernel/arch/$(OT_ARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/arch/$(OT_ARCH)/include
INC += -I$(ISP_ROOT)/user/firmware/include
INC += -I$(CBB_ROOT)/init/$(OSTYPE)/include
EXTRA_CFLAGS += $(INC)

#*************************************************************************
TARGET := $(KO_PREFIX)_sample_ist
EXTDRV_KO := y

#*************************************************************************
# compile linux or liteos
include $(MAKE_DRV_FILE)
