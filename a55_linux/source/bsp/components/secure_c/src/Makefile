######
###     This is a sample file for linux platform, please write the Makefile according to the actual situation of the product
###     If you want to create a shared library, use the command:"make"
###     If you want to create a static library, use the command:"make lib"
###     If you want to create a .ko file, use the command:"make kernel"
###     If you want to filter out unsupported compiler options, add the command:"CHECK_OPTION=check"
######

PROJECT=libsecurec.so
#if you need a debug version library, use "-g" instead of "-s -DNDEBUG -O2".
# If you compiler report a warning on "break strict-aliasing rules", there is no problem. If you need to clear all warnings, you can add "-fno-strict-aliasing" option to your compiler, but this will impact the performance a little.
CC?=gcc

#for linux secure compile options from stdandard 2018.8
SECURE_CFLAG_FOR_SHARED_LIBRARY  = -fPIC
SECURE_LDFLAG_FOR_SHARED_LIBRARY += -s
SECURE_LDFLAG_FOR_SHARED_LIBRARY += -Wl,-z,relro,-z,now,-z,noexecstack
#This option ensure forced links to functions in the library. If these functions are already contained in other libraries, cross-calls will occur
#SECURE_LDFLAG_FOR_SHARED_LIBRARY += -Wl,-Bsymbolic


GCC_HAVE_STRONG=$(shell $(CC) -fstack-protector-strong -E -dM - < /dev/null 2>&1 |grep -- "fstack-protector-strong" >/dev/null ;if [ $$? -ne 0 ] ;then echo yes;fi)
ifeq ($(GCC_HAVE_STRONG),yes)
SECURE_CFLAG_FOR_SHARED_LIBRARY  += -fstack-protector-strong
else
SECURE_CFLAG_FOR_SHARED_LIBRARY  += -fstack-protector-all
endif


#-fvisibility=hidden need modify source code
#SECURE_CFLAG_FOR_SHARED_LIBRARY_OPTIONAL += -fvisibility=hidden

#-ftrapv -D_FORTIFY_SOURCE=2 -fstack-check May result in performance degradation after opening
SECURE_CFLAG_FOR_SHARED_LIBRARY_OPTIONAL += -D_FORTIFY_SOURCE=2 -O2
#SECURE_CFLAG_FOR_SHARED_LIBRARY_OPTIONAL += -ftrapv
#SECURE_CFLAG_FOR_SHARED_LIBRARY_OPTIONAL += -fstack-check


SECURE_CFLAG_FOR_SHARED_LIBRARY_OPTIONAL +=  -Wformat=2 -Wfloat-equal -Wshadow
# about pie option , We compiled a dynamic library, so we did not use it. ,If you want to compile executable files, please open this option
#SECURE_CFLAG_FOR_EXE = -fPIE -pie



## code standard options
SECUREC_CODE_STANDARD_OPTION = -Wconversion
SECUREC_CODE_STANDARD_OPTION += -Wformat-security
SECUREC_CODE_STANDARD_OPTION += -Wextra
SECUREC_CODE_STANDARD_OPTION += --param ssp-buffer-size=4

#repeat options
#SECUREC_CODE_STANDARD_OPTION += -D_FORTIFY_SOURCE=2
#SECUREC_CODE_STANDARD_OPTION += -Wl,-z,relro,-z,now
#SECUREC_CODE_STANDARD_OPTION += -fstack-protector

