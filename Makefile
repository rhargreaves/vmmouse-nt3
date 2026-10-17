CC ?= clang
LLD ?= lld-link
DLLTOOL ?= llvm-dlltool

CFLAGS = -target i686-pc-windows-msvc \
         -march=i486 -m32 \
         -ffreestanding -fno-builtin \
         -fno-stack-protector \
         -mno-sse -mno-sse2 \
         -I./include \
         -O2 -Wall

LDFLAGS = /nologo \
          /subsystem:native \
          /entry:DriverEntry \
          /driver \
          /base:0x10000 \
          /align:0x20 \
          /nodefaultlib \
          /majorosversion:3 /minorosversion:51 \
          /majorimageversion:3 /minorimageversion:51 \
          /majorsubsystemversion:3 /minorsubsystemversion:51

BUILD_DIR = build
TARGET = $(BUILD_DIR)/vmmouse.sys

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/ntoskrnl.lib: tools/ntoskrnl.def | $(BUILD_DIR)
	$(DLLTOOL) -m i386 -d tools/ntoskrnl.def -l $(BUILD_DIR)/ntoskrnl.lib

$(BUILD_DIR)/vmmouse.obj: src/vmmouse.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/vmmouse.c -o $(BUILD_DIR)/vmmouse.obj

$(TARGET): $(BUILD_DIR)/vmmouse.obj $(BUILD_DIR)/ntoskrnl.lib
	$(LLD) $(LDFLAGS) $(BUILD_DIR)/vmmouse.obj $(BUILD_DIR)/ntoskrnl.lib /out:$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
