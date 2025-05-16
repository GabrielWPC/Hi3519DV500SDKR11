
include ../Makefile.param

INC_FLAGS +=-I./include
INC_FLAGS +=-I./../scene
INC_FLAGS +=-I./../common
INC_FLAGS +=-I$(REL_DIR)/include

SMP_SRCS += $(wildcard ./src/core/*.c)
SMP_SRCS += $(wildcard ./src/sample/*.c)

INC_FLAGS +=-I./include
INC_FLAGS +=-I$(REL_DIR)/include

TARGET := sample_scene

TARGET_PATH := $(PWD)
MPI_LIBS += $(REL_LIB)/libss_mpi_mcf.a
MPI_LIBS += $(REL_LIB)/libss_mpi_mcf_calibrate.a

ifeq ($(CONFIG_OT_AIISP_SUPPORT), y)
ifeq ($(CONFIG_OT_AIBNR_SUPPORT), y)
MPI_LIBS += $(REL_LIB)/libss_mpi_aibnr.a
CFLAGS += -DCONFIG_SCENEAUTO_AIBNR_SUPPORT
endif
ifeq ($(CONFIG_OT_AIDRC_SUPPORT), y)
MPI_LIBS += $(REL_LIB)/libss_mpi_aidrc.a
CFLAGS += -DCONFIG_SCENEAUTO_AIDRC_SUPPORT
endif
ifeq ($(CONFIG_OT_AI3DNR_SUPPORT), y)
MPI_LIBS += $(REL_LIB)/libss_mpi_ai3dnr.a
MPI_LIBS += $(REL_LIB)/libss_mpi_mcf.a
CFLAGS += -DCONFIG_SCENEAUTO_AI3DNR_SUPPORT
endif
endif

CFLAGS += $(INC_FLAGS)


# compile linux or LITEOS
include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