#from product options
PRODUCT_OPTION_FOR_SELECTION  = -Warray-bounds
PRODUCT_OPTION_FOR_SELECTION += -Wpointer-arith
PRODUCT_OPTION_FOR_SELECTION += -Wcast-qual
PRODUCT_OPTION_FOR_SELECTION += -Wstrict-prototypes
PRODUCT_OPTION_FOR_SELECTION += -Wmissing-prototypes
PRODUCT_OPTION_FOR_SELECTION += -Wstrict-overflow=1
PRODUCT_OPTION_FOR_SELECTION += -Wstrict-aliasing=2
PRODUCT_OPTION_FOR_SELECTION += -Wswitch -Wswitch-default
PRODUCT_OPTION_FOR_SELECTION += -Wdate-time
PRODUCT_OPTION_FOR_SELECTION += -Wdisabled-optimization
PRODUCT_OPTION_FOR_SELECTION += -Wduplicated-branches
PRODUCT_OPTION_FOR_SELECTION += -Wduplicated-cond
PRODUCT_OPTION_FOR_SELECTION += -Wignored-qualifiers
PRODUCT_OPTION_FOR_SELECTION += -Wimplicit-fallthrough=3
PRODUCT_OPTION_FOR_SELECTION += -Wmissing-include-dirs
PRODUCT_OPTION_FOR_SELECTION += -Wshift-negative-value
PRODUCT_OPTION_FOR_SELECTION += -Wsign-compare
PRODUCT_OPTION_FOR_SELECTION += -Wtrampolines
PRODUCT_OPTION_FOR_SELECTION += -Wtype-limits
PRODUCT_OPTION_FOR_SELECTION += -Wwrite-strings
PRODUCT_OPTION_FOR_SELECTION += -Werror
PRODUCT_OPTION_FOR_SELECTION += -Wunused-macros
PRODUCT_OPTION_FOR_SELECTION += -Wshift-overflow=2
PRODUCT_OPTION_FOR_SELECTION += -Wnested-externs
PRODUCT_OPTION_FOR_SELECTION += -Wlogical-op
PRODUCT_OPTION_FOR_SELECTION += -Wjump-misses-init
PRODUCT_OPTION_FOR_SELECTION += -Wvla
PRODUCT_OPTION_FOR_SELECTION += -Wframe-larger-than=4096
PRODUCT_OPTION_FOR_SELECTION += -Wsuggest-attribute=format
PRODUCT_OPTION_FOR_SELECTION += -Wmissing-format-attribute
PRODUCT_OPTION_FOR_SELECTION += # -fno-inline-small-functions -fno-indirect-inlining -fno-inline-functions-called-once -fno-early-inlining -fno-inline
PRODUCT_OPTION_FOR_SELECTION += # -ftrapv
ifneq ($(LLVM), )
PRODUCT_OPTION_FOR_SELECTION += -Wno-error=format-nonliteral
endif
ifeq ($(CHECK_OPTION),check)
PRODUCT_OPTION := $(shell touch tmp.c; echo "int main(void){return 0;}" > tmp.c; \
        for loop in $(PRODUCT_OPTION_FOR_SELECTION); \
        do err=$$({ `$(CC) $$loop -I../include -c tmp.c >/dev/null`; } 2>&1);\
	echo $${err} | grep ^clang:.w.*ted\]$$ > /dev/null 2>&1;\
	if [ $$? = 0 ] || [ "X$${err}" = "X" ];then echo "$$loop";fi;\
        done;\
        if [ -f tmp.o ];then rm tmp.o;fi;\
        if [ -f tmp.c ];then rm tmp.c;fi;)
else
PRODUCT_OPTION := $(PRODUCT_OPTION_FOR_SELECTION)
endif

CFLAG   = -Wall -DNDEBUG -O2 $(SECURE_CFLAG_FOR_SHARED_LIBRARY) $(SECURE_CFLAG_FOR_EXE) $(SECURE_CFLAG_FOR_SHARED_LIBRARY_OPTIONAL) $(SECUREC_CODE_STANDARD_OPTION) $(PRODUCT_OPTION)
#CFLAG += -DSECUREC_VXWORKS_PLATFORM
#CFLAG += -DSECUREC_SUPPORT_STRTOLD
#CFLAG += -DSECUREC_VXWORKS_VERSION_5_4
#CFLAG += -D__STDC_WANT_LIB_EXT1__=0
CFLAG  += $(CFLAG_EXT)

ARCH:=$(shell getconf LONG_BIT)

ifeq ($(MAKECMDGOALS),lib)
#Set static library related options
CFLAG :=$(filter-out "xxxxx",$(CFLAG))
endif

#SOURCES=$(wildcard *.c)
SOURCES = fscanf_s.c gets_s.c memcpy_s.c memmove_s.c memset_s.c scanf_s.c securecutil.c secureinput_a.c secureprintoutput_a.c snprintf_s.c sprintf_s.c sscanf_s.c strcat_s.c strcpy_s.c strncat_s.c strncpy_s.c strtok_s.c vfscanf_s.c vscanf_s.c vsnprintf_s.c vsprintf_s.c vsscanf_s.c

