#include "utils/file_utils.h"
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <cctype>
#include <cstdio>

std::vector<FileEntry> FileUtils::listDirectory(const std::string& path) {
    std::vector<FileEntry> result;
    DIR* dir = opendir(path.c_str());
    if (!dir) return result;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        
        std::string fullPath = path;
        if (fullPath.back() != '/') fullPath += "/";
        fullPath += name;

        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            FileEntry fe;
            fe.name = name;
            fe.path = fullPath;
            fe.isDirectory = S_ISDIR(st.st_mode);
            fe.size = st.st_size;
            result.push_back(fe);
        } else {
            // Fallback if stat fails
            FileEntry fe;
            fe.name = name;
            fe.path = fullPath;
            fe.isDirectory = (entry->d_type == DT_DIR);
            fe.size = 0;
            result.push_back(fe);
        }
    }
    closedir(dir);
    return result;
}

bool FileUtils::exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool FileUtils::isDirectory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

std::string FileUtils::getParent(const std::string& path) {
    if (path == "/" || path.empty()) return path;
    std::string p = path;
    // Remove trailing slash
    if (p.back() == '/') p.pop_back();
    size_t pos = p.find_last_of('/');
    if (pos == std::string::npos) return ".";
    if (pos == 0) return "/";
    return p.substr(0, pos);
}

std::string FileUtils::getExtension(const std::string& path) {
    size_t dot = path.find_last_of('.');
    size_t slash = path.find_last_of('/');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return "";
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string FileUtils::getFileName(const std::string& path) {
    size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) return path;
    return path.substr(slash + 1);
}

std::string FileUtils::formatSize(uint64_t bytes) {
    char buf[64];
    if (bytes < 1024) snprintf(buf, sizeof(buf), "%llu B", (unsigned long long)bytes);
    else if (bytes < 1024*1024) snprintf(buf, sizeof(buf), "%.1f KB", bytes/1024.0);
    else if (bytes < 1024*1024*1024) snprintf(buf, sizeof(buf), "%.1f MB", bytes/(1024.0*1024));
    else snprintf(buf, sizeof(buf), "%.2f GB", bytes/(1024.0*1024*1024));
    return std::string(buf);
}

bool FileUtils::isMediaFile(const std::string& filename) {
    std::string ext = getExtension(filename);
    std::vector<std::string> mediaExts = {
        "mp4","mkv","avi","mov","flv","webm","m4v","mpg","mpeg","3gp","ts",
        "mp3","flac","wav","aac","ogg","wma","m4a","opus","aiff"
    };
    return std::find(mediaExts.begin(), mediaExts.end(), ext) != mediaExts.end();
}

std::vector<std::string> FileUtils::getAvailableMounts() {
    std::vector<std::string> mounts = {"/mnt/usb0", "/mnt/usb1", "/data", "/mnt/sandbox/pfsmnt"};
    std::vector<std::string> available;
    for (auto &m : mounts) if (exists(m)) available.push_back(m);
    return available;
}
