CC = clang
LLD = lld-link
DLLTOOL = llvm-dlltool
GENISOIMAGE = genisoimage
CLANG_FORMAT = clang-format
NT_VERSION = 3.10
VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//')

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
              /subsystem:native,$(NT_VERSION) \
              /entry:DriverEntry \
              /driver \
              /base:0x10000 \
              /align:32 \
              /filealign:32 \
              /nodefaultlib \
              /version:$(NT_VERSION) \
              /dynamicbase:no \
              /nxcompat:no \
              /safeseh:no \
              /tsaware:no \
              /release

LDFLAGS_EXE = /nologo \
              /subsystem:console,$(NT_VERSION) \
              /entry:mainCRTStartup \
              /base:0x400000 \
              /nodefaultlib \
              /version:$(NT_VERSION) \
              /dynamicbase:no \
              /nxcompat:no \
              /safeseh:no \
              /tsaware:no \
              /release

BUILD_DIR = build
INSTDRV_BUILD_DIR = $(BUILD_DIR)/instdrv
VMMOUSE_BUILD_DIR = $(BUILD_DIR)/vmmouse
ISO_ROOT = $(BUILD_DIR)/iso_root

TARGET = $(BUILD_DIR)/vmmouse.sys
INSTALLER = $(BUILD_DIR)/instdrv.exe
ISO = $(BUILD_DIR)/vmmouse.iso
VERSIONED_ISO = $(BUILD_DIR)/vmmouse-$(VERSION).iso

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
            $(ISO_ROOT)/README.TXT \
            $(ISO_ROOT)/LICENSE.TXT \
            $(ISO_SCRIPTS)

all: $(ISO) $(VERSIONED_ISO)
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

$(INSTALLER): $(INSTDRV_OBJS) $(INSTDRV_LIBS) | $(BUILD_DIR)
	$(LLD) $(LDFLAGS_EXE) $^ /out:$@

$(ISO_ROOT)/VMMOUSE.SYS: $(TARGET) | $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/INSTDRV.EXE: $(INSTALLER) | $(ISO_ROOT)
	cp $< $@

$(ISO_ROOT)/README.TXT: scripts/README.TXT | $(ISO_ROOT)
	sed -e 's/@VERSION@/$(VERSION)/g' -e 's/\r*$$/\r/' $< > $@

$(ISO_ROOT)/LICENSE.TXT: LICENSE | $(ISO_ROOT)
	sed 's/\r*$$/\r/' $< > $@

$(ISO_ROOT)/%.BAT: scripts/%.BAT | $(ISO_ROOT)
	sed 's/\r*$$/\r/' $< > $@

$(ISO): $(ISO_FILES)
	$(GENISOIMAGE) -o $@ -V "VMMOUSE" -iso-level 2 -R $(ISO_ROOT)

$(VERSIONED_ISO): $(ISO)
	cp $< $@

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
	docker run --rm -v $(CURDIR):/build -w /build $(DOCKER_IMAGE) make clean all VERSION=$(VERSION)
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
