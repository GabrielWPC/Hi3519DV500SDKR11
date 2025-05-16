ifeq ($(PARAM_FILE), )
	OPTEE_SAMPLE_DIR:=$(PWD)/../..
	include $(OPTEE_SAMPLE_DIR)/Makefile.param
	include $(PARAM_FILE)
endif

include Makefile.param

CFLAGS += -I$(PWD)/include
CFLAGS += -O3

SMP_SRCS := $(wildcard ./*.c)
SMP_SRCS += $(wildcard ./sample_svp_npu/*.c)

TARGET := sample_svp_npu_main
TARGET_PATH := $(PWD)

# compile linux or LITEOS
include $(PWD)/../../../$(ARM_ARCH)_$(OSTYPE).mak
