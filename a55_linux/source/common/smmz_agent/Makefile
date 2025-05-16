ifeq ($(PARAM_FILE),)
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CONFIG_OT_SMMZ_AGENT_SUPPORT), y)

target := smmz_agent_kernel libsmmz_agent
target_clean := $(addsuffix _clean, $(target))

.PHONY:all clean $(target) $(target_clean)

all: $(target)
	@echo "Build mem all success"
libsmmz_agent:
	@cd user && make
	@echo "Compile libsmmz_agent success"
smmz_agent_kernel:
	@cd kernel && make
	@echo "Compile mmz success"

clean: $(target_clean)
	@echo "Clean mem all success"
smmz_agent_kernel_clean:
	@cd kernel && make clean
libsmmz_agent_clean:
	@cd user && make clean
else
all:
	@echo "smmz agent not compile"
clean:
	@echo "smmz agent don't need clean"
endif
