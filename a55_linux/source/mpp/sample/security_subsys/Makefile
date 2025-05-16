include ../Makefile.param


########### SRCS & INC ############
SMP_SRCS := $(wildcard $(PWD)/*.c)

CFLAGS += -I$(REL_INC)
MPI_LIBS += $(REL_LIB)/libss_mpi_otp.a
MPI_LIBS += $(REL_LIB)/libss_mpi_km.a
MPI_LIBS += $(REL_LIB)/libss_mpi_cipher.a

TARGET := sample_cipher

TARGET_PATH := $(PWD)

CFLAGS	+= -DCONFIG_CRYPTO_CHIP_HI3519DV500

ifeq ($(CONFIG_LLVM), y)
	CFLAGS += -fsanitize=address
else
	CFLAGS += -fstack-protector-strong
endif

# compile linux or LITEOS
include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
