ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(MPP_BUILD), y)
CFLAGS += -I$(SOURCE_PATH)/common/include -I$(CBB_ROOT)/include
else
CFLAGS += -I$(REL_INC)
endif

CFLAGS += -I$(OSAL_ROOT)/include -I../../
CFLAGS += -fstack-protector-all    # SP
CFLAGS += -pie -fPIE               # PIE
CFLAGS += -s                       # STRIP
LIB_CFLAGS += -Wl,-z,relro,-z,now  # BIND NOW

default:
	$(CC) -Wall -O2 $(CFLAGS) $(LIB_CFLAGS) sample_adc.c -o sample_adc

clean:
	@rm -rf sample_adc *.o
