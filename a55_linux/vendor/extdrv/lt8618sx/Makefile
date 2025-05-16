
ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

SRCS := lt8618sx.c
INC += -I$(OSAL_ROOT)/linux/kernel/himedia
INC += -I$(OSAL_ROOT)/include
INC += -I$(REL_INC)
INC += -I$(VENDOR_ROOT)/$(HIARCH)/lt8618sx
INC_FILE:=$(VENDOR_ROOT)/$(HIARCH)/lt8618sx/lt8618sx.h

EXTRA_CFLAGS += -DI2C_INTERNAL

EXTDRV_CFLAGS += $(INC)
EXTDRV_CFLAGS += $(INC)

#*************************************************************************
TARGET := ot_lt8618sx

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)

