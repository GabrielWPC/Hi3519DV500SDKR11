# $(ROOT)/source/Mafile

ifeq ($(PARAM_FILE), ) 
    PARAM_FILE:=../../source/Makefile.param
    include $(PARAM_FILE)
endif

EXTDRV_KO=$(REL_KO)/extdrv

sub_dir := $(shell ls -d */ | sed "s;/;;g")
clean_dir := $(addsuffix _clean,$(sub_dir))

.PHONY:clean all $(sub_dir) $(clean_dir)
all: $(sub_dir)

$(sub_dir):
	@cd $@ && make

clean: $(clean_dir)
	@echo -e "Clean ok."

$(clean_dir):
	@cd $(patsubst %_clean, %, $@) && make clean
