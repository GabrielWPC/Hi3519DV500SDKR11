ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

#*************************************************************************
# all source file in this module
SRCS := mipi_rx.c \
        mipi_rx_hal.c  \
        ../init/$(OSTYPE)/mipi_rx_init.c
#*************************************************************************
EXTRA_CFLAGS += -I$(PWD)
ifeq ($(MPP_BUILD), y)
EXTRA_CFLAGS += -I$(DRV_ROOT)/$(OT_ARCH)/init/$(OSTYPE)
EXTRA_CFLAGS += -I$(SYS_ROOT)/ext_inc/
EXTRA_CFLAGS += -I$(BASE_ROOT)/ext_inc/
else
EXTRA_CFLAGS += -I$(DRV_ROOT)/init/$(OSTYPE)
endif
EXTRA_CFLAGS += -D$(OT_FPGA)

ifeq ($(OSTYPE),liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif

TARGET := $(KO_PREFIX)_mipi_rx
INC_FILE := $(PWD)/ot_mipi_rx.h

include $(MAKE_DRV_FILE)
