ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../../Makefile.param
     include $(PARAM_FILE)
endif
COMMON_DIR ?= $(shell pwd)/common

include $(shell pwd)/Makefile.param


all:
	@cd $(OPEN_SOURCE_PATH)/iniparser && make
clean:
	@cd $(OPEN_SOURCE_PATH)/iniparser && make clean;

