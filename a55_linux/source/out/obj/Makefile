#*******************************************************************************
#The rule of make Linux kernel module
#*******************************************************************************
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(wildcard $(KERNEL_ROOT)/Makefile),)
$(error The $(KERNEL_ROOT) is not exist! Please compile it first.)
endif

ifneq ($(BUILD_DRIVER_TO_KERNEL),y)
objs_list = $(shell ls *.o | sed "s;mod_;;g" | sed "s;\.o;;g")
target = $(filter-out base isp smmz_agent,$(objs_list))
target_clean=$(addsuffix _clean,$(target))

cbb_target = isp gfbg security_subsys
cbb_clean=$(addsuffix _clean,$(cbb_target))

app_target = sample tools
app_clean=$(addsuffix _clean,$(app_target))

prepare = platform osal mem smmz_agent base interdrv vendor
prepare_clean=$(addsuffix _clean,$(prepare))

OSAL_SYMBOLS := $(TEMP_PATH)/source/osal/src/linux/kernel/Module.symvers
BASE_SYMBOLS := $(OSAL_SYMBOLS)
BASE_SYMBOLS += $(TEMP_PATH)/source/common/mem/mmz/linux/Module.symvers
BASE_SYMBOLS += $(PWD)/tmp/base/Module.symvers
export KBUILD_EXTRA_SYMBOLS := $(BASE_SYMBOLS)
KBUILD_EXTRA_SYMBOLS += $(TEMP_PATH)/source/interdrv/sysconfig/Module.symvers

.PHONY: all clean $(target) $(target_clean) $(prepare) $(prepare_clean) $(cbb_target) $(cbb_clean) $(app_target) $(app_clean)

all: $(target) $(cbb_target) $(app_target) $(app_clean)
clean: $(target_clean) $(prepare_clean) $(cbb_clean)
	@rm tmp -rf

platform:
	@echo $(target)
	@cd $(SDK_PATH)/../../platform/securec && make KBUILD_EXTRA_SYMBOLS=
platform_clean:
	@cd $(SDK_PATH)/../../platform/securec && make clean

osal: platform
	@cd $(SDK_PATH)/source/osal/src/$(OSTYPE)/kernel && make KBUILD_EXTRA_SYMBOLS=
osal_clean:
	@cd $(SDK_PATH)/source/osal/src/$(OSTYPE)/kernel && make clean

mem: osal
	@cd $(SDK_PATH)/source/common/mem && make KBUILD_EXTRA_SYMBOLS=$(OSAL_SYMBOLS)
mem_clean:
	@cd $(SDK_PATH)/source/common/mem && make clean

smmz_agent:
	@cd $(SDK_PATH)/source/common/smmz_agent && make KBUILD_EXTRA_SYMBOLS=
smmz_agent_clean:
	@cd $(SDK_PATH)/source/common/smmz_agent && make clean

base: mem
	@mkdir tmp/$@ -p && cp Makefile.linux tmp/$@/Makefile
	@cd tmp/$@ && make TARGET=$@
base_clean:
	@rm tmp/base -rf
	@rm $(REL_KO)/ot_base.ko -f

interdrv: base
	@cd $(SDK_PATH)/source/interdrv && make KBUILD_EXTRA_SYMBOLS="$(BASE_SYMBOLS)"
interdrv_clean:
	@cd $(SDK_PATH)/source/interdrv && make clean

vendor: base
	@cd $(SDK_PATH)/vendor/extdrv && make KBUILD_EXTRA_SYMBOLS="$(BASE_SYMBOLS)"
vendor_clean:
	@cd $(SDK_PATH)/vendor/extdrv && make clean

$(target): $(prepare)
	@mkdir tmp/$@ -p && cp Makefile.linux tmp/$@/Makefile
	@cd tmp/$@ && make TARGET=$@
$(target_clean):
	@rm tmp/$(subst _clean,,$@) -rf
	@rm $(REL_KO)/$(KO_PREFIX)_$(subst _clean,,$@).ko -f

$(cbb_target): $(prepare)
	@cd $(SDK_PATH)/source/mpp/cbb/$@ && make
$(cbb_clean):
	@cd $(SDK_PATH)/source/mpp/cbb/$(subst _clean,,$@) && make clean

$(app_target): $(target) $(cbb_target)
	@cd $(SDK_PATH)/source/mpp/$@ && make
$(app_clean):
	@cd $(SDK_PATH)/source/mpp/$(subst _clean,,$@) && make clean
else
BOARD_ROOT=$(KERNEL_ROOT)/drivers/vendor/board

.PHONY: all clean
all:
	@mkdir -p $(BOARD_ROOT)/mpp
	@cp Makefile.config $(BOARD_ROOT)
ifeq ($(wildcard $(BOARD_ROOT)/Makefile),)
	@cp Makefile.kernel $(BOARD_ROOT)/Makefile
	@cp -rf $(REL_DIR) $(BOARD_ROOT)
	@cp -rf $(COMMON_ROOT) $(BOARD_ROOT)
	@cp -rf $(DRV_ROOT) $(BOARD_ROOT)
	@cp -rf $(OSAL_ROOT) $(BOARD_ROOT)
	@cp -rf $(SDK_PATH)/$(EXTDRV) $(BOARD_ROOT)
	@cp -rf $(CBB_ROOT) $(BOARD_ROOT)/mpp
	@sed -i "s#KBUILD_LDFLAGS :=#KBUILD_LDFLAGS := --allow-multiple-definition#" $(KERNEL_ROOT)/Makefile
	@echo "obj-y += board/" >> $(KERNEL_ROOT)/drivers/vendor/Makefile
endif
	@make -C $(KERNEL_ROOT) ARCH=arm64 CROSS_COMPILE=$(CONFIG_OT_CROSS) uImage

clean:
	@rm -rf $(BOARD_ROOT)
	@sed -i "s# --allow-multiple-definition##" $(KERNEL_ROOT)/Makefile
	@sed -i "/obj-y += board/d" $(KERNEL_ROOT)/drivers/vendor/Makefile
	@make -C $(KERNEL_ROOT) ARCH=arm64 CROSS_COMPILE=$(CONFIG_OT_CROSS) uImage
endif
