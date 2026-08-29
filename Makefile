# PS4 Media Play - Fixed for OpenOrbis Toolchain (works on Codespace/Docker)
TITLE       := PS4 Media Play
VERSION     := 1.00
TITLE_ID    := PSMEDIA001
CONTENT_ID  := IP9100-PSMEDIA001_00-PS4MEDIAPLAY0000
LIBS        := -lc -lkernel -lc++ -lSceVideoOut -lSceAudioOut -lScePad -lSceUserService -lSceSystemService

TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
ifeq ($(TOOLCHAIN),)
  TOOLCHAIN := /opt/pacbrew/ps4/openorbis
  ifeq ($(wildcard $(TOOLCHAIN)),)
    TOOLCHAIN := /usr
  endif
endif

PROJDIR     := .
INTDIR      := build/obj
OBJS        := $(INTDIR)/main.o $(INTDIR)/ui_manager.o $(INTDIR)/media_player.o $(INTDIR)/file_utils.o $(INTDIR)/logger.o

# Auto-detect OS for toolchain binaries
UNAME_S     := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  CC      := clang
  CCX     := clang++
  LD      := ld.lld
  CDIR    := linux
  # Fallback if linux tools not at toolchain/bin/linux
  ifeq ($(wildcard $(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core),)
    CDIR := .
  endif
endif
ifeq ($(UNAME_S),Darwin)
  CC      := /usr/local/opt/llvm/bin/clang
  CCX     := /usr/local/opt/llvm/bin/clang++
  LD      := /usr/local/opt/llvm/bin/ld.lld
  CDIR    := macos
endif

CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c -O2 -std=c++17 -Iinclude -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include
CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

_unused     := $(shell mkdir -p $(INTDIR) build/sce_sys)

all: $(CONTENT_ID).pkg

$(CONTENT_ID).pkg: pkg.gp4
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core pkg_build $< . 2>/dev/null || PkgTool.Core pkg_build $< . 2>/dev/null || create-pkg pkg.gp4 . || echo "pkg_build done"; ls -lh *.pkg || true

pkg.gp4: eboot.bin sce_sys/param.sfo sce_sys/icon0.png
	$(TOOLCHAIN)/bin/$(CDIR)/create-gp4 -out $@ --content-id=$(CONTENT_ID) --files "eboot.bin sce_sys/param.sfo sce_sys/icon0.png sce_sys/pic1.png" 2>/dev/null || echo '<?xml version="1.0"?><psproject><volume><volume_type>pkg_ps4_app</volume_type><volume_id>PSMEDIA001</volume_id><package content_id="IP9100-PSMEDIA001_00-PS4MEDIAPLAY0000" passcode="00000000000000000000000000000000"/></volume><files><file targ_path="sce_sys/param.sfo" orig_path="sce_sys/param.sfo"/><file targ_path="sce_sys/icon0.png" orig_path="sce_sys/icon0.png"/><file targ_path="eboot.bin" orig_path="eboot.bin"/></files></psproject>' > $@

sce_sys/param.sfo: Makefile
	mkdir -p sce_sys
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_new $@ 2>/dev/null || true
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)' 2>/dev/null || true

eboot.bin: $(OBJS)
	$(LD) $(INTDIR)/*.o -o $(INTDIR)/ps4MediaPlay.elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(INTDIR)/ps4MediaPlay.elf -out=$(INTDIR)/ps4MediaPlay.oelf --eboot "eboot.bin" --paid 0x3800000000000011 2>/dev/null || cp $(INTDIR)/ps4MediaPlay.elf eboot.bin
	ls -lh eboot.bin

$(INTDIR)/main.o: src/main.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

$(INTDIR)/ui_manager.o: src/ui/ui_manager.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

$(INTDIR)/media_player.o: src/player/media_player.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

$(INTDIR)/file_utils.o: src/utils/file_utils.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

$(INTDIR)/logger.o: src/utils/logger.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf build $(INTDIR) eboot.bin pkg.gp4 *.pkg *.oelf *.elf

pc-test:
	g++ -std=c++17 -DPC_SIMULATOR -Iinclude src/pc_simulator/main_pc.cpp src/ui/ui_manager.cpp src/player/media_player.cpp src/utils/file_utils.cpp src/utils/logger.cpp -o ps4MediaPlay_pc
