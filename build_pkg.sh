#!/bin/bash
# build_pkg.sh - سكريبت بناء PKG تلقائي
set -e

echo "=== PS4 Media Play - Build Script ==="

if [ -z "$OO_PS4_TOOLCHAIN" ]; then
  echo "[!] OO_PS4_TOOLCHAIN غير مضبوط، أحاول المسار الافتراضي..."
  export OO_PS4_TOOLCHAIN=/opt/pacbrew/ps4/openorbis
fi

if [ ! -d "$OO_PS4_TOOLCHAIN" ]; then
  echo "[!] OpenOrbis SDK غير مثبت!"
  echo "    للتثبيت:"
  echo "    git clone https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain.git"
  echo "    cd OpenOrbis-PS4-Toolchain && make"
  echo ""
  echo "[*] سيتم انشاء مشروع stub بدون SDK للمعاينة..."
  mkdir -p build/sce_sys
  cp sce_sys/param.json build/sce_sys/ 2>/dev/null || true
  echo "[+] تم تجهيز الهيكل - ثبّت SDK لبناء PKG حقيقي"
  exit 0
fi

echo "[*] SDK found at $OO_PS4_TOOLCHAIN"
make clean
make -j$(nproc)

if [ -f "ps4MediaPlay.pkg" ]; then
  echo "[+] PKG built successfully: ps4MediaPlay.pkg ($(du -h ps4MediaPlay.pkg | cut -f1))"
  echo "[*] انسخه الى USB وثبته عبر GoldHEN Package Installer"
else
  echo "[!] Build failed"
  exit 1
fi
