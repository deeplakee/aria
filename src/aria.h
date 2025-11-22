#ifndef ARIA_ARIA_H
#define ARIA_ARIA_H

#include "sys.h"

namespace aria {

class AriaVM;

using AriaEnv = AriaVM;

inline constexpr const char *platform =
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

#endif //ARIA_ARIA_H
