ifeq ($(EXTDRV_PARAM_FILE), )
    SUB_DIR_DEEP=../
    EXTDRV_PARAM_FILE:=../../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

ifeq ($(CONFIG_OT_MOTIONFUSION_SUPPORT), y)

SRCS += ./src/motionsensor_exe.c
SRCS += ./src/motionsensor_buf.c
ifeq ($(CONFIG_OT_PROC_SHOW_SUPPORT), y)
SRCS += ./src/motionsensor_mng_proc.c
endif

INC := -I$(OSAL_ROOT)/include
INC += -I$(SDK_PATH)/source/common/base/ext_inc
INC += -I$(PWD)/include
INC += -I$(PWD)/../include/
INC += -I$(PWD)/../ext_inc

ifeq ($(MPP_BUILD),y)
	INC += -I$(CBB_ROOT)/sys/ext_inc
else
	INC += -I$(REL_INC) -I$(REL_INC)/exp_inc
endif

EXTDRV_CFLAGS += $(INC)

#*************************************************************************
TARGET := motionsensor_mng
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
