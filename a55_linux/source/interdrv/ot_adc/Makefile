ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

TARGET := $(KO_PREFIX)_adc
#OT_ARCH
#*************************************************************************
# all source file in this module
SRCS := adc.c \
        ../init/$(OSTYPE)/adc_init.c
#*************************************************************************
EXTRA_CFLAGS += -I$(PWD)
EXTRA_CFLAGS += -I$(PWD)/arch/$(OT_ARCH)

ifeq ($(OSTYPE),liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif

INC_FILE := $(wildcard $(PWD)/ot_adc.h)

EXT_TARGET:=sample
EXT_CLEAN:=sample_clean

include $(MAKE_DRV_FILE)

sample:
	@cd sample/$(OT_ARCH) && make
sample_clean:
	@cd sample/$(OT_ARCH) && make clean

