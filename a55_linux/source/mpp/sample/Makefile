ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

target_list=$(shell ls -d */ | sed "s;/;;g")

target = $(filter-out common,$(target_list))
target_clean=$(addsuffix _clean,$(target))

.PHONY:clean all common common_clean $(target) $(target_clean)

all:$(target)
	@echo "~~~~~~~~~~~~~~Build All Sample SUCCESS~~~~~~~~~~~~~~"
clean:$(target_clean) common_clean
	@echo "~~~~~~~~~~~~~~Clean All Sample SUCCESS~~~~~~~~~~~~~~"

common:
	@echo "~~~~~~~~~~Start build $@~~~~~~~~~~"
	@cd common && make
common_clean:
	@echo "~~~~~~~~~~Start clean $(subst _clean,,$@)~~~~~~~~~~"
	@cd common && make clean

$(target):common
	@echo "~~~~~~~~~~Start build $@~~~~~~~~~~"
	@cd $@ && make
$(target_clean):
	@echo "~~~~~~~~~~Start clean $(subst _clean,,$@)~~~~~~~~~~"
	@cd $(subst _clean,,$@) && make clean
