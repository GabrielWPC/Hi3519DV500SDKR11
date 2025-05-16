1.How to use Makefile of directory bsp:
	1) Since there are many dependency components for compiling a ROOTFS in Makefile, it is not guaranteed that a separately compiled ROOTFS is available. "make all" compiler is recommended.
	2) uboot, kernel can be compiled separately.
	3) This readme describes hi3519dv500 or hi3516dv500.
	4) The compilation scripts in this directory support the use of aarch64-v01c01-linux-musl and aarch64-v01c01-linux-gnu 64bit toolchains for compilation;
	5) The aarch64-v01c01-linux-musl toolchain corresponds to the musl library, and the aarch64-v01c01-linux-gnu toolchain corresponds to the glibc library
	6) During overall compilation, if you need to use the musl library toolchain, please configure LIB_ TYPE=musl; To use the glibc library toolchain, please configure LIB_ TYPE=glibc;
	7) During separating compilation. If you need to use the musl library toolchain, please configure CROSS_ COMPILE=aarch64-v01c01-linux-musl -, to use the glibc library toolchain, please configure CROSS_ COMPILE=aarch64-v01c01-linux-gnu-;
	8) Taking the musl cross toolchain compilation command as an example.
(1)Compile the entire bsp directory:
	make all
	Default Compilation:
	make BOOT_MEDIA=emmc  LIB_TYPE=musl CHIP=hi3519dv500 all

	Compilation parameter description：
        1)BOOT_MEDIA selects the emmc startup method for compilation, and can choose the spi (spi nor and spi nand) startup method for compilation, which is BOOT_ MEDIA=spi;
        2)LIB_TYPE defaults to musl compilation, and can choose glibc compilation, which is LIB_TYPE=glibc；
        3)CHIP defaults to hi3519dv500 and can be compiled with hi3516dv500, which means CHIP=hi3516dv500
(2)Clear all compiled files under bsp directory:
	make clean
(3)Completely remove all compiled files under bsp directory, and the generated images:
	make distclean

The following compilation command takes hi3519dv500 as an example.
(4)Separately compile kernel:
    a. Enter the top directory the kernel source code, do the following:
		tar xf linux-5.10.tar.gz
		mv linux-5.10 linux-5.10.y
		cd linux-5.10.y
		patch -p1 < ../linux-5.10.patch
		make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_defconfig (or make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_defconfig) (only for spi nor/spinand flash startup)
		(eMMC starts by doing the following: make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_emmc_defconfig (or make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_emmc_defconfig))
		make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- menuconfig
		make ARCH=arm64 CROSS_COMPILE=aarch64-v01c01-linux-musl- uImage
    b. Enter the top directory:open_source/arm-trusted-firmware, do the following:
	    	make all
		bl31.bin file in the dir of bsp/../open_source/arm-trusted-firmware/arm-trusted-firmware-2.7/build/hi3519dv500/release is atf's bl31.bin, need burn to single mtd partion in falsh.

(5)Separately compile non-sercure boot image:
	Note: The package is released on the DEMO board by default. If the customer's board is inconsistent with the DEMB board, you need to modify the uboot table according to the customer's own board environment. Otherwise, uboot may not start or other problems may occur. spi flash demoboard and emmc demoboard use the same boot table
	a. Enter the top directory of boot source code, do the following:
		tar xf u-boot-2022.07.tar.bz2
		cd u-boot-2022.07
		patch -p1 < ../u-boot-2022.07.patch
		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_defconfig (or make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_defconfig) (only for spi nor/spinand flash startup)
		(eMMC starts by doing the following:make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3519dv500_emmc_defconfig (or make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- hi3516dv500_emmc_defconfig))
		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- menuconfig
		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- -j 20
		make -C ../../gzip/
		cp ../../gzip/bin/gzip arch/arm/cpu/armv8/hi3519dv500/hw_compressed/ -rf
		chmod +x arch/arm/cpu/armv8/hi3519dv500/hw_compressed/gzip
		make ARCH=arm CROSS_COMPILE=aarch64-v01c01-linux-musl- u-boot-z.bin
		Under the open_source/uboot/u-boot-2022.07 of u-boot-hi3519dv500.bin is the sub image of non-sercure boot image.

	b. Generate file of the corresponding platform:
		Enter the bsp/tools/pc/boot_tools/ directory form Windows OS, open the corresponding Excel file, click the "Generate reg bin file" button in the main tag, and generate reg_info.bin.

    	c. please enter the bsp directory and perform following steps to make the non-sercure boot image, :
	        cd components/gsl
	        make clean
	        make
	        cd -
	        cp components/gsl/pub/gsl.bin ./tools/pc/image_tool/input
	        cp ../../../../open_source/u-boot/u-boot-2022.07/u-boot-hi3519dv500.bin ./tools/pc/image_tool/input/u-boot-original.bin
	        cp tools/pc/boot_tools/reg_info.bin ./tools/pc/image_tool/input
	        cd ./tools/pc/image_tool
	        python oem/oem_quick_build.py
		Under the tools/pc/image_tool/image/oem directory, tee_boot_image.bin is the "Boot Image" for non-sercure boot solution.

(6)Build file system image(take hi3519dv500 for example):
    After compiling the entire bsp directory, the compiled file system and default file system images will be generated in the bsp/pub/directory. If the specifications of the startup media used by the user do not match the default file system images, the file system images can be made according to the specifications of the startup media on the single board.

	Filesystem images of jffs2 format is available for spi flash. While making jffs2 image or squashfs image, you need to specify the spi flash block size.


	The flash block size will be printed when uboot runs. run mkfs.jffs2 first to get the right parameters from it's printed information.
	Here the block size is 64KB, for example:
	bsp/pub/bin/pc/mkfs.jffs2 -d bsp/pub/rootfs_musl_arm64 -l -e 0x10000 -o bsp/pub/rootfs_hi3519dv500_64k.jffs2

	UBIFS format image is available for nand flash. Pagesize and blocksize of nand flash are needed when making UBIFS image.
	This information will be printed when uboot starts.The following example is 2KB page size, 128KB block size:
	./bsp/tools/pc/ubi_sh/mkubiimg.sh hi3519dv500 2k 128k bsp/pub/rootfs_musl_arm64 32M bsp/pub/bin/pc

	Emmc uses ext4 image format, taking 96MB image as an example:(need prepare rootfs_hi3519dv500_96M.ext4、rootfs_musl_arm64、populate-extfs.sh)
	dd if=/dev/zero of=bsp/pub/hi3519dv500_emmc_image_musl/rootfs_hi3519dv500_96M.ext4 bs=512 count=196608
	note:(196608 = 96 * 1024 * 1024 / 512)
	./bsp/pub/bin/pc/mkfs.ext4 bsp/pub/hi3519dv500_emmc_image_musl/rootfs_hi3519dv500_96M.ext4
	enter dir: open_source/e2fsprogs/out/pc/contrib:
	./populate-extfs.sh ../../../../../smp/a55_linux/source/bsp/pub/rootfs_musl_arm64 ../../../../../smp/a55_linux/source/bsp/pub/hi3519dv500_emmc_image_musl/rootfs_hi3519dv500_96M.ext4

2.Note
(1)Executable files in Linux may become non-executable after copying them to somewhere else under Windows, and result in souce code cannot be compiled. Many symbolic link files will be generated in the souce package after compiling the u-boot or the kernel. The volume of the source package will become very big, because all the symbolic link files are turning out to be real files in Windows. So, DO NOT copy source package in Windows.
(2)The HI3519DV500 has floating-point unit and neon. The libraries in the file system are compiled with har
d floating point and neon compatible with the soft floating point call interface. Therefore, you need to ad
d the "-mcpu=cortex-a55" options to Makefile during compilation of all board codes.
