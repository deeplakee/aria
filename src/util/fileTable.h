#ifndef FILETABLE_H
#define FILETABLE_H

#include "common.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace aria {
class FileTable
{
public:
    struct FileEntry
    {
        String name;         // 文件名
        String content; // 文件内容
    };

    // 注册文件，如果已经存在则返回已有的 id
    // 如果文件不存在则抛异常
    uint32_t addFile(const String &filename)
    {
        auto it = fileToId.find(filename);
        if (it != fileToId.end()) {
            return it->second;
        }

        // 读取文件内容
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) {
            throw std::runtime_error("unable to open file: " + filename);
        }
        std::ostringstream oss;
        oss << ifs.rdbuf();

        FileEntry entry{filename, oss.str()};
        uint32_t id = static_cast<uint32_t>(idToFile.size());
        idToFile.push_back(std::move(entry));
        fileToId[filename] = id;
        return id;
    }

    // 根据 id 获取文件名
    const String &getFilename(uint32_t fileId) const { return idToFile.at(fileId).name; }

    // 根据 id 获取文件内容
    const String &getFileContent(uint32_t fileId) const { return idToFile.at(fileId).content; }

    // 文件总数
    size_t size() const { return idToFile.size(); }

private:
    std::vector<FileEntry> idToFile;                    // id -> 文件信息（名 + 内容）
    std::unordered_map<std::string, uint32_t> fileToId; // 文件名 -> id
};
} // namespace aria

#endif //FILETABLE_H
