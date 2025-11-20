#ifndef UTIL_H
#define UTIL_H

#include "common.h"

#include <format>
#include <utility>

namespace aria {

using std::swap;

using std::format;

template<typename... Args>
void print(std::ostream &os, const String &fmt, Args &&...args)
{
    os << std::vformat(fmt, std::make_format_args(args...));
}

template<typename... Args>
void print(const String &fmt, Args &&...args)
{
    print(std::cout, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void println(std::ostream &os, const String &fmt, Args &&...args)
{
    os << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

template<typename... Args>
void println(const String &fmt, Args &&...args)
{
    println(std::cout, fmt, std::forward<Args>(args)...);
}

template<typename T>
concept PointerType = std::is_pointer_v<T>;

template<PointerType T>
void *toVoidPtr(T ptr)
{
    return static_cast<void *>(ptr);
}

#define var_to_str(var) #var

#if defined(__GNUC__) || defined(__clang__)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

inline bool isDigit(char ch)
{
    return isdigit(ch);
}

inline bool isAlpha(char ch)
{
    return isalpha(ch) || ch == '_';
}

inline bool isAlphaNum(char ch)
{
    return isalpha(ch) || isdigit(ch);
}

inline std::array<uint8_t, 2> splitWord(uint16_t word)
{
    return {static_cast<uint8_t>(word & 0xFF), static_cast<uint8_t>(word >> 8)};
}

inline std::array<uint8_t, 4> splitDWord(uint32_t word)
{
    return {
        static_cast<uint8_t>(word & 0xFF),
        static_cast<uint8_t>((word >> 8) & 0xFF),
        static_cast<uint8_t>((word >> 16) & 0xFF),
        static_cast<uint8_t>((word >> 24) & 0xFF)};
}

inline bool isZero(double x)
{
    constexpr double epsilon = 1e-10; // 允许的误差范围
    return std::abs(x) < epsilon;
}

uint32_t nextPowerOf2(uint32_t a);

String escapeBraces(String s);

String readFile(const String &path);

// Get the full directory of current program.
// If it fails, return an empty string "".
String getProgramDirectory();

// Get the working directory of current program.
String getWorkingDirectory();

// Get absolute path of filePath.
String getAbsolutePath(const String &currentDirectory, const String &filePath);

bool isFilePath(const String& path);

/**
 * @brief 根据文件A的完整路径和文件B的相对或完整路径，返回文件B的完整路径。
 *
 * @param currentFilePath 文件A的完整路径
 * @param modulePath 模块名/路径
 * @return String 模块的完整路径；如果路径无效或出错则返回空字符串
 */
String resolveRelativePath(const String &currentFilePath, const String &modulePath);

String getAbsoluteModulePath(const String &currentFilePath, const String &modulePath);

} // namespace aria

#endif // UTIL_H
