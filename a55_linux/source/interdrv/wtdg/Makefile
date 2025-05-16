ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

TARGET := $(KO_PREFIX)_wdt
# /* ARCH_LIBNAME*/
#*************************************************************************
# all source file in this module
SRCS := ot_wtdg.c \
        ../init/$(OSTYPE)/wtdg_init.c

#*************************************************************************
EXTRA_CFLAGS += $(MPP_CFLAGS)
EXTRA_CFLAGS += $(MKP_CFLAGS)
EXTRA_CFLAGS += -I$(PWD)
EXTRA_CFLAGS += -I$(OSAL_ROOT)/include

EXTRA_CFLAGS += -I$(PWD)/arch/$(OT_ARCH)

INC_FILE := $(wildcard $(PWD)/*.h)

EXT_TARGET:=sample
EXT_CLEAN:=sample_clean

include $(MAKE_DRV_FILE)

sample:
	@cd sample && make
sample_clean:
	@cd sample && make clean
