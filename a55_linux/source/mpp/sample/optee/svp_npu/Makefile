export OPTEE_SAMPLE_DIR=$(PWD)/..
ifeq ($(PARAM_FILE), )
     include $(OPTEE_SAMPLE_DIR)/Makefile.param
	 include $(PARAM_FILE)
endif

ifeq ($(CROSS), aarch64-llvm-linux-)
	CROSS_COMPILE=aarch64-v01c01-linux-musl-
else
	CROSS_COMPILE=$(CROSS)
endif

OBJ_DIR := $(TEMP_PATH)/$(subst $(SDK_PATH)/,,$(CURDIR))
TARGET_PATH := $(PWD)

.PHONY: all host ta clean host_clean ta_clean

all: host ta

host:
	@cd host && make

ta:
	@if [ ! -f $(OPTEE_PATH)/keys/default_ta.pem ]; then \
		echo "default_ta.pem does not exist, creating..."; \
		openssl genrsa -out $(OPTEE_PATH)/keys/default_ta.pem 4096 || (echo "openssl command failed, please install openssl." && exit 1); \
	else \
		echo "$(OPTEE_PATH)/keys/default_ta.pem already exists."; \
	fi
	@mkdir -p $(OBJ_DIR)/ta
	@cp -r ta/* $(OBJ_DIR)/ta
	@cd $(OBJ_DIR) && make -C ta CROSS_COMPILE="$(CROSS_COMPILE)" TA_DEV_KIT_DIR="$(OPTEE_PATH)"
	@mv $(OBJ_DIR)/ta/*.ta $(TARGET_PATH)/ta

clean: host_clean ta_clean

host_clean:
	@cd host && make clean

ta_clean:
	@rm -rf $(TARGET_PATH)/ta/*.ta
	@rm -rf $(OBJ_DIR)/ta