SOURCES += fwscanf_s.c secureinput_w.c secureprintoutput_w.c swprintf_s.c swscanf_s.c vfwscanf_s.c vswprintf_s.c vswscanf_s.c vwscanf_s.c wcscat_s.c wcscpy_s.c wcsncat_s.c wcsncpy_s.c wcstok_s.c wmemcpy_s.c wmemmove_s.c wscanf_s.c

OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

.PHONY:clean lib kernel

ENABLE_SCANF_FILE=$(findstring SECUREC_ENABLE_SCANF_FILE=0,$(CFLAG))
ifeq ($(ENABLE_SCANF_FILE),SECUREC_ENABLE_SCANF_FILE=0)
OBJECTS:=$(filter-out fscanf_s.o vfscanf_s.o vscanf_s.o scanf_s.o vwscanf_s.o wscanf_s.o fwscanf_s.o vfwscanf_s.o,$(OBJECTS))
endif


ifneq ($(CFLAGS),)
CFLAG :=$(CFLAGS)
endif
CFLAG += -I../include
LD_FLAG ?= $(SECURE_LDFLAG_FOR_SHARED_LIBRARY) $(SECURE_CFLAG_FOR_SHARED_LIBRARY)
AR ?=ar
RANLIB ?=ranlib

$(PROJECT): note_msg $(OBJECTS)
	@mkdir -p ../obj
	mkdir -p ../lib
	$(CC)  -shared -o ../lib/$@ $(patsubst %.o,../obj/%.o,$(OBJECTS)) $(LD_FLAG)
	@echo "finish $(PROJECT)"
	#you may add you custom commands here

lib: note_msg $(OBJECTS)
	$(AR) crv libsecurec.a $(patsubst %.o,../obj/%.o,$(OBJECTS))
	$(RANLIB) libsecurec.a
	-mkdir -p ../lib
	-cp libsecurec.a ../lib
	@echo "finish libsecurec.a"
	#you may add you custom commands here
.c.o:
	@mkdir -p ../obj
	$(CC) -c $< $(CFLAG) -o ../obj/$(patsubst %.c,%.o,$<)

EXTRA_CFLAGS += -I$(INCDIR) -fstack-protector $(CFLAG_EXT)

# provide the default value to module name and ccflags-y
ifeq ($(MODULE),)
	MODULE := ksecurec
endif
ifeq ($(DEBUG),y)
	ccflags-y += -DDEBUG
endif

ifneq ($(KERNELRELEASE),)
    obj-m := ksecurec.o
ifeq ($(SECUREC_KERNEL_ALL),)
    #ksecurec-objs := memcpy_s.o memmove_s.o memset_s.o  securecutil.o  strcat_s.o strcpy_s.o strncat_s.o strncpy_s.o
    ksecurec-objs := memcpy_s.o memmove_s.o memset_s.o  securecutil.o  strcat_s.o strcpy_s.o strncat_s.o strncpy_s.o sprintf_s.o vsprintf_s.o snprintf_s.o vsnprintf_s.o secureprintoutput_a.o sscanf_s.o vsscanf_s.o secureinput_a.o strtok_s.o
else
    ksecurec-objs := memcpy_s.o memmove_s.o memset_s.o  securecutil.o  strcat_s.o strcpy_s.o strncat_s.o strncpy_s.o sprintf_s.o vsprintf_s.o snprintf_s.o vsnprintf_s.o secureprintoutput_a.o sscanf_s.o vsscanf_s.o secureinput_a.o strtok_s.o
endif
else
    KERNELDIR := /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)
kernel: note_msg
	$(MAKE) -C $(KERNELDIR) M=$(PWD) INCDIR=$(PWD)/../include modules
endif

NOTE_MSG:='\n'
NOTE_MSG+='---------------------------------------------------------\n'
NOTE_MSG+='+ This Makefile is a sample file, do not use in product +\n'
NOTE_MSG+='---------------------------------------------------------\n'

note_msg:
	-@echo  -e $(NOTE_MSG)

clean:
	@echo "cleaning ...."
	-rm -rf modules.order Module.symvers  $(MODULE).ko $(MODULE).mod.c  $(MODULE).mod.o $(MODULE).o *.o
	-rm -rf ../obj ../lib
	-rm -rf libsecurec.a
	@echo "clean up"

