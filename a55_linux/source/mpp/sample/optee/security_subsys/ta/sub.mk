global-incdirs-y += ../include

TEST_FLAGS-y	+= -I$(PWD)
TEST_FLAGS-y	+= -I../include
TEST_FLAGS-y	+= -I../api_include/common
TEST_FLAGS-y	+= -I../api_include/mpi

TEST_FLAGS-y	+= -D__LP32__ -DCONFIG_CRYPTO_CHIP_HI3519DV500

ifeq ($(CONFIG_LLVM), y)
	CPPFLAGS += -fsanitize=address
else
	CPPFLAGS += -fstack-protector-strong
endif

srcs-y		+= $(wildcard *.c)
cflags-y  += $(TEST_FLAGS-y)

globl-incdirs-y += $(TA_DEV_KIT_DIR)/include
