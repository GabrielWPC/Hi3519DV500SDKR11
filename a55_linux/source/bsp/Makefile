##########################################################################################
#	prepare param
##########################################################################################
export OSDRV_DIR=$(shell pwd)
export TEE_ENABLE?=0
SECURE_BOOT_ENABLE?=0

ifeq ($(OSDRV_DIR)/../open_source, $(wildcard $(OSDRV_DIR)/../open_source))
OPEN_SOURCE=$(OSDRV_DIR)/../open_source
else ifeq ($(OSDRV_DIR)/../../open_source, $(wildcard $(OSDRV_DIR)/../../open_source))
OPEN_SOURCE=$(OSDRV_DIR)/../../open_source
else ifeq ($(OSDRV_DIR)/../../../../open_source, $(wildcard $(OSDRV_DIR)/../../../../open_source))
OPEN_SOURCE=$(OSDRV_DIR)/../../../../open_source
else ifeq ($(OSDRV_DIR)/open_source, $(wildcard $(OSDRV_DIR)/open_source))
OPEN_SOURCE=$(OSDRV_DIR)/open_source
else
$(warning "---------------------------------------------------------------------")
$(warning "   Cannot found the open_source Contents!!       ")
$(warning "---------------------------------------------------------------------")
$(error )
endif

MAKE=make
LIB_TYPE ?= musl
export LLVM ?=
export CHIP?=hi3519dv500
export CHIP_FAMILY=hi3519dv500
ifeq ($(LIB_TYPE), musl)
export OSDRV_CROSS ?= aarch64-v01c01-linux-musl
else ifeq ($(LIB_TYPE), glibc)
export OSDRV_CROSS ?= aarch64-v01c01-linux-gnu
else
$(warning "---------------------------------------------------------------------")
$(warning "   LIB_TYPE Input Is ERR!!       ")
$(warning "---------------------------------------------------------------------")
$(error )
endif

ifneq ($(LLVM), )
export CROSS_COMPILE=
else
export CROSS_COMPILE=$(OSDRV_CROSS)-
endif
export CPU_TYPE :=
export BUILD :=all
ARCH_TYPE ?= arm64
export ARCH :=arm64
export ARM_ARCH_VERSION := armv8
AMP_TYPE ?= linux
GDB_COMPILE ?= y

BOOT_MEDIA ?= emmc

ifeq ($(BOOT_MEDIA), spi)
MEDIUM_FLAG = _
endif

ifeq ($(BOOT_MEDIA), emmc)
MEDIUM_FLAG = _emmc_
endif

ifeq ($(TEE_ENABLE),1)
TEE_FLAG = tee_
endif

ifeq ($(CHIP),hi3519dv500)
REGBIN_XLSM ?=Hi3519DV500-DMEB_6L_T_DDR4_2666M-2GB_16bitx2-A55_1G.xlsm
else ifeq ($(CHIP),hi3516dv500)
REGBIN_XLSM ?=Hi3516DV500-DMEB_6L_T_DDR4_2400M-2GB_16bitx2-A55_850M.xlsm
else
$(error you must set valid CHIP: hi3519dv500 | hi3516dv500)
endif

export KERNEL_CFG:=$(CHIP)$(MEDIUM_FLAG)$(TEE_FLAG)defconfig

TOOLCHAINI_VERSION := ver100.140

TOOLCHAIN_FILE:= $(shell which $(OSDRV_CROSS)-gcc )
TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(TOOLCHAIN_FILE)))
RUNTIMELIB_DIR=$(shell dirname $(TOOLCHAIN_DIR))/$(RUNTIME_LIB)

CROSS_SPECIFIED:=y
TOOLCHAIN_RUNTIME_LIB:=runtime_lib

ifneq ($(LLVM),)
BUSYBOX_CFG ?= config_aarch64_a55_softfp_neon_llvm
else
BUSYBOX_CFG ?= config_aarch64_a55_softfp_neon
endif
export UBOOT_CONFIG:=$(CHIP)$(MEDIUM_FLAG)defconfig

BUSYBOX_VER:=busybox-1.34.1
BUSYBOX_DIR := $(OPEN_SOURCE)/busybox
TOOLCHAIN_RUNTIME_LIB_C:=lib.tgz

ATF_VER:=arm-trusted-firmware-2.7
ATF_TAR:=$(ATF_VER).tar.gz
ATF_DIR:=$(OPEN_SOURCE)/arm-trusted-firmware

