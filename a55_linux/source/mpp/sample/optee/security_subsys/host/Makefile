ifeq ($(PARAM_FILE), )
	OPTEE_SAMPLE_DIR:=$(PWD)/../..
	include $(OPTEE_SAMPLE_DIR)/Makefile.param
	include $(PARAM_FILE)
endif

include Makefile.param

CFLAGS += -I$(PWD)/../include -I$(TA_DEV_KIT_DIR)/host_include
CFLAGS += -O3

SMP_SRCS := $(wildcard ./*.c)

TARGET := sample_cipher
TARGET_PATH := $(PWD)

# compile linux or LITEOS
include $(PWD)/../../../$(ARM_ARCH)_$(OSTYPE).mak