CC = clang
LLD = lld-link
DLLTOOL = llvm-dlltool
GENISOIMAGE = genisoimage

CFLAGS = -target i686-pc-windows-msvc \
         -march=i486 -m32 \
         -ffreestanding -fno-builtin \
         -fno-stack-protector \
         -mno-sse -mno-sse2 \
         -I./include \
         -O2 -Wall

LDFLAGS_SYS = /nologo \
              /subsystem:native,3.51 \
              /entry:DriverEntry \
              /driver \
              /base:0x10000 \
              /align:32 \
              /filealign:32 \
              /tsaware:no \
              /nodefaultlib \
              /version:3.51 \
              /dynamicbase:no \
              /nxcompat:no

LDFLAGS_EXE = /nologo \
              /subsystem:console,3.51 \
              /entry:mainCRTStartup \
              /base:0x400000 \
              /nodefaultlib \
              /version:3.51 \
              /dynamicbase:no \
              /nxcompat:no

BUILD_DIR = build
ISO_ROOT = $(BUILD_DIR)/iso_root
TARGET = $(BUILD_DIR)/vmmouse.sys
INSTALLER = $(BUILD_DIR)/instdrv.exe
ISO = $(BUILD_DIR)/vmmouse.iso

all: $(ISO)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/ntoskrnl.lib: tools/ntoskrnl.def | $(BUILD_DIR)
	$(DLLTOOL) -k -m i386 -d tools/ntoskrnl.def -l $(BUILD_DIR)/ntoskrnl.lib

$(BUILD_DIR)/kernel32.lib: tools/kernel32.def | $(BUILD_DIR)
	$(DLLTOOL) -k -m i386 -d tools/kernel32.def -l $(BUILD_DIR)/kernel32.lib

$(BUILD_DIR)/advapi32.lib: tools/advapi32.def | $(BUILD_DIR)
	$(DLLTOOL) -k -m i386 -d tools/advapi32.def -l $(BUILD_DIR)/advapi32.lib

$(BUILD_DIR)/vmmouse.obj: src/vmmouse.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/vmmouse.c -o $(BUILD_DIR)/vmmouse.obj

$(BUILD_DIR)/instdrv.obj: src/instdrv.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/instdrv.c -o $(BUILD_DIR)/instdrv.obj

$(TARGET): $(BUILD_DIR)/vmmouse.obj $(BUILD_DIR)/ntoskrnl.lib
	$(LLD) $(LDFLAGS_SYS) $(BUILD_DIR)/vmmouse.obj $(BUILD_DIR)/ntoskrnl.lib /out:$(TARGET)
	python3 tools/pechecksum.py $(TARGET)

$(INSTALLER): $(BUILD_DIR)/instdrv.obj $(BUILD_DIR)/kernel32.lib $(BUILD_DIR)/advapi32.lib
	$(LLD) $(LDFLAGS_EXE) $(BUILD_DIR)/instdrv.obj $(BUILD_DIR)/kernel32.lib \
	    $(BUILD_DIR)/advapi32.lib /out:$(INSTALLER)
	python3 tools/pechecksum.py $(INSTALLER)

$(ISO): $(TARGET) $(INSTALLER)
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)
	cp $(TARGET) $(ISO_ROOT)/VMMOUSE.SYS
	cp $(INSTALLER) $(ISO_ROOT)/INSTDRV.EXE
	printf '@echo off\r\nD:\r\nINSTDRV.EXE\r\npause\r\n' > $(ISO_ROOT)/INSTALL.BAT
	printf '@echo off\r\nD:\r\nINSTDRV.EXE /u\r\npause\r\n' > $(ISO_ROOT)/UNINSTALL.BAT
	$(GENISOIMAGE) -o $(ISO) -V "VMMOUSE" -iso-level 2 -R $(ISO_ROOT)

DOCKER_IMAGE = vmmouse-builder

docker-image:
	docker build -t $(DOCKER_IMAGE) .

docker-build: docker-image
	docker run --rm -v $(CURDIR):/build -w /build $(DOCKER_IMAGE) make clean all

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean docker-image docker-build
