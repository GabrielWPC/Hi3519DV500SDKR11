ifeq ($(EXTDRV_PARAM_FILE), )
    SUB_DIR_DEEP=../
    EXTDRV_PARAM_FILE:=../../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

ifeq ($(CONFIG_OT_MOTIONFUSION_SUPPORT), y)

SRCS := motionsensor_spi.c

INC := -I$(REL_INC)
INC += -I$(OSAL_ROOT)/include
EXTDRV_CFLAGS += $(INC)
EXTDRV_CFLAGS += $(COMM_INC)

#*************************************************************************
TARGET := ot_spi
#*************************************************************************
# compile linux or liteos
include $(PWD)/../../Make.$(OSTYPE)

#CONFIG_OT_MOTIONFUSION_SUPPORT endif
else
all:
	@echo "motionsensor module not compile"
clean:
	@echo "motionsensor module don't need clean"
endif