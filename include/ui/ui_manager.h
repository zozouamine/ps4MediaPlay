#pragma once
#include <string>
#include <vector>

class MediaPlayer;

enum class AppState {
    BROWSER,    // متصفح الملفات
    PLAYER,     // شاشة التشغيل
    PLAYLIST,   // قائمة التشغيل
    SETTINGS    // الاعدادات
};

struct MenuItem {
    std::string name;
    std::string path;
    bool isDirectory;
    bool isMediaFile;
    std::string sizeStr;
};

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool init(int width, int height);
    void shutdown();
    void update();
    void render();
    void run(MediaPlayer* player);

    // Input handlers - mapped to DS4
    void onButtonCross();     // X - اختيار/تشغيل
    void onButtonCircle();    // O - رجوع
    void onButtonTriangle();  // Triangle - معلومات
    void onButtonSquare();    // Square - قائمة تشغيل
    void onButtonOptions();   // OPTIONS - اعدادات
    void onNavigateUp();
    void onNavigateDown();
    void onNavigateLeft();
    void onNavigateRight();

    void setPlayer(MediaPlayer* p) { m_player = p; }
    void navigateTo(const std::string& path);
    void refreshCurrentDir();

private:
    void renderBrowser();
    void renderPlayer();
    void renderPlaylist();
    void renderSettings();
    void renderTopBar();
    void renderControlsBar();
    
    int m_width, m_height;
    AppState m_state;
    MediaPlayer* m_player;

    std::string m_currentPath;
    std::vector<MenuItem> m_items;
    int m_selectedIndex;
    int m_scrollOffset;

    // Settings
    bool m_showHiddenFiles;
    std::string m_sortMode;

    // For rendering (PS4 uses liborbisNFS / debug font, or custom)
    void drawText(int x, int y, const std::string& text, int color);
    void drawRect(int x, int y, int w, int h, int color);
    void drawVideoFrame();
};
