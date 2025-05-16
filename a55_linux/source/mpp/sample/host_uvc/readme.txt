To reduce the CPU usage when playing YUV, add the line below At the beginning of the file  open_source/linux/linux-5.10.y/drivers/media/usb/uvc/uvc_video.c :
#define CONFIG_DMA_NONCOHERENT


When the NV21 format is played, the screen image flickers and the memcpy error is reported on serial port. This is because the kernel does not support NV21 in UVC host driver.
Modify as follows:

1. drivers\media\usb\uvc\uvcvideo.h
add a macro before #define UVC_GUID_FORMAT_NV12 : 
#define UVC_GUID_FORMAT_NV21 \
	{ 'N', 'V', '2', '1', 0x00, 0x00, 0x10, 0x00, \
	0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}

2. drivers\media\usb\uvc\uvc_driver.c
add an struct item in array :
static struct uvc_format_desc uvc_fmts[] ... {
...
    {
        .name = "YUV 4:2:0 (NV21)",
        .guid = UVC_GUID_FORMAT_NV21,
        .fcc = V4L2_PIX_FMT_NV21,
    },
...
}
	
rebuild kernel and burn it.


How to change USB driver to HOST mode ?  Run this command on the board:
echo host > /proc/10320000.usb30drd/mode


What to do if sample_uvc is killed when OS out of memory?
The OS default memory is too small, host_uvc sample need more memory, we can expands OS memory and shrink MMZ memory to solve this problem.
Assuming we are using a nand flash board that has 2GB RAM, do as follows:
1. Check u-boot env:
# pri
...
bootargs=mem=132928K console=ttyAMA0,115200 clk_ignore_unused root=ubi0:ubifs rootfstype=ubifs rw ubi.mtd=4 mtdparts=nand:512K(boot),512K(env),512K(bl31),13M(kernel),32M(rootfs)
...

2. Change the mem in u-boot, run u-boot command:
setenv bootargs 'mem=524096K console=ttyAMA0,115200 clk_ignore_unused root=ubi0:ubifs rootfstype=ubifs rw ubi.mtd=4 mtdparts=nand:512K(boot),512K(env),512K(bl31),13M(kernel),32M(rootfs)'
sa   # save env

3. Change mmz_start/mmz_size in load ko script. Enter directory smp/a55_linux/source/out/ko, edit file load3519dv500/load3519dv500_sec, the default is:
    mmz_start=0x48200000;         # mmz start addr, default:0x48200000 (0x40000000+0x30000(FDT&ATF)+0x81D0000(OS))
    mmz_size=1918M;               # mmz size, default:1918M (2048M-130M)

modify it as this:
    mmz_start=0x60000000;
    mmz_size=1536M;

Note that the kernel memory and MMZ memory cannot overlap!