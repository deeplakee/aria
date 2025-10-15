#ifndef ARIA_H
#define ARIA_H

namespace aria {

class AriaVM;

using AriaEnv = AriaVM;

inline constexpr const char *platform =
#ifdef _WIN32
#ifdef _WIN64
    "windows64"
#else
    "windows32"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#include "TargetConditionals.h"

#if TARGET_OS_IPHONE
    "ios"
#elif TARGET_OS_MAC
    "macos"
#else
    "apple_unknown"
#endif
#elif __ANDROID__
    "android"
#elif __linux__
    "linux"
#elif __unix__
    "unix"
#else
    "unknown_platform"
#endif
    ;

inline constexpr const char *version = "0.1";
inline constexpr const char *AriaProgramName = "aria";
inline constexpr const char *AriaSourceSuffix = ".aria";
inline constexpr const char *tmp_aria_file_name = "__tmp_aria_file__.aria";
inline constexpr const char *tmp_aria_file_path = "./__tmp_aria_file__.aria";

inline constexpr const char *overloadingAdd_FunName = "__add__";
inline constexpr const char *overloadingSub_FunName = "__sub__";
inline constexpr const char *overloadingMul_FunName = "__mul__";
inline constexpr const char *overloadingDiv_FunName = "__div__";
inline constexpr const char *overloadingMod_FunName = "__mod__";
inline constexpr const char *overloadingEqual_FunName = "__equal__";
} // namespace aria

#endif //ARIA_H
