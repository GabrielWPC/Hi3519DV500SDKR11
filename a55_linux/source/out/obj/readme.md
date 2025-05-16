## Prepare
Please build kernel first(Refer to smp/a55_linux/source/bsp/readme_xx.txt), build drivers need KERNEL_ROOT,

## Build
### build drivers as modules
1. Use `make -j` to rebuild all drivers as modules, and build all sample/tools after that.
2. If kernel config is changed, all drivers must be rebuild.

### build drivers to kernel
1. Change Makefile.config to select which driver is need to build to kernel.
2. Use `make BUILD_DRIVER_TO_KERNEL=y -j` to build selected drivers to kernel.

#### Note
1. If some drivers are build to the kernel, the corresponding module param need to be configured from the bootargs of the U-boot. And for avoid conflicts, the module parameter mmz= of ot_mmz is changed to mmz_zones in bootargs.
2. vo and vo_dev cannot be built into the kernel. Related functions are still under development.
