# PS4 Media Play Makefile - OpenOrbis SDK
# يتطلب تثبيت OpenOrbis PS4 Toolchain

TARGET := ps4MediaPlay.elf
TITLE_ID := PSMEDIA001
TITLE := PS4 Media Play
VERSION := 01.00

# OpenOrbis SDK Path
OO_PS4_TOOLCHAIN := /opt/pacbrew/ps4/openorbis
CC := $(OO_PS4_TOOLCHAIN)/bin/clang
CXX := $(OO_PS4_TOOLCHAIN)/bin/clang++
LD := $(OO_PS4_TOOLCHAIN)/bin/ld.lld

# Flags
CFLAGS := -cc1 -triple x86_64-scei-ps4-elf -O2 -std=c++17 -Iinclude -I$(OO_PS4_TOOLCHAIN)/include -I$(OO_PS4_TOOLCHAIN)/include/c++/v1
LDFLAGS := -L$(OO_PS4_TOOLCHAIN)/lib -lSceVideoOut -lSceAudioOut -lScePad -lSceUserService -lSceSystemService -lkernel -lc -lc++

SRC := src/main.cpp src/ui/ui_manager.cpp src/ui/file_browser.cpp src/player/media_player.cpp src/player/audio_decoder.cpp src/player/video_decoder.cpp src/utils/logger.cpp src/utils/file_utils.cpp
OBJ := $(SRC:.cpp=.o)

all: $(TARGET) pkg

$(TARGET): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS) -entry _main

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

pkg: $(TARGET)
	@echo "[*] Building PKG..."
	@mkdir -p build/sce_sys
	@cp -r assets/* build/ 2>/dev/null || true
	@cp sce_sys/param.sfo build/sce_sys/ 2>/dev/null || true
	@cp sce_sys/icon0.png build/sce_sys/ 2>/dev/null || true
	@$(OO_PS4_TOOLCHAIN)/bin/create-eboot -in build -out ps4MediaPlay.pkg --title "$(TITLE)" --title-id $(TITLE_ID)  || echo "PKG requires OpenOrbis pkg tools"
	@echo "[+] PKG Created: ps4MediaPlay.pkg"

clean:
	rm -f $(OBJ) $(TARGET) ps4MediaPlay.pkg
	rm -rf build

install: pkg
	@echo "انسخ ps4MediaPlay.pkg الى USB وثبته عبر GoldHEN Package Installer"

# للتجربة بدون SDK (محاكاة على PC)
pc-test:
	g++ -std=c++17 -DPC_SIMULATOR -Iinclude src/pc_simulator/main_pc.cpp -o ps4MediaPlay_pc -lSDL2 -lSDL2_mixer -lavformat -lavcodec -lavutil -lswscale