KERNEL_VER:=linux-5.10.y
KERNEL_DIR:=$(OPEN_SOURCE)/linux
KERNEL_TAR:=$(KERNEL_VER).tgz

UBOOT_VER:=u-boot-2022.07
UBOOT_DIR:=$(OPEN_SOURCE)/u-boot
UBOOT_TAR:=$(UBOOT_VER).tar.bz2

SECURE_BOOT:=secureboot_release
SECURE_BOOT_TAR:=$(SECURE_BOOT).tgz
CROSS_CC :=
ifneq ($(LLVM),)
	LLVM_TOOLCHAIN_PATH:=$(shell llvm-config --bindir 2>/dev/null)
	LLVM_INSTALL_TOOL:=$(LLVM_TOOLCHAIN_PATH)/../tools/install_dynamic_basic_library.sh
	LLVM_DEFAULT_TARGET:=$(shell clang -print-target-triple 2>/dev/null)
	CROSS_CC = clang
else
	CROSS_CC = $(OSDRV_CROSS)-gcc
        GCC_INSTALL_TOOL:=$(TOOLCHAIN_DIR)/tools/install_dynamic_basic_library.sh
endif

CRAMFS_VER:= util-linux-2.37.2
CRAMFS_DIR:=$(OPEN_SOURCE)/util-linux
CRAMFS_TAR:=$(CRAMFS_VER).tar.xz

XZ_DIR:= $(OPEN_SOURCE)/xz

LZMA_DIR:=$(OPEN_SOURCE)/lzma

EUDEV_VER:= eudev-3.2.10
EUDEV_DIR:=$(OPEN_SOURCE)/eudev
EUDEV_TAR:=$(EUDEV_VER).tar.gz

MTD_DIR:=$(OPEN_SOURCE)/mtd-utils/
MTD_VER:= mtd-utils-2.1.4
MTD_TAR:=$(MTD_VER).tar.bz2
VENDOR_TOOLS:=tools/board/reg-tools-1.0.0

ZLIB_VER:= zlib-1.2.12
ZLIB_DIR:= $(OPEN_SOURCE)/zlib
ZLIB_TAR:=$(ZLIB_VER).tar.gz

SQUASHFS_VER:= squashfs-4.5
SQUASHFS_DIR:=$(OPEN_SOURCE)/squashfs
SQUASHFS_TAR:=$(SQUASHFS_VER).tar.gz

ETHTOOL_VER:= ethtool-5.15
ETHTOOL_DIR:= $(OPEN_SOURCE)/ethtool
SQUASHFS_TAR:=$(ETHTOOL_VER).tar.xz

REG_BIN:=reg_info.bin
BOOT_REG_BIN:=$(CHIP)_$(REG_BIN)
JFFS2_IMAGE_BIN_64K:=rootfs_$(CHIP)_64k.jffs2
JFFS2_IMAGE_BIN_128K:=rootfs_$(CHIP)_128k.jffs2
JFFS2_IMAGE_BIN_256K:=rootfs_$(CHIP)_256k.jffs2

EMMC_ROOTFS_SIZE ?= 96
E2FSPROGS_DIR:=$(OPEN_SOURCE)/e2fsprogs/
EXT4_TOOL:=mkfs.ext4
EXT4_IMAGE_BIN:=rootfs_$(CHIP)_$(EMMC_ROOTFS_SIZE)M.ext4
COUNT_SIZE := $(shell echo $$(( $(EMMC_ROOTFS_SIZE) * 1024 * 2 )))

REGBING_PACKAGE_VER:=RegBin-v5.2.11_linux-x86_64
REGBING_PACKAGE_FILE:=RegBin
REGBING_HEAD_MAGIC=2b8c6e1a1a6e8c2b
OPTEE_DIR:=$(OPEN_SOURCE)/optee
BOOT_IMAGE_TOOL_DIR=$(OSDRV_DIR)/tools/pc/image_tool
BOOT_IMAGE_TOOL_INPUT_DIR=$(BOOT_IMAGE_TOOL_DIR)/input
BOOT_IMAGE_TOOL_OUTPUT_FILE=$(BOOT_IMAGE_TOOL_DIR)/image/oem/boot_image.bin
TEE_IMAGE_TOOL_OUTPUT_FILE=$(BOOT_IMAGE_TOOL_DIR)/image/oem/tee_image.bin

