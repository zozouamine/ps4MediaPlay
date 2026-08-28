#pragma once
#include <string>
#include <vector>

struct FileEntry {
    std::string name;
    std::string path;
    bool isDirectory;
    uint64_t size;
};

class FileUtils {
public:
    static std::vector<FileEntry> listDirectory(const std::string& path);
    static bool exists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static std::string getParent(const std::string& path);
    static std::string getExtension(const std::string& path);
    static std::string getFileName(const std::string& path);
    static std::string formatSize(uint64_t bytes);
    static bool isMediaFile(const std::string& filename);
    static std::vector<std::string> getAvailableMounts();
};
