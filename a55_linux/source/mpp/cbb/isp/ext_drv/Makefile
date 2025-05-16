ifeq ($(PARAM_FILE), )
        PARAM_FILE:=../../../../Makefile.param
        include $(PARAM_FILE)
endif


target=$(shell find ./ -maxdepth 2 -mindepth 2 -name "Makefile" | xargs dirname)
target_clean=$(addsuffix _clean,$(target))
cur_name=$(shell pwd | xargs basename)
ifeq ($(cur_name), $(OT_ARCH))
cur_name=$(shell pwd | xargs dirname | xargs basename)
endif

.PHONY:all clean echo_info $(target) $(target_clean)
all: echo_info $(target)

clean: $(target_clean)

echo_info:
	@echo -e "\e[0;32;1m--Compiling '$(cur_name)'... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- OT_ARCH=$(OT_ARCH), OT_CHIP=$(OT_CHIP), CVER=$(CVER), OT_DBG=$(OT_DBG)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo -e "\e[0m"

$(target):
	@cd $@ && make

$(target_clean):
	@cd $(subst _clean,,$@) && make clean

