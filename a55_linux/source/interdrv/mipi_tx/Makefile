ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

#*************************************************************************
# all source file in this module
SRCS := mipi_tx.c \
        mipi_tx_hal.c  \
        ../init/$(OSTYPE)/mipi_tx_init.c
#*************************************************************************
EXTRA_CFLAGS += -I./
ifeq ($(MPP_BUILD), y)
EXTRA_CFLAGS += -I$(DRV_ROOT)/$(OT_ARCH)/init/$(OSTYPE)
else
EXTRA_CFLAGS += -I$(DRV_ROOT)/init/$(OSTYPE)
endif
EXTRA_CFLAGS += -D$(OT_FPGA)

ifeq ($(OSTYPE),liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif

TARGET := $(KO_PREFIX)_mipi_tx
INC_FILE := $(PWD)/ot_mipi_tx.h
include $(MAKE_DRV_FILE)
