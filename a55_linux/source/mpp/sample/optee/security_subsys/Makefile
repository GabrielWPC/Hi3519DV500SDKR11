export OPTEE_SAMPLE_DIR=$(PWD)/..
ifeq ($(PARAM_FILE), )
	include $(OPTEE_SAMPLE_DIR)/Makefile.param
	include $(PARAM_FILE)
endif

TA_DEV_KIT_DIR = $(OPTEE_PATH)
ifeq ($(CROSS), aarch64-llvm-linux-)
	CROSS_COMPILE=aarch64-v01c01-linux-musl-
else
	CROSS_COMPILE=$(CROSS)
endif

TARGET_PATH := $(PWD)

.PHONY: all host ta clean host_clean ta_clean

all: host ta

host:
	@cd host && make CROSS_COMPILE="$(CROSS_COMPILE)" TA_DEV_KIT_DIR="$(TA_DEV_KIT_DIR)"

ta:
	@if [ ! -f $(OPTEE_PATH)/keys/default_ta.pem ]; then \
		echo "default_ta.pem does not exist, creating..."; \
		openssl genrsa -out $(OPTEE_PATH)/keys/default_ta.pem 4096 || (echo "openssl command failed, please install openssl." && exit 1); \
	else \
		echo "$(OPTEE_PATH)/keys/default_ta.pem already exists."; \
	fi
	@cd ta && make CROSS_COMPILE="$(CROSS_COMPILE)" TA_DEV_KIT_DIR="$(TA_DEV_KIT_DIR)"

clean: host_clean ta_clean

host_clean:
	@cd host && make clean

ta_clean:
	@cd ta && make clean TA_DEV_KIT_DIR="$(TA_DEV_KIT_DIR)"

