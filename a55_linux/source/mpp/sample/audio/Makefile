
include ../Makefile.param

SMP_SRCS := $(wildcard *.c)

TARGET := $(SMP_SRCS:%.c=%)

SMP_SRCS += $(wildcard $(PWD)/adp/*.c)

TARGET_PATH := $(PWD)

LIBS_LD_CFLAGS += -L$(REL_LIB) -lsecurec -lmp3_enc -lmp3_lame -lmp3_dec

# compile linux or liteos
include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
