ROOTFS_BASE	:= rootfs_base/
ROOTFS_OUT	:= rootfs/

# NEED_DIR += <mode>:<path>
NEED_DIR := 0755:/bin
NEED_DIR += 0750:/boot
NEED_DIR += 0755:/dev
NEED_DIR += 0755:/etc
NEED_DIR += 0755:/home
NEED_DIR += 0755:/lib
NEED_DIR += 0755:/mnt
NEED_DIR += 0755:/opt
NEED_DIR += 0555:/proc
NEED_DIR += 0700:/root
NEED_DIR += 0755:/run
NEED_DIR += 0755:/sbin
NEED_DIR += 0755:/share
NEED_DIR += 0555:/sys
NEED_DIR += 0755:/tmp
NEED_DIR += 0755:/usr
NEED_DIR += 0755:/usr/bin
NEED_DIR += 0755:/usr/lib
NEED_DIR += 0755:/usr/sbin
NEED_DIR += 0755:/usr/share
NEED_DIR += 0755:/var
NEED_DIR += 0755:/lost+found

get-mode = $(word 1,$(subst :, ,$1))
get-path = $(word 2,$(subst :, ,$1))

TARGET_MODE_DIR := $(subst :/,:$(ROOTFS_OUT),$(NEED_DIR))
TARGET_DIR := $(foreach item, $(TARGET_MODE_DIR), $(call get-path,$(item)))

all: $(ROOTFS_OUT) $(TARGET_DIR)

$(ROOTFS_OUT): FORCE
	cp -rf $(ROOTFS_BASE) $@

$(TARGET_DIR): $(ROOTFS_OUT) FORCE
	mkdir -p $@
	chmod $(call get-mode,$(filter %:$@,$(TARGET_MODE_DIR))) $@

clean:
	rm -rf $(ROOTFS_OUT)

.PHONY: FORCE
FORCE:
