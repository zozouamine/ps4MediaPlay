#include "player/media_player.h"
#include "utils/logger.h"
#include "utils/file_utils.h"
#include <algorithm>

extern "C" {
#ifdef __has_include
#if __has_include(<libavformat/avformat.h>)
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#define HAS_FFMPEG 1
#endif
#endif
}

MediaPlayer::MediaPlayer() : m_state(PlayerState::STOPPED), m_currentTime(0), m_volume(80), m_autoPlay(true), m_loop(false), m_playlistIndex(-1), m_formatCtx(nullptr), m_codecCtx(nullptr), m_audioHandle(nullptr) {
    m_currentInfo = {"", "", MediaType::UNKNOWN, 0, 0, 0, "", false};
}

MediaPlayer::~MediaPlayer() {
    stop();
}

bool MediaPlayer::detectMediaType(const std::string& path, MediaType& out) {
    std::string ext = FileUtils::getExtension(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    std::vector<std::string> videoExts = {"mp4","mkv","avi","mov","flv","webm","m4v","mpg","mpeg","3gp"};
    std::vector<std::string> audioExts = {"mp3","flac","wav","aac","ogg","wma","m4a","opus"};

    if (std::find(videoExts.begin(), videoExts.end(), ext) != videoExts.end()) { out = MediaType::VIDEO; return true; }
    if (std::find(audioExts.begin(), audioExts.end(), ext) != audioExts.end()) { out = MediaType::AUDIO; return true; }
    out = MediaType::UNKNOWN;
    return false;
}

bool MediaPlayer::initDecoder(const std::string& path) {
    Logger::info("Initializing decoder for: %s", path.c_str());
    
#ifdef HAS_FFMPEG
    AVFormatContext* fmt = nullptr;
    if (avformat_open_input(&fmt, path.c_str(), nullptr, nullptr) != 0) {
        Logger::error("avformat_open_input failed");
        return false;
    }
    avformat_find_stream_info(fmt, nullptr);
    // Find best streams
    int videoStream = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    int audioStream = av_find_best_stream(fmt, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    
    if (videoStream >= 0) {
        m_currentInfo.type = MediaType::VIDEO;
        m_currentInfo.width = fmt->streams[videoStream]->codecpar->width;
        m_currentInfo.height = fmt->streams[videoStream]->codecpar->height;
        Logger::info("Video: %dx%d", m_currentInfo.width, m_currentInfo.height);
    } else if (audioStream >= 0) {
        m_currentInfo.type = MediaType::AUDIO;
    }
    
    if (fmt->duration != AV_NOPTS_VALUE) {
        m_currentInfo.durationSeconds = fmt->duration / AV_TIME_BASE;
    }
    
    m_formatCtx = fmt;
#else
    // Fallback without FFmpeg - use PS4 native (limited)
    // On real PS4, FFmpeg is statically linked from OpenOrbis ports
    Logger::info("FFmpeg not linked, using stub decoder");
    m_currentInfo.durationSeconds = 0;
    m_formatCtx = (void*)0x1; // stub
#endif

#ifndef PC_SIMULATOR
    // Open AudioOut on PS4
    // SceAudioOutParam param; ...
    // m_audioHandle = sceAudioOutOpen(...);
#endif

    return true;
}

void MediaPlayer::deinitDecoder() {
#ifdef HAS_FFMPEG
    if (m_formatCtx) {
        AVFormatContext* fmt = (AVFormatContext*)m_formatCtx;
        avformat_close_input(&fmt);
        m_formatCtx = nullptr;
    }
#else
    m_formatCtx = nullptr;
#endif

#ifndef PC_SIMULATOR
    // if (m_audioHandle) sceAudioOutClose(m_audioHandle);
#endif
}

bool MediaPlayer::play(const std::string& path) {
    if (!FileUtils::exists(path)) {
        Logger::error("File not found: %s", path.c_str());
        m_state = PlayerState::ERROR;
        return false;
    }

    stop();

    MediaType type;
    if (!detectMediaType(path, type)) {
        Logger::warn("Unknown media type, trying to play anyway: %s", path.c_str());
        type = MediaType::VIDEO;
    }

    m_currentInfo.path = path;
    m_currentInfo.title = FileUtils::getFileName(path);
    m_currentInfo.type = type;

    if (!initDecoder(path)) {
        m_state = PlayerState::ERROR;
        return false;
    }

    // Add to playlist if not there
    auto it = std::find(m_playlist.begin(), m_playlist.end(), path);
    if (it == m_playlist.end()) {
        m_playlist.push_back(path);
        m_playlistIndex = m_playlist.size() - 1;
    } else {
        m_playlistIndex = std::distance(m_playlist.begin(), it);
    }

    m_state = PlayerState::PLAYING;
    m_currentTime = 0;
    Logger::info("▶ Playing: %s [%s] Duration: %ds", path.c_str(), type==MediaType::VIDEO?"VIDEO":"AUDIO", m_currentInfo.durationSeconds);
    
    if (m_onTrackChanged) m_onTrackChanged(path);
    return true;
}

void MediaPlayer::pause() {
    if (m_state == PlayerState::PLAYING) {
        m_state = PlayerState::PAUSED;
        Logger::info("⏸ Paused at %ds", m_currentTime);
    }
}

void MediaPlayer::resume() {
    if (m_state == PlayerState::PAUSED) {
        m_state = PlayerState::PLAYING;
        Logger::info("▶ Resumed");
    }
}

void MediaPlayer::togglePause() {
    if (m_state == PlayerState::PLAYING) pause();
    else if (m_state == PlayerState::PAUSED) resume();
    else if (m_state == PlayerState::STOPPED && !m_playlist.empty()) {
        play(m_playlist[m_playlistIndex >=0 ? m_playlistIndex : 0]);
    }
}

void MediaPlayer::stop() {
    if (m_state != PlayerState::STOPPED) {
        Logger::info("⏹ Stopped");
    }
    deinitDecoder();
    m_state = PlayerState::STOPPED;
    m_currentTime = 0;
}

void MediaPlayer::seek(int seconds) {
    if (m_state == PlayerState::STOPPED) return;
    if (seconds < 0) seconds = 0;
    if (m_currentInfo.durationSeconds > 0 && seconds > m_currentInfo.durationSeconds) seconds = m_currentInfo.durationSeconds;
    
    m_currentTime = seconds;
#ifdef HAS_FFMPEG
    if (m_formatCtx) {
        AVFormatContext* fmt = (AVFormatContext*)m_formatCtx;
        int64_t ts = (int64_t)seconds * AV_TIME_BASE;
        av_seek_frame(fmt, -1, ts, AVSEEK_FLAG_BACKWARD);
    }
#endif
    Logger::info("Seek to %d/%d sec", m_currentTime, m_currentInfo.durationSeconds);
}

void MediaPlayer::seekRelative(int delta) {
    seek(m_currentTime + delta);
}

void MediaPlayer::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    m_volume = volume;
#ifndef PC_SIMULATOR
    // sceAudioOutSetVolume(m_audioHandle, ...)
#endif
    Logger::info("Volume: %d%%", m_volume);
}

void MediaPlayer::next() {
    if (m_playlist.empty()) return;
    m_playlistIndex = (m_playlistIndex + 1) % m_playlist.size();
    play(m_playlist[m_playlistIndex]);
}

void MediaPlayer::previous() {
    if (m_playlist.empty()) return;
    m_playlistIndex = (m_playlistIndex - 1 + m_playlist.size()) % m_playlist.size();
    play(m_playlist[m_playlistIndex]);
}

void MediaPlayer::addToPlaylist(const std::string& path) {
    if (std::find(m_playlist.begin(), m_playlist.end(), path) == m_playlist.end()) {
        m_playlist.push_back(path);
        if (m_playlistIndex == -1) m_playlistIndex = 0;
    }
}

void MediaPlayer::clearPlaylist() {
    m_playlist.clear();
    m_playlistIndex = -1;
}

void MediaPlayer::update() {
    if (m_state == PlayerState::PLAYING) {
        // In real decoder, increment based on decoded frames
        // Here we simulate time progression
        static int frameCounter = 0;
        frameCounter++;
        if (frameCounter >= 60) { // 1 sec at 60fps
            frameCounter = 0;
            m_currentTime++;
            if (m_currentInfo.durationSeconds > 0 && m_currentTime >= m_currentInfo.durationSeconds) {
                if (m_autoPlay && m_playlist.size() > 1) {
                    next();
                } else if (m_loop) {
                    seek(0);
                } else {
                    stop();
                }
            }
        }
    }
}
