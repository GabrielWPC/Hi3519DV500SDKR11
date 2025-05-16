CSRC  = $(wildcard *.c)

CFLAGS += -nostdinc

#.SILENT:

default:
	echo "  CC    $(CSRC)"
	echo "$(CC) $(CFLAGS) $(CSRC)"
	$(CC) $(CFLAGS) $(CSRC)

clean:
	rm *.o *.a -f