ifeq ($(TEE_ENABLE),1)
IMAGE_TOOL_SCRIPT=$(BOOT_IMAGE_TOOL_DIR)/oem/oem_quick_build_tee.py
else ifeq ($(SECURE_BOOT_ENABLE),1)
IMAGE_TOOL_SCRIPT=$(BOOT_IMAGE_TOOL_DIR)/oem/oem_quick_build_secure_boot.py
else
IMAGE_TOOL_SCRIPT=$(BOOT_IMAGE_TOOL_DIR)/oem/oem_quick_build.py
endif

UBOOT_ENV_DIR := $(OSDRV_DIR)/tools/pc/uboot_env
ifeq ($(TEE_ENABLE),1)
ENVTXT_INFIX := _tee
else
ENVTXT_INFIX :=
endif

##########################################################################################
#	prepare regbin tools
##########################################################################################
REGBING_PACKAGE = $(shell find $(OSDRV_DIR)/tools/pc/boot_tools/ \
					-name "$(REGBING_PACKAGE_VER).tgz")
ifeq ($(REGBING_PACKAGE), )
$(warning "---------------------------------------------------------------------")
$(warning "   Cannot found the $(REGBING_PACKAGE_VER).tgz package !!       ")
$(warning "---------------------------------------------------------------------")
$(error )
endif
TARGET_XLSM := $(REGBIN_XLSM)

ifeq ($(TEE_ENABLE),0)
export PUB_BOARD:=board_$(LIB_TYPE)_$(ARCH_TYPE)
export PUB_IMAGE:=$(CHIP)$(MEDIUM_FLAG)image_$(LIB_TYPE)
export PUB_ROOTFS:=rootfs_$(LIB_TYPE)_$(ARCH_TYPE)
else
export PUB_BOARD:=board_$(LIB_TYPE)_$(ARCH_TYPE)_tee
export PUB_IMAGE:=$(CHIP)$(MEDIUM_FLAG)image_$(LIB_TYPE)_tee
export PUB_ROOTFS:=rootfs_$(LIB_TYPE)_$(ARCH_TYPE)_tee
endif

BLACK = "\e[30;1m"
RED  =  "\e[31;1m"
GREEN = "\e[32;1m"
YELLOW = "\e[33;1m"
BLUE  = "\e[34;1m"
PURPLE = "\e[35;1m"
CYAN  = "\e[36;1m"
WHITE = "\e[37;1m"
DONE    = "\033[0m"
##########################################################################################
#	set task
##########################################################################################
all: toolchain_version_check prepare boot atf kernel rootfs_prepare secure_libs busybox pctools boardtools rootfs_build teeos tee_client gslboot_build uboot_env
clean: boot_clean kernel_clean atf_clean \
       busybox_clean pctools_clean boardtools_clean \
       rootfs_clean gslboot_clean uboot_env_clean\
       secure_libs_clean teeos_clean tee_client_clean

distclean:clean pub_clean
a:=$(shell $(OSDRV_CROSS)-gcc --version)
b:=$(findstring $(TOOLCHAINI_VERSION),$(a))
c:=$(word 4, $(a))
##########################################################################################
#task [0.0]	toolchain version check
##########################################################################################
toolchain_version_check:
	@echo "---------task [0.0]    toolchain_version_check"
ifneq ($(b),$(TOOLCHAINI_VERSION))
	$(warning "---------------------------------------------------------------------")
	$(warning "       < !!! Attention:please check toolchain version !!! >          ")
	$(warning "         The current toolchain version is $(c)  ")
	$(warning "         But requested toolchain version is ($(TOOLCHAINI_VERSION))")
	$(warning "         Please make sure the toolchain version is the best match    ")
	$(warning "---------------------------------------------------------------------")
	$(shell read -p "Press Enter to continue compile; Ctrl + C to stop it !!" )
endif

##########################################################################################
#task [0.1]	prepare
##########################################################################################
prepare:toolchain_version_check
	@echo "---------task [0.1]    prepare"
	mkdir $(OSDRV_DIR)/pub/$(PUB_IMAGE) -p
	mkdir $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -p
	mkdir $(OSDRV_DIR)/pub/bin/pc -p

