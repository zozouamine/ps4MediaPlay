#pragma once
#include <string>
#include <vector>
#include <functional>

enum class MediaType { UNKNOWN, AUDIO, VIDEO };
enum class PlayerState { STOPPED, PLAYING, PAUSED, BUFFERING, ERROR };

struct MediaInfo {
    std::string path;
    std::string title;
    MediaType type;
    int durationSeconds; // total
    int width, height;
    std::string codec;
    bool hasSubtitles;
};

class MediaPlayer {
public:
    MediaPlayer();
    ~MediaPlayer();

    // Core controls - mapped to DS4
    bool play(const std::string& path);
    void pause();
    void resume();
    void togglePause();
    void stop();
    void seek(int seconds); // absolute
    void seekRelative(int deltaSeconds); // L2/R2
    void setVolume(int volume); // 0-100
    void next();
    void previous();

    // Playlist
    void addToPlaylist(const std::string& path);
    void clearPlaylist();
    void setAutoPlay(bool v) { m_autoPlay = v; }
    std::vector<std::string> getPlaylist() const { return m_playlist; }

    // Info
    PlayerState getState() const { return m_state; }
    MediaInfo getMediaInfo() const { return m_currentInfo; }
    int getCurrentTime() const { return m_currentTime; }
    int getDuration() const { return m_currentInfo.durationSeconds; }
    int getVolume() const { return m_volume; }
    bool isPlaying() const { return m_state == PlayerState::PLAYING; }

    void update(); // call each frame
    void setOnTrackChanged(std::function<void(const std::string&)> cb) { m_onTrackChanged = cb; }

private:
    bool detectMediaType(const std::string& path, MediaType& out);
    bool initDecoder(const std::string& path);
    void deinitDecoder();

    PlayerState m_state;
    MediaInfo m_currentInfo;
    int m_currentTime;
    int m_volume;
    bool m_autoPlay;
    bool m_loop;

    std::vector<std::string> m_playlist;
    int m_playlistIndex;

    // Decoder handles (FFmpeg on PS4)
    void* m_formatCtx;  // AVFormatContext*
    void* m_codecCtx;   // AVCodecContext*
    void* m_audioHandle; // SceAudioOut handle

    std::function<void(const std::string&)> m_onTrackChanged;
};
