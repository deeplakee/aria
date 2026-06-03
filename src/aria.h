#ifndef ARIA_ARIA_H
#define ARIA_ARIA_H

#include "sys.h"
#include <cstddef>

namespace aria {

class AriaVM;

using AriaEnv = AriaVM;

inline constexpr const char *k_platform =
#if defined(SYS_WINDOWS)
    "windows"
#elif defined(SYS_LINUX)
    "linux"
#elif defined(SYS_MACOS)
    "macos"
#elif defined(SYS_FREEBSD)
    "freebsd"
#elif defined(SYS_ANDROID)
    "android"
#elif defined(SYS_IOS)
    "ios"
#else
    "unknown_platform"
#endif
    ;

inline constexpr const char *k_version = "0.1";
inline constexpr const char *k_aria_program_name = "aria";
inline constexpr const char *k_aria_source_suffix = ".aria";

inline constexpr const char *k_init_fun_name = "init";
inline constexpr size_t k_init_fun_name_len = 4;

inline constexpr const char *k_to_string_fun_name = "to_str";
inline constexpr size_t k_to_string_fun_name_len = 6;

inline constexpr const char *k_overloading_add_fun_name = "__add__";
inline constexpr const char *k_overloading_sub_fun_name = "__sub__";
inline constexpr const char *k_overloading_mul_fun_name = "__mul__";
inline constexpr const char *k_overloading_div_fun_name = "__div__";
inline constexpr const char *k_overloading_mod_fun_name = "__mod__";
inline constexpr const char *k_overloading_equal_fun_name = "__equal__";
} // namespace aria

#endif //ARIA_ARIA_H
