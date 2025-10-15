#include "util/util.h"

#include "aria.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h> // macOS 专用 API
#else
#include <limits.h> // PATH_MAX
#include <unistd.h> // Linux/Unix readlink
#endif

namespace aria {

namespace fs = std::filesystem;

uint32_t next_power_of_2(uint32_t a)
{
    if (a == UINT32_MAX) {
        return UINT32_MAX;
    }
    int begin = 16;
    while (begin < a) {
        begin *= 2;
    }
    return begin;
}

String escape_braces(String s)
{
    size_t pos = 0;
    while ((pos = s.find('{', pos)) != std::string::npos) {
        s.replace(pos, 1, "{{");
        pos += 2;
    }
    pos = 0;
    while ((pos = s.find('}', pos)) != std::string::npos) {
        s.replace(pos, 1, "}}");
        pos += 2;
    }
    return s;
}

String readFile(const String &path)
{
    String filePath = getAbsolutePath(getWorkingDirectory(), path);
    if (filePath.empty()) {
        throw std::runtime_error("could not get absolute path");
    }

    std::ifstream file(filePath, std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

String getProgramDirectory()
{
    static String cachedPath;
    if (!cachedPath.empty()) {
        return cachedPath;
    }

    String path;
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return path;
    }
#elif defined(__APPLE__)
    char buffer[PATH_MAX];
    uint32_t length = PATH_MAX;
    if (_NSGetExecutablePath(buffer, &length) != 0) {
        return path;
    }
#else
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (length <= 0 || length >= PATH_MAX) {
        return path;
    }
#endif
    buffer[length - 1] = '\0';
    path = fs::path(buffer).parent_path().string();
    cachedPath = path;
    return cachedPath;
}

String getWorkingDirectory()
{
    static String cachedPath;
    if (!cachedPath.empty()) {
        return cachedPath;
    }
    cachedPath = fs::current_path().string();
    return cachedPath;
}

String getAbsolutePath(const String &currentDirectory, const String &filePath)
{
    try {
        if (currentDirectory.empty() || filePath.empty()) {
            return "";
        }

        fs::path pathA = currentDirectory;
        fs::path pathB = filePath;

        if (pathB.is_absolute()) {
            return fs::weakly_canonical(pathB).string();
        }
        return fs::weakly_canonical(pathA / pathB).string();
    } catch (...) {
        return "";
    }
}

bool isFilePath(const String &path)
{
    if (path.empty())
        return false;
    String trimmed = path;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    if (trimmed.empty())
        return false;

    // 1️⃣ 排除 URL 或明显非路径字符串
    static const std::regex url_pattern(R"(^[a-zA-Z]+://)");
    if (std::regex_search(trimmed, url_pattern))
        return false;

    // 2️⃣ 判断是否包含路径分隔符（Windows 或 Unix）
    if (trimmed.find('/') == std::string::npos && trimmed.find('\\') == std::string::npos)
        return false;

    // 3️⃣ 使用 C++17 filesystem 检查路径格式是否合法
    try {
        fs::path p(trimmed);
        // 判断是否为绝对路径或相对路径（都算是路径）
        return p.is_absolute() || !p.empty();
    } catch (...) {
        return false;
    }
}

String resolveRelativePath(const String &currentFilePath, const String &modulePath)
{
    if (modulePath.empty())
        return "";

    fs::path baseDir = fs::absolute(fs::path(currentFilePath).parent_path());
    fs::path resolvedPath;

    // 1️⃣ 相对路径（例如 "../math.aria" 或 "./utils/math.aria"）
    if (modulePath.rfind("./", 0) == 0 || modulePath.rfind("../", 0) == 0) {
        fs::path target = baseDir / modulePath;
        if (fs::exists(target))
            resolvedPath = fs::canonical(target);
        else
            return ""; // 文件不存在
    }
    // 2️⃣ 绝对路径
    else if (fs::path(modulePath).is_absolute()) {
        fs::path target = fs::path(modulePath);
        if (fs::exists(target))
            resolvedPath = fs::canonical(target);
        else
            return ""; // 文件不存在
    }
    // 3️⃣ 模块名（如 "math", "core.utils", "network/http"）
    else {
        String mod = modulePath;
        for (auto &ch : mod) {
            if (ch == '.')
                ch = '/'; // 将模块名的点替换为目录分隔符
        }

        fs::path try1 = baseDir / (mod + AriaSourceSuffix);
        fs::path try2 = baseDir / mod / "index.aria";

        if (fs::exists(try1))
            resolvedPath = fs::canonical(try1);
        else if (fs::exists(try2))
            resolvedPath = fs::canonical(try2);
        else
            return "";
    }

    return resolvedPath.string();
}

String getAbsoluteModulePath(const String &currentFilePath, const String &modulePath)
{
    String path = resolveRelativePath(currentFilePath, modulePath);
    if (path.empty()) {
        path = resolveRelativePath(getProgramDirectory() + "/lib/", modulePath);
    }
    return path;
}

} // namespace aria
