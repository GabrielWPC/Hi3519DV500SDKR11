include ../Makefile.param

SMP_SRCS := $(wildcard *.c)
TARGET := $(SMP_SRCS:%.c=%)

MPI_LIBS += $(REL_LIB)/libss_tde.a

TARGET_PATH := $(PWD)

# compile linux or LITEOS
ifeq ($(OSTYPE), linux)
CFLAGS += -DCONFIG_SUPPORT_SAMPLE_ROTATION
include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
else
.PHONY : clean all
all:

clean:

endif
