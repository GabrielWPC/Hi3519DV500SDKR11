ifeq ($(PARAM_FILE),)
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

target := mmz libot_mpi_sysmem
target_clean := $(addsuffix _clean, $(target))

ifeq ($(OSTYPE),linux)
	MMZ_ARCH_NAME := linux
else ifeq ($(OSTYPE),liteos)
	MMZ_ARCH_NAME := liteos_v1
else
	MMZ_ARCH_NAME := liteos_v2
endif

.PHONY:all clean $(target) $(target_clean)

all: $(target)
	@echo "Build mem all success"
libot_mpi_sysmem:
	@cd mpi && make
	@echo "Compile libot_mpi_sysmem success"
mmz:
	@cd mmz/$(MMZ_ARCH_NAME) && make
	@echo "Compile mmz success"

clean: $(target_clean)
	@echo "Clean mem all success"
libot_mpi_sysmem_clean:
	@cd mpi && make clean
mmz_clean:
	@cd mmz/$(MMZ_ARCH_NAME) && make clean
