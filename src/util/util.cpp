#include "util/util.h"

#include "aria.h"
#include "sys.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

#if defined(SYS_WINDOWS)
    #include <windows.h>
#elif defined(SYS_MACOS) || defined(SYS_IOS)
    #include <mach-o/dyld.h>
#elif defined(SYS_LINUX) || defined(SYS_ANDROID) || defined(SYS_FREEBSD)
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

String read_file(const String &path)
{
    String file_path = get_absolute_path(get_working_directory(), path);
    if (file_path.empty()) {
        throw std::runtime_error("could not get absolute path");
    }

    std::ifstream file(file_path, std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

String get_program_directory()
{
    static String cached_path;
    if (!cached_path.empty()) {
        return cached_path;
    }

    String path;
#if defined(SYS_WINDOWS)
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return path;
    }
#elif defined(SYS_MACOS) || defined(SYS_IOS)
    char buffer[PATH_MAX];
    uint32_t length = PATH_MAX;
    if (_NSGetExecutablePath(buffer, &length) != 0) {
        return path;
    }
#elif defined(SYS_LINUX) || defined(SYS_ANDROID) || defined(SYS_FREEBSD)
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (length <= 0 || length >= PATH_MAX) {
        return path;
    }
#endif
    buffer[length - 1] = '\0';
    path = fs::path(buffer).parent_path().string();
    cached_path = path;
    return cached_path;
}

String get_working_directory()
{
    static String cached_path;
    if (!cached_path.empty()) {
        return cached_path;
    }
    cached_path = fs::current_path().string();
    return cached_path;
}

String get_absolute_path(const String &current_directory, const String &file_path)
{
    try {
        if (current_directory.empty() || file_path.empty()) {
            return "";
        }

        fs::path path_a = current_directory;
        fs::path path_b = file_path;

        if (path_b.is_absolute()) {
            return fs::weakly_canonical(path_b).string();
        }
        return fs::weakly_canonical(path_a / path_b).string();
    } catch (...) {
        return "";
    }
}

bool is_file_path(const String &path)
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

String resolve_relative_path(const String &current_file_path, const String &module_path)
{
    if (module_path.empty())
        return "";

    fs::path base_dir = fs::absolute(fs::path(current_file_path).parent_path());
    fs::path resolved_path;

    // 1️⃣ 相对路径（例如 "../math.aria" 或 "./utils/math.aria"）
    if (module_path.rfind("./", 0) == 0 || module_path.rfind("../", 0) == 0) {
        fs::path target = base_dir / module_path;
        if (fs::exists(target))
            resolved_path = fs::canonical(target);
        else
            return ""; // 文件不存在
    }
    // 2️⃣ 绝对路径
    else if (fs::path(module_path).is_absolute()) {
        fs::path target = fs::path(module_path);
        if (fs::exists(target))
            resolved_path = fs::canonical(target);
        else
            return ""; // 文件不存在
    }
    // 3️⃣ 模块名（如 "math", "core.utils", "network/http"）
    else {
        String mod = module_path;
        for (auto &ch : mod) {
            if (ch == '.')
                ch = '/'; // 将模块名的点替换为目录分隔符
        }

        fs::path try1 = base_dir / (mod + k_aria_source_suffix);
        fs::path try2 = base_dir / mod / "index.aria";

        if (fs::exists(try1))
            resolved_path = fs::canonical(try1);
        else if (fs::exists(try2))
            resolved_path = fs::canonical(try2);
        else
            return "";
    }

    return resolved_path.string();
}

String get_absolute_module_path(const String &current_file_path, const String &module_path)
{
    String path = resolve_relative_path(current_file_path, module_path);
    if (path.empty()) {
        path = resolve_relative_path(get_program_directory() + "/lib/", module_path);
    }
    return path;
}

} // namespace aria
