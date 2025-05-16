
include ../Makefile.param

INC_FLAGS +=-I./include
INC_FLAGS +=-I$(REL_DIR)/include
SMP_SRCS += $(wildcard *.c)
SMP_SRCS += $(wildcard ./src/*.c)
TARGET := sample_svc_rate

TARGET_PATH := $(PWD)
CFLAGS += $(INC_FLAGS)

include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
