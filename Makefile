CC = clang
LLD = lld-link
DLLTOOL = llvm-dlltool
GENISOIMAGE = genisoimage
CLANG_FORMAT = clang-format

CFLAGS = -target i686-pc-windows-msvc \
         -march=i486 -m32 \
         -std=c11 \
         -ffreestanding -fno-builtin \
         -fno-stack-protector \
         -mno-sse -mno-sse2 \
         -Iinstdrv/src -Ivmmouse/src \
         -O2 \
         -Wall -Wextra -Wpedantic -Werror \
         -Wno-unused-parameter

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
INSTDRV_BUILD_DIR = $(BUILD_DIR)/instdrv
VMMOUSE_BUILD_DIR = $(BUILD_DIR)/vmmouse
ISO_ROOT = $(BUILD_DIR)/iso_root

TARGET = $(BUILD_DIR)/vmmouse.sys
INSTALLER = $(BUILD_DIR)/instdrv.exe
ISO = $(BUILD_DIR)/vmmouse.iso

C_SOURCES = $(wildcard instdrv/src/*.c vmmouse/src/*.c)
C_HEADERS = $(wildcard instdrv/src/*.h vmmouse/src/*.h)
ALL_C_FILES = $(C_SOURCES) $(C_HEADERS)

INSTDRV_SRCS = $(wildcard instdrv/src/*.c)
INSTDRV_OBJS = $(patsubst instdrv/src/%.c,$(INSTDRV_BUILD_DIR)/%.obj,$(INSTDRV_SRCS))
INSTDRV_LIBS = $(INSTDRV_BUILD_DIR)/kernel32.lib $(INSTDRV_BUILD_DIR)/advapi32.lib

VMMOUSE_SRCS = $(wildcard vmmouse/src/*.c)
VMMOUSE_OBJS = $(patsubst vmmouse/src/%.c,$(VMMOUSE_BUILD_DIR)/%.obj,$(VMMOUSE_SRCS))
VMMOUSE_LIBS = $(VMMOUSE_BUILD_DIR)/ntoskrnl.lib

ISO_SCRIPTS = $(patsubst scripts/%.BAT,$(ISO_ROOT)/%.BAT,$(wildcard scripts/*.BAT))
ISO_FILES = $(ISO_ROOT)/VMMOUSE.SYS \
            $(ISO_ROOT)/INSTDRV.EXE \
            $(ISO_SCRIPTS)

all: $(ISO)
.PHONY: all

$(BUILD_DIR) $(INSTDRV_BUILD_DIR) $(VMMOUSE_BUILD_DIR) $(ISO_ROOT):
	mkdir -p $@

$(INSTDRV_BUILD_DIR)/%.obj: instdrv/src/%.c | $(INSTDRV_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(INSTDRV_BUILD_DIR)/%.lib: instdrv/lib/%.def | $(INSTDRV_BUILD_DIR)
	$(DLLTOOL) -k -m i386 -d $< -l $@

$(VMMOUSE_BUILD_DIR)/%.obj: vmmouse/src/%.c | $(VMMOUSE_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(VMMOUSE_BUILD_DIR)/%.lib: vmmouse/lib/%.def | $(VMMOUSE_BUILD_DIR)
	$(DLLTOOL) -k -m i386 -d $< -l $@

$(TARGET): $(VMMOUSE_OBJS) $(VMMOUSE_LIBS) | $(BUILD_DIR)
	$(LLD) $(LDFLAGS_SYS) $^ /out:$@
	python3 tools/pechecksum.py $@

$(INSTALLER): $(INSTDRV_OBJS) $(INSTDRV_LIBS) | $(BUILD_DIR)
	$(LLD) $(LDFLAGS_EXE) $^ /out:$@
	python3 tools/pechecksum.py $@

$(ISO_ROOT)/VMMOUSE.SYS: $(TARGET) | $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/INSTDRV.EXE: $(INSTALLER) | $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/%.BAT: scripts/%.BAT | $(ISO_ROOT)
	sed 's/\r*$$/\r/' $< > $@

$(ISO): $(ISO_FILES)
	$(GENISOIMAGE) -o $@ -V "VMMOUSE" -iso-level 2 -R $(ISO_ROOT)

format:
	$(CLANG_FORMAT) -i $(ALL_C_FILES)
.PHONY: format

lint:
	$(CLANG_FORMAT) --dry-run --Werror $(ALL_C_FILES)
.PHONY: lint

DOCKER_IMAGE = vmmouse-builder

docker-image:
	docker build -t $(DOCKER_IMAGE) .
.PHONY: docker-image

docker-build: docker-image
	docker run --rm -v $(CURDIR):/build -w /build $(DOCKER_IMAGE) make clean all
.PHONY: docker-build

docker-format: docker-image
	docker run --rm -v $(CURDIR):/build -w /build $(DOCKER_IMAGE) make format
.PHONY: docker-format

docker-lint: docker-image
	docker run --rm -v $(CURDIR):/build -w /build $(DOCKER_IMAGE) make lint
.PHONY: docker-lint

clean:
	rm -rf $(BUILD_DIR)
.PHONY: clean
