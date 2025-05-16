ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../Makefile.param
    include $(PARAM_FILE)
endif

INIT_FILE := smmz_agent_init.c
INIT_INC := ot_smmz_agent_mod_init.h
INC_FILE := $(wildcard include/*.h)

SRCS := smmz_agent.c smmz_agent_misc_dev.c smmz_agent_phy_buffer_allocator.c smmz_agent_dma_buf.c
SRCS_O := $(SRCS)

SRCS += $(INIT_FILE)

TARGET   := $(KO_PREFIX)_smmz_agent

MOD_NAME := mod_smmz_agent

DRV_CFLAGS += -I$(CURDIR)/include

include $(MAKE_DRV_FILE)
