ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

#*************************************************************************
# all source file in this module
SRCS := otuser.c

#*************************************************************************
EXTRA_CFLAGS += -I./

ifeq ($(OSTYPE),liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif

TARGET := $(KO_PREFIX)_user
include $(MAKE_DRV_FILE)
