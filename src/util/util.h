#ifndef ARIA_UTIL_H
#define ARIA_UTIL_H

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
void *to_void_ptr(T ptr)
{
    return static_cast<void *>(ptr);
}

inline bool is_digit(char ch)
{
    return isdigit(ch);
}

inline bool is_alpha(char ch)
{
    return isalpha(ch) || ch == '_';
}

inline bool is_alpha_num(char ch)
{
    return isalpha(ch) || isdigit(ch);
}

inline std::array<uint8_t, 2> split_word(uint16_t word)
{
    return {static_cast<uint8_t>(word & 0xFF), static_cast<uint8_t>(word >> 8)};
}

inline std::array<uint8_t, 4> split_dword(uint32_t word)
{
    return {
        static_cast<uint8_t>(word & 0xFF),
        static_cast<uint8_t>((word >> 8) & 0xFF),
        static_cast<uint8_t>((word >> 16) & 0xFF),
        static_cast<uint8_t>((word >> 24) & 0xFF)};
}

inline bool is_zero(double x)
{
    constexpr double epsilon = 1e-10; // 允许的误差范围
    return std::abs(x) < epsilon;
}

uint32_t next_power_of_2(uint32_t a);

String escape_braces(String s);

String read_file(const String &path);

// Get the full directory of current program.
// If it fails, return an empty string "".
String get_program_directory();

// Get the working directory of current program.
String get_working_directory();

// Get absolute path of file_path.
String get_absolute_path(const String &current_directory, const String &file_path);

bool is_file_path(const String& path);

/**
 * @brief 根据文件A的完整路径和文件B的相对或完整路径，返回文件B的完整路径。
 *
 * @param current_file_path 文件A的完整路径
 * @param module_path 模块名/路径
 * @return String 模块的完整路径；如果路径无效或出错则返回空字符串
 */
String resolve_relative_path(const String &current_file_path, const String &module_path);

String get_absolute_module_path(const String &current_file_path, const String &module_path);

} // namespace aria

#endif //ARIA_UTIL_H
