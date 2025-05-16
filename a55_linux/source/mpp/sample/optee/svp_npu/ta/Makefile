ifeq ($(PARAM_FILE), )
	OPTEE_SAMPLE_DIR:=$(PWD)/../..
	include $(OPTEE_SAMPLE_DIR)/Makefile.param
	include $(PARAM_FILE)
	TA_DEV_KIT_DIR = $(OPTEE_PATH)
endif

CFG_TEE_TA_LOG_LEVEL ?= 1
CPPFLAGS += -DCFG_TEE_TA_LOG_LEVEL=$(CFG_TEE_TA_LOG_LEVEL)

# The UUID for the Trusted Application
BINARY=426e8f6f-ec74-427b-9424-0dd8f3f33706

-include $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk

ifeq ($(wildcard $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk), )
clean:
	@echo 'Note: $$(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk not found, cannot clean TA'
	@echo 'Note: TA_DEV_KIT_DIR=$(TA_DEV_KIT_DIR)'
endif
