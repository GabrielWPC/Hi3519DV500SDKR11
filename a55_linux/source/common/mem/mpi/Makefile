ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CBB_PARAM_FILE), )
    CBB_PARAM_FILE:=../../../mpp/cbb/Makefile.param
    include $(CBB_PARAM_FILE)
endif

MPI_CFLAGS := $(MPP_CFLAGS)
MPI_CFLAGS += -I$(MEM_ROOT)/include
MPI_CFLAGS += -I$(MEM_ROOT)/include/inner
MPI_CFLAGS += -I$(MEM_ROOT)/mkp/include

LIB_NAME=ot_mpi_sysmem
MPI_SRC_DIR = $(MEM_ROOT)/mpi
INC_FILE := $(wildcard $(MEM_ROOT)/include/*.h)

include $(MAKE_MPI_FILE)
