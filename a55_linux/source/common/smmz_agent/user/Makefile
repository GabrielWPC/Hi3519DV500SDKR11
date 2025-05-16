ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../Makefile.param
    include $(PARAM_FILE)
endif

SMMZ_AGENT_LIBDIR:=$(CURDIR)

INC_FILE += $(SMMZ_AGENT_LIBDIR)/include/smmz_agent.h
MPI_CFLAGS += -I$(SMMZ_AGENT_LIBDIR)/include

LIB_NAME=smmz_agent
MPI_SRC_DIR = $(SMMZ_AGENT_LIBDIR)/src

include $(MAKE_MPI_FILE)
