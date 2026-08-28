# دليل التثبيت - PS4 Media Play

## الطريقة 1: التثبيت الجاهز (بدون بناء) - أسهل طريقة

اذا لم ترد بناء المشروع بنفسك، يمكنك بناء PKG مباشرة:

### عبر GoldHEN Package Installer
1. انسخ ملف `ps4MediaPlay.pkg` الى USB (FAT32 أو exFAT)
2. ضع USB في PS4
3. فعل الجيلبريك GoldHEN
4. اذهب الى `Settings > GoldHEN > Package Installer`
5. اختر `ps4MediaPlay.pkg` وثبته
6. ستجد التطبيق في الشاشة الرئيسية باسم "PS4 Media Play"

### عبر FTP (بدون USB)
1. فعل GoldHEN + FTP Server
2. من الكمبيوتر، اتصل بـ FTP على IP جهاز PS4 (منفذ 2121)
3. ارفع `ps4MediaPlay.pkg` الى `/data/`
4. في PS4: GoldHEN > Package Installer > Install from /data

---

## الطريقة 2: بناء المشروع من المصدر (للمطورين)

### المتطلبات على الكمبيوتر (Linux / WSL / macOS)
```bash
# تثبيت OpenOrbis SDK
git clone https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain.git
cd OpenOrbis-PS4-Toolchain
make
export OO_PS4_TOOLCHAIN=/opt/pacbrew/ps4/openorbis
export PATH=$OO_PS4_TOOLCHAIN/bin:$PATH

# تثبيت FFmpeg for PS4 (ports)
git clone https://github.com/OpenOrbis/OpenOrbis-Ports.git
cd OpenOrbis-Ports/ports/ffmpeg && make
```

### بناء PKG
```bash
cd "ps4Media Play"
make clean && make
# سينتج ملف ps4MediaPlay.pkg
```

### التجربة على الكمبيوتر قبل النقل لـ PS4
```bash
# محاكي PC (SDL2)
sudo apt install libsdl2-dev libavformat-dev libavcodec-dev
make pc-test
./ps4MediaPlay_pc
```

---

## وضع الملفات للتشغيل

بعد تثبيت التطبيق:

| المصدر | المسار في PS4 | ملاحظات |
|--------|---------------|---------|
| USB | `/mnt/usb0/` أو `/mnt/usb1/` | ضع ملفات MP4/MP3 في فلاشة FAT32/exFAT |
| HDD داخلي | `/data/media/` | انقل عبر FTP |
| HDD داخلي | `/user/home/media/` | بديل |

**يفضل exFAT للملفات >4GB (أفلام كبيرة)**

---

## حل المشاكل

- **لا يظهر USB؟** تأكد أن الفلاشة exFAT وجرب المنفذين. بعض PS4 يحتاج اعادة تشغيل بعد ادخال USB.
- **فيديو بدون صوت؟** تأكد أن كودك الصوت AAC/MP3. استخدم Handbrake لتحويل MKV الى MP4/AAC اذا لزم.
- **التطبيق لا يفتح؟** تأكد من تفعيل GoldHEN قبل فتح التطبيق.

## الفيرمويرات المدعومة
6.72, 7.02, 7.55, 9.00, 9.60, 10.01, 11.00 (أي فيرموير به GoldHEN)

