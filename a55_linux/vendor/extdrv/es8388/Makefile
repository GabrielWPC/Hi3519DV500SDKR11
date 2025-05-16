ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

SRCS := es8388.c

INC := -I$(REL_INC)
INC += -I$(OSAL_ROOT)/linux/kernel/ot_media
INC += -I$(OSAL_ROOT)/include

ifeq ($(CONFIG_GPIO_I2C),y)
	INC += -DOT_GPIO_I2C
	INC+=-I$(PWD)/../gpio-i2c-ex
else
	INC += -DOT_I2C
	INC+=-I$(PWD)/../ot_i2c
endif

EXTDRV_CFLAGS += $(INC)

#*************************************************************************
TARGET := ot_es8388

#*************************************************************************
# compile linux or liteos
include $(PWD)/../Make.$(OSTYPE)
