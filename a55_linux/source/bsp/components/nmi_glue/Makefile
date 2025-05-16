CUR_PATH = $(shell pwd)

ifeq ($(CUR_PATH)/../../../open_source/linux/linux-5.10.y, $(wildcard $(CUR_PATH)/../../../open_source/linux/linux-5.10.y))
KDIR = ../../../open_source/linux/linux-5.10.y
else ifeq ($(CUR_PATH)/../../../../open_source/linux/linux-5.10.y, $(wildcard $(CUR_PATH)/../../../../open_source/linux/linux-5.10.y))
KDIR = ../../../../open_source/linux/linux-5.10.y
else ifeq ($(CUR_PATH)/../../../../../open_source/linux/linux-5.10.y, $(wildcard $(CUR_PATH)/../../../../../open_source/linux/linux-5.10.y))
KDIR = ../../../../../open_source/linux/linux-5.10.y
else ifeq ($(CUR_PATH)/../../../../../../open_source/linux/linux-5.10.y, $(wildcard $(CUR_PATH)/../../../../../../open_source/linux/linux-5.10.y))
KDIR = ../../../../../../open_source/linux/linux-5.10.y
else
$(warning "---------------------------------------------------------------------")
$(warning "   Cannot found the linux-5.10.y Contents!!       ")
$(warning "---------------------------------------------------------------------")
$(error )
endif

obj-m := nmi_glue.o
nmi_glue-y := nmi_glue_register.o \
        nmi_glue_setup.o
CROSS_COMPILE ?= aarch64-v01c01-linux-musl-

export SECURE_LIBS_CFLAGS ?= -fstack-protector-strong -fPIC
export SECURE_EXEC_CFLAGS ?= -fPIE
ifneq ($(CONFIG_LLVM), y)
SECURE_EXEC_CFLAGS += -pie -s
endif
export SECURE_LD_CFLAGS   ?= -Wl,-z,relro -Wl,-z,noexecstack -Wl,-z,now,-s

all:
	make ARCH=arm64 CFLAGS=-Map,mp.map CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean

