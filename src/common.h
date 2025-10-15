#ifndef COMMON_H
#define COMMON_H

static_assert(sizeof(void *) == 8, "This program requires a 64-bit system.");

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace aria {

using StringView = std::string_view;

using String = std::string;

template<typename T>
using List = std::vector<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T1, typename T2>
using Map = std::unordered_map<T1, T2>;

template<typename T1, typename T2>
using Pair = std::pair<T1, T2>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename... Args>
using Tuple = std::tuple<Args...>;

#ifdef DEBUG_MODE
#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_COMPILED_CODE
#define DEBUG_PRINT_CODE_AST
#define DEBUG_PRINT_SRC_CODE
#define DEBUG_TRACE_STRING_OBJECT_CREATE
#define DEBUG_STRESS_GC
#define DEBUG_PRINT_IMPORT_MODULE_PATH
#define DEBUG_LOG_GC
#include <cassert>
#endif

#ifdef DISABLE_DEBUG_TRACE_EXECUTION
#ifdef DEBUG_TRACE_EXECUTION
#undef DEBUG_TRACE_EXECUTION
#endif
#endif

#ifdef DISABLE_DEBUG_PRINT_COMPILED_CODE
#ifdef DEBUG_PRINT_COMPILED_CODE
#undef DEBUG_PRINT_COMPILED_CODE
#endif
#endif

#ifdef DISABLE_DEBUG_PRINT_CODE_AST
#ifdef DEBUG_PRINT_CODE_AST
#undef DEBUG_PRINT_CODE_AST
#endif
#endif

#ifdef DISABLE_DEBUG_PRINT_SRC_CODE
#ifdef DEBUG_PRINT_SRC_CODE
#undef DEBUG_PRINT_SRC_CODE
#endif
#endif

#ifdef DISABLE_DEBUG_TRACE_STRING_OBJECT_CREATE
#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
#undef DEBUG_TRACE_STRING_OBJECT_CREATE
#endif
#endif

#ifdef DISABLE_DEBUG_STRESS_GC
#ifdef DEBUG_STRESS_GC
#undef DEBUG_STRESS_GC
#endif
#endif

#ifdef DISABLE_DEBUG_PRINT_IMPORT_MODULE_PATH
#ifdef DEBUG_PRINT_IMPORT_MODULE_PATH
#undef DEBUG_PRINT_IMPORT_MODULE_PATH
#endif
#endif

///////////////////////
#ifdef DEBUG_MODE
#define DISABLE_DEBUG_LOG_GC
#endif

#ifdef DISABLE_DEBUG_LOG_GC
#ifdef DEBUG_LOG_GC
#undef DEBUG_LOG_GC
#endif
#endif

} // namespace aria

#endif //COMMON_H
