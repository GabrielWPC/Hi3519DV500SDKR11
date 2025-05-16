ifeq ($(EXTDRV_PARAM_FILE), )
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

.PHONY: default clean

ifeq ($(CONFIG_OT_MOTIONFUSION_SUPPORT), y)

default:
	@echo -e "\e[0;32;1m--Compiling 'motionsensor_spi'...\e[0;36;1m"
	@@echo -e "\e[0m"
	@cd ./motionsensor_spi && make;
	@echo -e "\e[0;32;1m--Compiling 'motionsensor_mng'...\e[0;36;1m"
	@@echo -e "\e[0m"
	@cd ./motionsensor_mng && make;
	@echo -e "\e[0;32;1m--Compiling 'motionsensor_chip'...\e[0;36;1m"
	@@echo -e "\e[0m"
	@cd ./motionsensor_chip && make;
	@mkdir -p $(REL_INC) && cp -rf include/ot_common_motionsensor.h $(REL_INC)
	@mkdir -p $(REL_INC) && cp -rf include/ot_motionsensor_chip_cmd.h $(REL_INC)
	@mkdir -p $(REL_INC) && cp -rf include/ot_motionsensor_mng_cmd.h $(REL_INC)

clean:
	@cd ./motionsensor_spi && make clean;
	@cd ./motionsensor_mng && make clean;
	@cd ./motionsensor_chip && make clean;
	@rm -f $(REL_INC)/ot_common_motionsensor.h
	@rm -f $(REL_INC)/ot_motionsensor_chip_cmd.h
	@rm -f $(REL_INC)/ot_motionsensor_mng_cmd.h

#CONFIG_OT_MOTIONFUSION_SUPPORT endif
else
all:
	@echo "motionsensor module not compile"
clean:
	@echo "motionsensor module don't need clean"
endif
