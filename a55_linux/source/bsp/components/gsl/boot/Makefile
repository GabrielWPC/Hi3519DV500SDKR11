CSRC  = $(wildcard *.c)
SSRC  = $(wildcard *.S)

CFLAGS += -nostdinc
SFLAGS += $(CFLAGS)

#.SILENT:

default:
	echo "  CC    $(CSRC)"
	echo "$(CC) $(CFLAGS) $(CSRC)"
	$(CC) $(CFLAGS) $(CSRC)
	echo "  CC    $(SSRC)"
	$(CC) $(SFLAGS) $(SSRC)

clean:
	rm *.o *.a -f