##########################################################################################
#task [0.2]	regbin_prepare
##########################################################################################
regbin_prepare:
	@echo "---------task [0.2]    regbin_prepare"
	@echo "---------TARGET_XLSM: $(TARGET_XLSM)"
	rm $(OSDRV_DIR)/tools/pc/boot_tools/$(REGBING_PACKAGE_FILE) -rf
	tar xzf $(REGBING_PACKAGE) -C $(OSDRV_DIR)/tools/pc/boot_tools
	chmod 777 $(OSDRV_DIR)/tools/pc/boot_tools/$(REGBING_PACKAGE_FILE)/regbin
	cp $(OSDRV_DIR)/tools/pc/boot_tools/$(TARGET_XLSM) $(OSDRV_DIR)/tools/pc/boot_tools/$(REGBING_PACKAGE_FILE)
	cd $(OSDRV_DIR)/tools/pc/boot_tools/$(REGBING_PACKAGE_FILE) && ./regbin $(TARGET_XLSM) $(BOOT_REG_BIN) -magic $(REGBING_HEAD_MAGIC);cd -
	mv $(OSDRV_DIR)/tools/pc/boot_tools/$(REGBING_PACKAGE_FILE)/$(BOOT_REG_BIN) $(OSDRV_DIR)/tools/pc/boot_tools
	rm $(OSDRV_DIR)/tools/pc/boot_tools/$(REGBING_PACKAGE_FILE) -rf

notools_prepare:
	pushd $(OSDRV_DIR)/pub;tar xzf $(PUB_ROOTFS).tgz;popd
##########################################################################################
#task [1]	build uboot64
##########################################################################################
boot: prepare regbin_prepare
	@echo -e $(GREEN)"---------task [1]	build boot64  $(UBOOT_DIR)"   $(DONE)
	$(MAKE) -C $(UBOOT_DIR) CHIP=$(CHIP) CHIP_FAMILY=$(CHIP_FAMILY) BOOT_MEDIA=$(BOOT_MEDIA) LLVM=$(LLVM) LIB_TYPE=$(LIB_TYPE) \
			ARCH_TYPE=$(ARCH_TYPE) all

