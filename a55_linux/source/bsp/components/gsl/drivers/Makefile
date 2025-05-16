CSRC  = $(wildcard *.c wildcard common/*.c wildcard otp/*.c wildcard svb/*.c  wildcard ddr_init/*.c ddr_init/boot_adaptation/common/*.c ddr_init/hal/*.c ddr_init/hal/chip/hi3519dv500/*.c ddr_init/hal/ddrc/common/*.c ddr_init/hal/ddrc/*.c ddr_init/hal/ddrt/common/*.c ddr_init/hal/ddrt/*.c ddr_init/hal/phy/common/*.c ddr_init/hal/phy/*.c ddr_init/hal/sysctrl/*.c ddr_init/hal/uart/*.c ddr_init/training/*.c ddr_init/api/*.c wildcard securelib/*.c wildcard uart/*.c  wildcard tzasc/*.c wildcard share_drivers/*.c wildcard ipc/*.c wildcard gpll/*.c)
OBJS := $(patsubst %.c,%.o,$(CSRC) )

#.SILENT:

CFLAGS += -I./
CFLAGS += -I./otp/
CFLAGS += -I./svb/
CFLAGS += -I./ddr_init/
CFLAGS += -I./ddr_init/boot_adaptation/common/
CFLAGS += -I./ddr_init/include/
CFLAGS += -I./ddr_init/include/hal/
CFLAGS += -I./ddr_init/include/hal/chip/hi3519dv500/
CFLAGS += -I./ddr_init/include/hal/ddrc/
CFLAGS += -I./ddr_init/include/hal/ddrt/
CFLAGS += -I./ddr_init/include/hal/phy/
CFLAGS += -I./ddr_init/include/hal/uart/
CFLAGS += -I./ddr_init/include/training/
CFLAGS += -I./uart/
CFLAGS += -I./tzasc/
CFLAGS += -I./ipc/
CFLAGS += -I./share_drivers/include/

default: libdrv.a

libdrv.a: $(OBJS)
	echo "  AR    libdrv.a"
	$(AR) r libdrv.a $(OBJS)
	echo "  RL    $@"
	$(RANLIB) $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@
#.c.o:
#	echo "  CC    $<"
#	$(CC) $(CFLAGS) $<

clean:
	find . -name '*.o' -type f | xargs rm -f
	find . -name '*.a' -type f | xargs rm -f