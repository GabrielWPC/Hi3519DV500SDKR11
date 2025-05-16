include ../Makefile.param

########### SRCS & INC ############
SMP_SRCS := $(wildcard $(PWD)/*.c)

TARGET := sample_mcf

TARGET_PATH := $(PWD)

MPI_LIBS += $(REL_LIB)/libss_mpi_mcf.a
MPI_LIBS += $(REL_LIB)/libss_mpi_mcf_calibrate.a

# compile linux or LITEOS
include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