boot_clean:
	rm $(UBOOT_DIR)/$(UBOOT_VER) -rf
	rm $(OSDRV_DIR)/tools/pc/boot_tools/*$(REG_BIN) -rf
	$(MAKE) -C $(UBOOT_DIR) LLVM=$(LLVM) clean
##########################################################################################
#task [2]	build atf
##########################################################################################
atf: prepare
	@echo -e $(GREEN)"---------task [2] build atf"  $(DONE)
	$(MAKE) -C $(ATF_DIR) CHIP=$(CHIP) CHIP_FAMILY=$(CHIP_FAMILY) BOOT_MEDIA=$(BOOT_MEDIA) LLVM=$(LLVM) LIB_TYPE=$(LIB_TYPE) \
				TEE_ENABLE=$(TEE_ENABLE) ARCH_TYPE=$(ARCH_TYPE) all
	cp $(ATF_DIR)/$(ATF_VER)/build/$(CHIP_FAMILY)/release/bl31.bin $(OSDRV_DIR)/pub/$(PUB_IMAGE)

atf_clean:
	rm -rf $(ATF_DIR)/$(ATF_VER)

##########################################################################################
#task [3]	build kernel
##########################################################################################
kernel: prepare teeos
	@echo -e $(GREEN)"---------task [3] build kernel"  $(DONE)
	$(MAKE) -C $(KERNEL_DIR) CHIP=$(CHIP) BOOT_MEDIA=$(BOOT_MEDIA) LLVM=$(LLVM) LIB_TYPE=$(LIB_TYPE) TEE_ENABLE=$(TEE_ENABLE) \
				ARCH_TYPE=$(ARCH_TYPE) all
	cp $(KERNEL_DIR)/$(KERNEL_VER)/arch/$(ARCH_TYPE)/boot/uImage-fdt $(OSDRV_DIR)/pub/$(PUB_IMAGE)/
kernel_clean:
	rm $(KERNEL_DIR)/$(KERNEL_VER) -rf
	rm $(KERNEL_DIR)/$(KERNEL_TAR) -rf

##########################################################################################
#task [4] build secure libs
##########################################################################################
secure_libs:
	@echo -e $(GREEN)"---------task [4] build secure libs"  $(DONE)
	pushd $(OSDRV_DIR)/components/secure_c/src;make clean;CC=gcc make lib CHECK_OPTION=check; popd
	mkdir $(OSDRV_DIR)/components/secure_c/libs -p
	cp $(OSDRV_DIR)/components/secure_c/lib/libsecurec.a $(OSDRV_DIR)/components/secure_c/libs/libsecurec-pc.a
	pushd $(OSDRV_DIR)/components/secure_c/src;make clean;CC=$(CROSS_CC) make lib CHECK_OPTION=check;popd
	cp $(OSDRV_DIR)/components/secure_c/lib/libsecurec.a $(OSDRV_DIR)/components/secure_c/libs/libsecurec-board.a
	pushd $(OSDRV_DIR)/components/secure_c/src;make clean;popd

secure_libs_clean:
	pushd $(OSDRV_DIR)/components/secure_c/src;make clean; popd
	rm $(OSDRV_DIR)/components/secure_c/libs -rf
##########################################################################################
#task [5]	prepare rootfs
##########################################################################################
rootfs_prepare: prepare
	@echo -e $(GREEN)"---------task [5] prepare rootfs "  $(DONE)
ifeq ($(OSDRV_DIR)/pub/$(PUB_ROOTFS), $(wildcard $(OSDRV_DIR)/pub/$(PUB_ROOTFS)))
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS); chmod +w usr/bin; chmod +w usr/sbin; chmod +w sbin; popd
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)* -rf
endif
	make -C $(OSDRV_DIR)/rootfs_scripts/ all
	cp -r $(OSDRV_DIR)/rootfs_scripts/rootfs/ $(OSDRV_DIR)/pub/$(PUB_ROOTFS)
ifneq ($(LLVM),)
	$(LLVM_INSTALL_TOOL) TOOLCHAIN_PATH=$(LLVM_TOOLCHAIN_PATH) INSTALL_PATH=$(OSDRV_DIR)/pub/$(PUB_ROOTFS) TARGET=$(LLVM_DEFAULT_TARGET)
else
	echo $(TOOLCHAIN_DIR)/$(TOOLCHAIN_RUNTIME_LIB)   $(TOOLCHAIN_RUNTIME_LIB_C)
	$(GCC_INSTALL_TOOL) $(TOOLCHAIN_DIR) $(OSDRV_DIR)/pub/$(PUB_ROOTFS)
endif

##########################################################################################
#task [6]	build busybox
##########################################################################################
busybox: prepare
	@echo -e $(GREEN)"---------task [6] build busybox "  $(DONE)
	$(MAKE) -C $(BUSYBOX_DIR) CHIP=$(CHIP) BOOT_MEDIA=$(BOOT_MEDIA) LLVM=$(LLVM)  LIB_TYPE=$(LIB_TYPE) \
				ARCH_TYPE=$(ARCH_TYPE) BUSYBOX_CFG=$(BUSYBOX_CFG) all

busybox_clean:
	$(MAKE)  LLVM=$(LLVM) -C $(BUSYBOX_DIR) clean

##########################################################################################
#task [7]	build pc tools
##########################################################################################
pctools: prepare secure_libs
	@echo -e $(GREEN)"---------task [7] build tools which run on pc"  $(DONE)
	$(MAKE) -C $(ZLIB_DIR) zlib_pc
	$(MAKE) -C $(MTD_DIR) mtd_pc

	cp $(MTD_DIR)/out/pc/mkfs.jffs2 $(OSDRV_DIR)/pub/bin/pc
	cp $(MTD_DIR)/out/pc/mkfs.ubifs $(OSDRV_DIR)/pub/bin/pc
	cp $(MTD_DIR)/out/pc/ubinize $(OSDRV_DIR)/pub/bin/pc

	$(MAKE) -C $(CRAMFS_DIR)
	cp $(CRAMFS_DIR)/out/pc/sbin/mkfs.cramfs $(OSDRV_DIR)/pub/bin/pc
	$(MAKE) -C $(XZ_DIR)
	$(MAKE) -C $(SQUASHFS_DIR)
	cp $(SQUASHFS_DIR)/out/pc/mksquashfs $(OSDRV_DIR)/pub/bin/pc
	$(MAKE) -C $(E2FSPROGS_DIR) e2fsprogs_pc
	cp $(E2FSPROGS_DIR)/out/pc/sbin/$(EXT4_TOOL) $(OSDRV_DIR)/pub/bin/pc
	cp $(OSDRV_DIR)/components/secure_c/libs/libsecurec-pc.a $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100
	cp $(OSDRV_DIR)/components/secure_c/include/*.h $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100
	$(MAKE) -C $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100
	cp $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100/nand_product $(OSDRV_DIR)/pub/bin/pc

pctools_clean:
	$(MAKE) -C $(ZLIB_DIR) distclean
	$(MAKE) -C $(MTD_DIR) distclean
	$(MAKE) -C $(SQUASHFS_DIR) distclean
	$(MAKE) -C $(CRAMFS_DIR) distclean
	$(MAKE) -C $(XZ_DIR) distclean
	$(MAKE) -C $(E2FSPROGS_DIR)  distclean
	$(MAKE) -C $(OSDRV_DIR)/tools/pc/nand_production/fmc_nand_ecc_product_v100 clean


##########################################################################################
#task [8]	build board tools
##########################################################################################
boardtools: rootfs_prepare secure_libs
	@echo -e $(GREEN)"---------task [8] build tools which run on board "  $(DONE)
	$(MAKE) -C $(VENDOR_TOOLS) LLVM=$(LLVM) OSDRV_CROSS=$(OSDRV_CROSS) all

	$(MAKE) -C $(ZLIB_DIR) LLVM=$(LLVM) OSDRV_CROSS=$(OSDRV_CROSS) zlib_board
	$(MAKE) -C $(EUDEV_DIR) LLVM=$(LLVM) OSDRV_CROSS=$(OSDRV_CROSS) -j1
	cp -af $(EUDEV_DIR)/rootfs/. $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/

	$(MAKE) -C $(MTD_DIR) LLVM=$(LLVM) OSDRV_CROSS=$(OSDRV_CROSS) mtd_board
	cp $(MTD_DIR)/out/board/bin/* $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)

	$(MAKE) -C $(ETHTOOL_DIR) LLVM=$(LLVM) OSDRV_CROSS=$(OSDRV_CROSS) all

	$(MAKE) -C $(E2FSPROGS_DIR) LLVM=$(LLVM) OSDRV_CROSS=$(OSDRV_CROSS) e2fsprogs_board
	cp $(E2FSPROGS_DIR)/out/board/sbin/mkfs.ext2 $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -rf;
	cp $(E2FSPROGS_DIR)/out/board/sbin/mkfs.ext3 $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -rf;
	cp $(E2FSPROGS_DIR)/out/board/sbin/mkfs.ext4 $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -rf;
	cp $(E2FSPROGS_DIR)/out/board/sbin/mke2fs $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -rf;
	cp $(ETHTOOL_DIR)/bin/sbin/ethtool $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -rf;

boardtools_clean:
	$(MAKE) -C $(OSDRV_DIR)/$(VENDOR_TOOLS) clean
	$(MAKE) -C $(EUDEV_DIR) clean
	$(MAKE) -C $(ZLIB_DIR) clean
	$(MAKE) -C $(MTD_DIR) distclean
	$(MAKE) -C $(E2FSPROGS_DIR) distclean
	$(MAKE) -C $(VENDOR_TOOLS) clean
	$(MAKE) -C $(ETHTOOL_DIR) clean

##########################################################################################
#task [9]	build rootfs
##########################################################################################
rootfs_build: rootfs_notools_build tee_client

rootfs_notools_build: rootfs_prepare busybox boardtools pctools
	@echo -e $(GREEN)"---------task [9] build rootfs"  $(DONE)
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/dev/* -rf
	
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);ln -s sbin/init init;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod go-x * -R;chmod go-w * -R;chmod go-r * -R;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 4755 bin/su;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 4555 bin/login usr/bin/passwd;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 2555 usr/bin/wall;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 1777 tmp;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 755 bin etc lib opt proc usr usr/bin usr/bin/crontab sbin sbin/arp usr/lib var;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 750 boot etc/init.d etc/init.d/*;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 644 etc/group etc/inittab etc/mtab etc/passwd etc/profile etc/protocols etc/services etc/shadow root/.profile;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 640 var/run/utmp;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);chmod 550 bin/mount bin/netstat sbin/route;popd;
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS);$(OSDRV_CROSS)-strip ./lib/*;popd;
	pushd $(OSDRV_DIR)/pub/bin/pc;fakeroot ./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x40000 -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(JFFS2_IMAGE_BIN_256K);popd
	pushd $(OSDRV_DIR)/pub/bin/pc;fakeroot ./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x20000 -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(JFFS2_IMAGE_BIN_128K);popd
	pushd $(OSDRV_DIR)/pub/bin/pc;fakeroot ./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x10000 -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(JFFS2_IMAGE_BIN_64K);popd
	cp $(OSDRV_DIR)/tools/pc/ubi_sh/mkubiimg.sh $(OSDRV_DIR)/pub/$(PUB_IMAGE)
	chmod +x $(OSDRV_DIR)/pub/$(PUB_IMAGE)/mkubiimg.sh
	# build the pagesize = 2k, blocksize = 128k, part_size = 32M #
	pushd $(OSDRV_DIR)/pub/$(PUB_IMAGE);fakeroot ./mkubiimg.sh $(CHIP) 2k 128k $(OSDRV_DIR)/pub/$(PUB_ROOTFS) 32M $(OSDRV_DIR)/pub/bin/pc;popd
	# build the pagesize = 4k, blocksize = 256k, part_size = 50M #
	pushd $(OSDRV_DIR)/pub/$(PUB_IMAGE);fakeroot ./mkubiimg.sh $(CHIP) 4k 256k $(OSDRV_DIR)/pub/$(PUB_ROOTFS) 50M $(OSDRV_DIR)/pub/bin/pc;popd
	rm $(OSDRV_DIR)/pub/$(PUB_IMAGE)/mkubiimg.sh

	dd if=/dev/zero of=$(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(EXT4_IMAGE_BIN) bs=512 count=$(COUNT_SIZE)
	$(OSDRV_DIR)/pub/bin/pc/$(EXT4_TOOL) $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(EXT4_IMAGE_BIN)
	pushd $(E2FSPROGS_DIR)/out/pc/contrib;fakeroot sh -c "./populate-extfs.sh $(OSDRV_DIR)/pub/$(PUB_ROOTFS) $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(EXT4_IMAGE_BIN)";popd
	find $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ -name '*svn' | xargs rm -rf
	pushd $(OSDRV_DIR)/pub;tar czf $(PUB_ROOTFS).tgz $(PUB_ROOTFS);popd

rootfs_clean:
	make -C $(OSDRV_DIR)/rootfs_scripts/ clean
ifeq ($(OSDRV_DIR)/pub/$(PUB_ROOTFS), $(wildcard $(OSDRV_DIR)/pub/$(PUB_ROOTFS)))
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS); chmod +w usr/bin; chmod +w usr/sbin; chmod +w sbin; popd
endif
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ -rf

include $(OPTEE_DIR)/Makefile.param
OPTEE_OS_DIR=$(OPTEE_DIR)/$(OPTEE_OS_VER)
OPTEE_CLIENT_DIR=$(OPTEE_DIR)/$(OPTEE_CLIENT_VER)
OPTEE_OS_EXPORT_DIR=$(OPTEE_OS_DIR)/out/arm-plat-vendor

##########################################################################################
#task [10]	build teeos_build
##########################################################################################
teeos:prepare
ifeq ($(TEE_ENABLE), 1)
	@echo -e $(GREEN)"---------task [10.1] build teeos"  $(DONE)
	$(MAKE) -C $(OPTEE_DIR) CHIP=$(CHIP) LIB_TYPE=$(LIB_TYPE) optee_os
endif

teeos_clean:
ifeq ($(TEE_ENABLE), 1)
	$(MAKE) -C $(OPTEE_DIR) optee_os_clean
endif

tee_client: prepare rootfs_prepare
ifeq ($(TEE_ENABLE), 1)
	@echo -e $(GREEN)"---------task [10.2] build tee_client"  $(DONE)
	$(MAKE) -C $(OPTEE_DIR) CHIP=$(CHIP) LIB_TYPE=$(LIB_TYPE) optee_client
	cp -af $(OPTEE_CLIENT_DIR)/out/tee-supplicant/tee-supplicant $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin/
	cp -af $(OPTEE_CLIENT_DIR)/out/libckteec/libckteec.so.* $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/lib64/
	cp -af $(OPTEE_CLIENT_DIR)/out/libteec/libteec.so.* $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/lib64/
endif

tee_client_clean:
ifeq ($(TEE_ENABLE), 1)
	$(MAKE) -C $(OPTEE_DIR) optee_client_clean
endif

##########################################################################################
#task [11]	build boot_image.bin
##########################################################################################
gslboot_build:regbin_prepare boot teeos atf prepare
	@echo -e $(GREEN)"---------task [11] build boot_image.bin"  $(DONE)
	pushd $(OSDRV_DIR)/components/gsl; make clean; make CHIP=$(CHIP) LLVM=$(LLVM) ;popd
	cp $(OSDRV_DIR)/tools/pc/boot_tools/$(BOOT_REG_BIN) $(BOOT_IMAGE_TOOL_INPUT_DIR)/reg_info.bin;
	cp $(UBOOT_DIR)/$(UBOOT_VER)/u-boot-$(CHIP).bin $(BOOT_IMAGE_TOOL_INPUT_DIR)/u-boot-original.bin;
	cp $(OSDRV_DIR)/components/gsl/pub/gsl.bin $(BOOT_IMAGE_TOOL_INPUT_DIR);
ifeq ($(TEE_ENABLE), 1)
	@echo -e $(GREEN)"---------Secure image"  $(DONE)
	cp $(OPTEE_OS_EXPORT_DIR)/core/tee.bin $(BOOT_IMAGE_TOOL_INPUT_DIR);
	mv $(OSDRV_DIR)/pub/$(PUB_IMAGE)/bl31.bin $(BOOT_IMAGE_TOOL_INPUT_DIR);
endif
	pushd $(BOOT_IMAGE_TOOL_DIR); python $(IMAGE_TOOL_SCRIPT);popd
	cp $(BOOT_IMAGE_TOOL_OUTPUT_FILE) $(OSDRV_DIR)/pub/$(PUB_IMAGE)/boot_image.bin;
ifeq ($(TEE_ENABLE), 1)
	cp $(TEE_IMAGE_TOOL_OUTPUT_FILE) $(OSDRV_DIR)/pub/$(PUB_IMAGE)/tee_image.bin;
endif
	@echo -e $(GREEN)"---------finish osdrv work"  $(DONE)

gslboot_clean:
	pushd $(OSDRV_DIR)/components/gsl; make clean; popd
	rm $(BOOT_IMAGE_TOOL_DIR)/image -rf
	rm $(BOOT_IMAGE_TOOL_DIR)/oem/tmp -rf
	find $(BOOT_IMAGE_TOOL_DIR) -name  '*.pyc'| xargs rm -rf
	find $(BOOT_IMAGE_TOOL_DIR) -name  '__pycache__'| xargs rm -rf
	find $(BOOT_IMAGE_TOOL_INPUT_DIR) -not -path "$(BOOT_IMAGE_TOOL_INPUT_DIR)/cfct_table*.bin" -name "*.bin" | xargs rm -rf

##########################################################################################
#task [12]	build u-boot env
##########################################################################################
uboot_env: prepare
	@echo -e $(GREEN)"---------task [12] build u-boot env"  $(DONE)
ifeq ($(BOOT_MEDIA),emmc)
	make -C $(UBOOT_ENV_DIR) ENVTXT=$(UBOOT_ENV_DIR)/env_text/$(CHIP)/emmc$(ENVTXT_INFIX)_env.txt
	@mv $(UBOOT_ENV_DIR)/env.bin $(OSDRV_DIR)/pub/$(PUB_IMAGE)/emmc$(ENVTXT_INFIX)_env.bin
	@mv $(UBOOT_ENV_DIR)/burn_table.xml $(OSDRV_DIR)/pub/$(PUB_IMAGE)/emmc$(ENVTXT_INFIX)_burn_table.xml
else
	make -C $(UBOOT_ENV_DIR) ENVTXT=$(UBOOT_ENV_DIR)/env_text/$(CHIP)/nand$(ENVTXT_INFIX)_env.txt
	@mv $(UBOOT_ENV_DIR)/env.bin $(OSDRV_DIR)/pub/$(PUB_IMAGE)/nand$(ENVTXT_INFIX)_env.bin
	@mv $(UBOOT_ENV_DIR)/burn_table.xml $(OSDRV_DIR)/pub/$(PUB_IMAGE)/nand$(ENVTXT_INFIX)_burn_table.xml
	make -C $(UBOOT_ENV_DIR) ENVTXT=$(UBOOT_ENV_DIR)/env_text/$(CHIP)/nor$(ENVTXT_INFIX)_env.txt
	@mv $(UBOOT_ENV_DIR)/env.bin $(OSDRV_DIR)/pub/$(PUB_IMAGE)/nor$(ENVTXT_INFIX)_env.bin
	@mv $(UBOOT_ENV_DIR)/burn_table.xml $(OSDRV_DIR)/pub/$(PUB_IMAGE)/nor$(ENVTXT_INFIX)_burn_table.xml
endif
uboot_env_clean:
	make -C $(OSDRV_DIR)/tools/pc/uboot_env/ clean

##########################################################################################
#task [13]	clean pub
##########################################################################################
pub_clean:
	rm $(OSDRV_DIR)/pub/* -rf

