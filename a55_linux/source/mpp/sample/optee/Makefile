include ../Makefile.param

target=$(shell ls -d */ | sed "s;/;;g" | grep -v "common")
target_clean=$(addsuffix _clean,$(target))
target_cleanall=$(addsuffix _cleanall,$(target))

.PHONY:all clean cleanall $(target) $(target_clean) target_cleanall common common_clean

all:$(target)
clean:$(target_clean)

common:
	@cd ../common && make
common_clean:
	@cd ../common && make clean

rsa_pem:
	@if [ ! -f $(OPTEE_PATH)/keys/default_ta.pem ]; then \
		echo "default_ta.pem does not exist, creating..."; \
		openssl genrsa -out $(OPTEE_PATH)/keys/default_ta.pem 4096 || (echo "openssl command failed, please install openssl." && exit 1); \
	else \
		echo "$(OPTEE_PATH)/keys/default_ta.pem already exists."; \
	fi

$(target): common rsa_pem
	@cd $@ && make
$(target_clean): common_clean
	@cd $(subst _clean,,$@) && make clean
