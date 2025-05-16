ifeq ($(EXTDRV_PARAM_FILE), )
    SUB_DIR_DEEP=../
    EXTDRV_PARAM_FILE:=../../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif
ifeq ($(CONFIG_OT_MOTIONFUSION_SUPPORT), y)
# Sensor Type: ICM20690 / ICM42670
SENSOR_TYPE ?= ICM42670

# Transfer Type: I2C SPI
export TRANSFER_TYPE := I2C

# ICM proc info function : y or n
ifeq ($(SENSOR_TYPE), ICM20690)
    export ICM20690_PROC = y
else ifeq ($(SENSOR_TYPE), ICM42670)
    export ICM42670_PROC = y
endif

SRCS = ./main/motionsensor.c

INC := -I$(REL_INC)
INC += -I$(OSAL_ROOT)/include
INC += -I$(CBB_ROOT)/include

INC += -I$(CBB_ROOT)/include/adapt
INC += -I$(CBB_ROOT)/base/include/adapt
INC += -I$(CBB_ROOT)/sys/include/adapt

#debug
INC += -I$(CBB_ROOT)/vi/ext_inc
ifeq ($(OSTYPE), liteos)
INC += -I$(LITEOSTOPDIR)/drivers/gpio/include
endif

INC := -I$(OSAL_ROOT)/include
INC += -I$(SDK_PATH)/source/common/base/ext_inc
INC += -I$(PWD)/include
INC += -I$(PWD)/main
INC += -I$(PWD)/sensor_dev/icm42670
INC += -I$(PWD)/pub
INC += -I$(PWD)/../include/
INC += -I$(PWD)/../ext_inc
INC += -I$(PWD)/../motionsensor_spi
INC += -I$(PWD)/../motionsensor_mng/include

ifeq ($(MPP_BUILD),y)
	INC += -I$(CBB_ROOT)/sys/ext_inc
else
	INC += -I$(REL_INC) -I$(REL_INC)/exp_inc
endif

ifeq ($(CONFIG_OT_PROC_SHOW_SUPPORT),y)
SRCS += ./proc/motionsensor_chip_proc.c
INC += -I$(PWD)/proc
ifeq ($(ICM20690_PROC),y)
	ccflags-y += -DIMU_ICM20690
else ifeq ($(ICM42670_PROC),y)
    ccflags-y += -DIMU_ICM42670
endif
endif

ifeq ($(SENSOR_TYPE), ICM20690)
    SRCS += ./sensor_dev/icm20690/icm20690.c
	SRCS += ./sensor_dev/icm20690/motionsensor_gpio.c
    INC += -I$(PWD)/sensor_dev/icm20690
else
    SRCS += ./sensor_dev/icm42670/icm42670.c
    SRCS += ./sensor_dev/icm42670/motionsensor_gpio.c
    INC += -I$(PWD)/sensor_dev/icm42670
endif

ifeq ($(TRANSFER_TYPE), I2C)
    SRCS += ./transfer_type/i2c/i2c_dev.c
    ccflags-y += -DTRANSFER_I2C
    INC += -I$(PWD)/transfer_type/i2c
    INC += -I$(TRANSFER_DIR)/i2c
endif

ifeq ($(TRANSFER_TYPE), SPI)
ifeq ($(CONFIG_LITEOS_OS),y)
    SRCS += ./transfer_type/spi/spi_dev.c
endif
	ccflags-y += -DTRANSFER_SPI
    INC += -I$(PWD)/transfer_type/spi
    INC += -I$(TRANSFER_DIR)/spi
endif

EXTDRV_CFLAGS += $(INC)

#*************************************************************************
TARGET := motionsensor_chip

#*************************************************************************
# compile linux or liteos
include $(PWD)/../../Make.$(OSTYPE)

#CONFIG_OT_MOTIONFUSION_SUPPORT endif
else
all:
	@echo "motionsensor module not compile"
clean:
	@echo "motionsensor module don't need clean"
endif
