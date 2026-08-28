#include "ui/ui_manager.h"
#include "player/media_player.h"
#include "utils/file_utils.h"
#include "utils/logger.h"
#include <algorithm>

#ifdef PC_SIMULATOR
// SDL optional - not required for console test
#ifdef HAS_SDL
#include <SDL2/SDL.h>
#endif
#endif

UIManager::UIManager() : m_width(1920), m_height(1080), m_state(AppState::BROWSER), m_player(nullptr), m_selectedIndex(0), m_scrollOffset(0), m_showHiddenFiles(false) {
    // PS4 mounts
    m_currentPath = "/mnt/usb0";
}

UIManager::~UIManager() {}

bool UIManager::init(int width, int height) {
    m_width = width;
    m_height = height;
    Logger::info("UI Manager init %dx%d", width, height);
    
    // Try USB mounts in order
    std::vector<std::string> mounts = {"/mnt/usb0", "/mnt/usb1", "/data", "/mnt/sandbox/NPXS20113_000", "/user/home"};
    for (auto &p : mounts) {
        if (FileUtils::exists(p)) {
            m_currentPath = p;
            break;
        }
    }
    refreshCurrentDir();
    return true;
}

void UIManager::shutdown() {
    Logger::info("UI Shutdown");
}

void UIManager::refreshCurrentDir() {
    m_items.clear();
    m_selectedIndex = 0;
    m_scrollOffset = 0;

    // Add ".." parent
    if (m_currentPath != "/" && m_currentPath != "/mnt/usb0") {
        m_items.push_back({"..", FileUtils::getParent(m_currentPath), true, false, ""});
    }

    auto files = FileUtils::listDirectory(m_currentPath);
    for (auto &f : files) {
        MenuItem item;
        item.name = f.name;
        item.path = f.path;
        item.isDirectory = f.isDirectory;
        item.isMediaFile = FileUtils::isMediaFile(f.name);
        item.sizeStr = FileUtils::formatSize(f.size);
        // Show only dirs and media files in browser
        if (item.isDirectory || item.isMediaFile) {
            m_items.push_back(item);
        }
    }

    // Sort: directories first
    std::sort(m_items.begin() + (m_items.size()>0 && m_items[0].name==".." ? 1:0), m_items.end(), [](const MenuItem& a, const MenuItem& b){
        if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
        return a.name < b.name;
    });

    Logger::info("Browsing: %s (%zu items)", m_currentPath.c_str(), m_items.size());
}

void UIManager::navigateTo(const std::string& path) {
    if (!FileUtils::exists(path)) {
        Logger::warn("Path not exists: %s", path.c_str());
        return;
    }
    if (FileUtils::isDirectory(path)) {
        m_currentPath = path;
        refreshCurrentDir();
    } else if (FileUtils::isMediaFile(path)) {
        // Play file
        if (m_player) {
            m_player->play(path);
            m_state = AppState::PLAYER;
        }
    }
}

void UIManager::onButtonCross() {
    if (m_state == AppState::BROWSER) {
        if (m_items.empty()) return;
        auto &item = m_items[m_selectedIndex];
        navigateTo(item.path);
    } else if (m_state == AppState::PLAYER) {
        if (m_player) m_player->togglePause();
    }
}

void UIManager::onButtonCircle() {
    if (m_state == AppState::BROWSER) {
        // Go parent
        std::string parent = FileUtils::getParent(m_currentPath);
        if (parent != m_currentPath) {
            m_currentPath = parent;
            refreshCurrentDir();
        }
    } else if (m_state == AppState::PLAYER) {
        if (m_player) m_player->stop();
        m_state = AppState::BROWSER;
    } else {
        m_state = AppState::BROWSER;
    }
}

void UIManager::onButtonTriangle() {
    // Show file info - toggle
    Logger::info("Triangle pressed - info");
}

void UIManager::onButtonSquare() {
    if (m_state == AppState::BROWSER && !m_items.empty()) {
        auto &item = m_items[m_selectedIndex];
        if (item.isMediaFile && m_player) {
            m_player->addToPlaylist(item.path);
            Logger::info("Added to playlist: %s", item.path.c_str());
        }
    }
    if (m_state == AppState::PLAYER) m_state = AppState::PLAYLIST;
    else if (m_state == AppState::PLAYLIST) m_state = AppState::PLAYER;
}

void UIManager::onButtonOptions() {
    m_state = (m_state == AppState::SETTINGS) ? AppState::BROWSER : AppState::SETTINGS;
}

void UIManager::onNavigateUp() {
    if (m_selectedIndex > 0) {
        m_selectedIndex--;
        if (m_selectedIndex < m_scrollOffset) m_scrollOffset = m_selectedIndex;
    }
}

void UIManager::onNavigateDown() {
    if (m_selectedIndex < (int)m_items.size() - 1) {
        m_selectedIndex++;
        if (m_selectedIndex >= m_scrollOffset + 15) m_scrollOffset++;
    }
}

void UIManager::onNavigateLeft() {
    if (m_state == AppState::PLAYER && m_player) m_player->seekRelative(-10);
}

void UIManager::onNavigateRight() {
    if (m_state == AppState::PLAYER && m_player) m_player->seekRelative(10);
}

void UIManager::update() {}

void UIManager::render() {
#ifdef PC_SIMULATOR
    // SDL rendering would go here
#else
    // PS4 rendering via SceVideoOut + liborbis (sample)
    // sceVideoOut* calls
#endif
    switch (m_state) {
        case AppState::BROWSER: renderBrowser(); break;
        case AppState::PLAYER: renderPlayer(); break;
        case AppState::PLAYLIST: renderPlaylist(); break;
        case AppState::SETTINGS: renderSettings(); break;
    }
}

void UIManager::renderBrowser() {
    // Layout: 1920x1080
    // Top bar 80px, file list, controls bar 100px
    renderTopBar();
    
    // File list rendering logic
    // Draw 15 items visible
    for (int i = 0; i < 15 && (m_scrollOffset + i) < (int)m_items.size(); i++) {
        int idx = m_scrollOffset + i;
        auto &item = m_items[idx];
        bool selected = (idx == m_selectedIndex);
        // Colors: selected = blue highlight #0070CC (PlayStation blue)
        int y = 120 + i * 52;
        // drawRect(80, y, 1760, 48, selected ? 0xFF0070CC : 0xFF1A1A1A);
        // drawText(120, y+16, (item.isDirectory ? "[DIR] " : "[FILE] ") + item.name, selected ? 0xFFFFFFFF : 0xFFCCCCCC);
    }
    
    renderControlsBar();
}

void UIManager::renderPlayer() {
    drawVideoFrame();
    // Overlay controls
    if (m_player) {
        // Progress bar
        // Time, title, volume
    }
}

void UIManager::renderPlaylist() {}
void UIManager::renderSettings() {}
void UIManager::renderTopBar() {}
void UIManager::renderControlsBar() {}

void UIManager::drawText(int x, int y, const std::string& text, int color) {
#ifdef PC_SIMULATOR
    // SDL_TTF
#endif
}

void UIManager::drawRect(int x, int y, int w, int h, int color) {}
void UIManager::drawVideoFrame() {}

void UIManager::run(MediaPlayer* player) {
    m_player = player;
#ifdef PC_SIMULATOR
    // SDL main loop for testing on PC before deploying to PS4
    Logger::info("Running PC simulator - use Arrow keys, Enter, ESC to navigate");
#endif
}
