1.bsp 顶层 Makefile 使用说明
	1)由于Makefile中文件系统编译依赖组件较多，不能保证单独编译的文件系统可用，建议采用make all编译；
	2)可单独编译uboot，kernel;
	3)本readme针对hi3519dv500/hi3516dv500进行说明;
	4)本目录下的编译脚本支持选用aarch64-v01c01-linux-musl、aarch64-v01c01-linux-gnu 64bit工具链进行编译;
	5)其中aarch64-v01c01-linux-musl工具链对应musl库，aarch64-v01c01-linux-gnu工具链对应glibc库;
	6)总体编译时，如需使用musl库工具链, 请配置LIB_TYPE=musl; 如需使用glibc库工具链,请配置LIB_TYPE=glibc;
	7)单独编译时，如需使用musl库工具链, 请配置CROSS_COMPILE=aarch64-v01c01-linux-musl-,如需使用glibc库工具链, 请配置CROSS_COMPILE=aarch64-v01c01-linux-gnu-;
	8)以下以musl交叉工具链编译命令为例.

(1)编译整个bsp目录：
	make all
	默认编译:
	make BOOT_MEDIA=emmc LIB_TYPE=musl CHIP=hi3519dv500 all

	编译参数说明：
	1)BOOT_MEDIA默认选择emmc启动方式编译，可选择spi(spi nor and spi nand)启动方式编译,即BOOT_MEDIA=spi;
	2)LIB_TYPE默认为musl编译, 可选择glibc编译，即LIB_TYPE=glibc；
	3)CHIP默认为hi3519dv500，可选择hi3516dv500编译,即CHIP=hi3516dv500。

(2)清除整个bsp目录的编译文件：
	make  clean

(3)彻底清除整个bsp目录的编译中间文件：
	make distclean

(4)单独编译kernel image：
        a.进入open_source/linux目录，执行以下操作
		tar xf linux-5.10.tar.gz
		mv linux-5.10 linux-5.10.y
		cd linux-5.10.y
		patch -p1 < ../linux-5.10.patch
		make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_defconfig (或 make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_defconfig) (仅用于 spi nor/nand flash)
		(emmc启动时执行如下操作: make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_emmc_defconfig (或 make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_emmc_defconfig))
		make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- menuconfig
		make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- uImage -j 20

	b.进入open_source/arm-trusted-firmware目录
		make all
		在open_source/arm-trusted-firmware/arm-trusted-firmware-2.7/build/hi3519dv500/release目录下，生成atf的bl31.bin镜像,需要烧写至flash独立分区.

(5)单独编译非安全启动镜像：
	注意：发布包默认按照 DEMO 板的配置发布，如果客户单板和 DEMO 板不一致，需要根据客户自己的单板环境修改uboot表格才能使用，否则可能导致uboot无法启动或者其他问题。spi flash与emmc demo板启动采用同一个表格。
	a.进入uboot源代码目录后，生成原始u-boot镜像:
		tar xf u-boot-2022.07.tar.bz2
		cd u-boot-2022.07
		patch -p1 < ../u-boot-2022.07.patch
		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_defconfig (或 make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_defconfig) (仅用于 spi nor/nand flash)
		(emmc启动时执行如下操作: make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_emmc_defconfig (或 make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_emmc_defconfig))
		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- menuconfig
		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- -j 20
		make -C ../../gzip/
		cp ../../gzip/bin/gzip arch/arm/cpu/armv8/hi3519dv500/hw_compressed/ -rf
		chmod +x arch/arm/cpu/armv8/hi3519dv500/hw_compressed/gzip

		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- u-boot-z.bin
		open_source/uboot/u-boot-2022.07下生成的u-boot-hi3519dv500.bin 即为非安全镜像的子镜像

    	b. 生成平台表格文件：Windows下进入到bsp/tools/pc/boot_tools/目录下打开对应单板的Excel文件,在main标签中点击"Generate reg bin file"按钮,生成reg_info.bin


	c.进入bsp目录，执行以下操作来制作非安全镜像：
		cd components/gsl
		make clean
		make
		cd -
		cp components/gsl/pub/gsl.bin ./tools/pc/image_tool/input
		cp ../../../../open_source/u-boot/u-boot-2022.07/u-boot-hi3519dv500.bin ./tools/pc/image_tool/input/u-boot-original.bin
		cp tools/pc/boot_tools/reg_info.bin ./tools/pc/image_tool/input
		cd ./tools/pc/image_tool
		python oem/oem_quick_build.py
		tools/pc/image_tool/image/oem/下生成的boot_image.bin即为“非安全启动”使用的Boot Image。

(6)制作文件系统镜像(以hi3519dv500为例)：
在bsp/pub/中有已经编译好的文件系统，因此无需再重复编译文件系统，只需要根据单板上启动介质的规格型号制作文件系统镜像即可。

	spi flash使用jffs2格式的镜像，制作jffs2镜像时，需要用到spi flash的块大小。这些信息会在uboot启动时会打印出来。建议使用时先直接运行mkfs.jffs2工具，根据打印信息填写相关参数。下面以块大小为64KB为例：
	bsp/pub/bin/pc/mkfs.jffs2 -d bsp/pub/rootfs_musl_arm64 -l -e 0x10000 -o bsp/pub/rootfs_hi3519dv500_64k.jffs2

	nand flash使用ubifs格式的镜像，制作ubifs镜像时，需要用到nand flash的pagesize和blocksize。这些信息会在uboot启动时会打印出来。
	下面以2KB pagesize、128KB block size为例：
	./bsp/tools/pc/ubi_sh/mkubiimg.sh hi3519dv500 2k 128k bsp/pub/rootfs_musl_arm64 32M bsp/pub/bin/pc

	emmc 使用ext4格式的镜像：以96MB镜像为例:(需要准备rootfs_hi3519dv500_96M.ext4、rootfs_musl_arm64、populate-extfs.sh)
	dd if=/dev/zero of=bsp/pub/hi3519dv500_emmc_image_musl/rootfs_hi3519dv500_96M.ext4 bs=512 count=196608
	备注:(196608 = 96 *1024 * 1024 / 512)
	./bsp/pub/bin/pc/mkfs.ext4 bsp/pub/hi3519dv500_emmc_image_musl/rootfs_hi3519dv500_96M.ext4
	进入 open_source/e2fsprogs/out/pc/contrib:
	./populate-extfs.sh ../../../../../smp/a55_linux/source/bsp/pub/rootfs_musl_arm64 ../../../../../smp/a55_linux/source/bsp/pub/hi3519dv500_emmc_image_musl/rootfs_hi3519dv500_96M.ext4

2.注意事项
(1)在windows下复制源码包时，linux下的可执行文件可能变为非可执行文件，导致无法编译使用；u-boot或内核下编译后，会有很多符号链接文件，

在windows下复制这些源码包, 会使源码包变的巨大，因为linux下的符号链接文件变为windows下实实在在的文件，导致源码包膨胀。
因此使用时请注意不要在windows下复制源代码包。
(2)HI3519DV500具有浮点运算单元和neon。文件系统中的库是采用软浮点和neon编译而成，因此请用户注意，所有HI3519DV500板端代码编译时需要在Makefile里面添加选项-mcpu=cortex-a55。
