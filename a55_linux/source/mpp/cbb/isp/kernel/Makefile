#*************************************************************************
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CBB_PARAM_FILE), )
    CBB_PARAM_FILE:=../../Makefile.param
    include $(CBB_PARAM_FILE)
endif
#*************************************************************************
# local header include

ISP_KERNEL_ROOT := $(ISP_ROOT)
MKP_CFLAGS+=-I $(PWD)/mkp/include
MKP_CFLAGS+=-I $(PWD)/arch/include
MKP_CFLAGS+=-I $(PWD)/arch/$(OT_ARCH)/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/include/arch/$(OT_ARCH)/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/user/firmware/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/user/pq_ai/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/user/firmware/arch/$(OT_ARCH)/include
MKP_CFLAGS+=-I $(MPP_PATH)/cbb/snap/include
MKP_CFLAGS+=-I$(VI_ROOT)/include/inner

ifneq ($(MPP_BUILD),y)
MKP_CFLAGS+=-I $(CBB_ROOT)/init/linux/include
MKP_CFLAGS+=-I $(ISP_KERNEL_ROOT)/ext_inc
endif
#*************************************************************************
# all source file in this module
SRCS:=

# src relative path
CBB_ISP_ROOT=./
CBB_ISP_ARCH=arch/$(OT_ARCH)/

# Makefile absolute path

include $(ISP_KERNEL_ROOT)/kernel/mkp/src/Makefile
include $(ISP_KERNEL_ROOT)/kernel/arch/$(OT_ARCH)/hal/Makefile

#INIT_FILE := ../../init/$(OSTYPE)/isp_init.c
INIT_FILE := ../../init/$(OSTYPE)/src/isp_init.c
INIT_INC := ../../init/linux/include/ot_isp_mod_init.h

SRCS_O := $(SRCS)
SRCS   += $(INIT_FILE)

#*************************************************************************
# release header
#INC_FILE  := $(wildcard $(ISP_ROOT)/include/*.h)
#INC_FILE  += $(wildcard include/*.h)
#INC_FILE  += $(wildcard ../region/*.h)
#INC_FILE  += ../region/region_ext.h
#INC_FILE  += $(wildcard )
#INC_FILE  += $(CBB_BASE_ARCH)/include/ot_defines.h

MOD_NAME := mod_isp

#*************************************************************************
TARGET := $(KO_PREFIX)_isp

#*************************************************************************
# compile linux or liteos
include $(MAKE_DRV_FILE)
