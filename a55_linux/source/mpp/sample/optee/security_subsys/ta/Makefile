ifeq ($(PARAM_FILE), )
	OPTEE_SAMPLE_DIR:=$(PWD)/../..
	include $(OPTEE_SAMPLE_DIR)/Makefile.param
	include $(PARAM_FILE)
	TA_DEV_KIT_DIR = $(OPTEE_PATH)
endif

CFG_TEE_TA_LOG_LEVEL ?= 4

# The UUID for the Trusted Application
BINARY=8aabf300-2450-11e4-abe2-9f02a5d5bc2f

-include $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk

ifeq ($(wildcard $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk), )
clean:
	@echo 'Note: $$(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk not found, cannot clean TA'
	@echo 'Note: TA_DEV_KIT_DIR=$(TA_DEV_KIT_DIR)'
endif
