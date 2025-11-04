cmake_minimum_required(VERSION 3.27)
project(aria LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 默认关闭测试
option(BUILD_TESTS "Enable building tests" OFF)

# 默认设置为 Release 模式
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()


#################
# aria_core 构建 #
#################
add_library(aria_core STATIC
        src/util/util.h
        src/common.h
        src/util/util.cpp
        src/value/nanBoxing.h
        src/value/value.h
        src/memory/gc.h
        src/memory/gc.cpp
        src/object/object.h
        src/value/valueArray.h
        src/value/valueArray.cpp
        src/value/valueStack.h
        src/value/valueStack.cpp
        src/value/value.cpp
        src/value/valueHashTable.h
        src/value/valueHashTable.cpp
        src/aria.h
        src/object/objString.h
        src/object/objString.cpp
        src/util/hash.h
        src/memory/stringPool.h
        src/memory/stringPool.cpp
        src/object/objException.h
        src/object/objException.cpp
        src/object/objList.h
        src/object/objList.cpp
        src/object/objMap.h
        src/object/objMap.cpp
        src/compile/token.h
        src/compile/token.cpp
        src/compile/lexer.h
        src/compile/lexer.cpp
        src/object/object.cpp
        src/compile/ast.h
        src/compile/parser.h
        src/compile/parser.cpp
        src/chunk/code.h
        src/chunk/chunk.h
        src/chunk/chunk.cpp
        src/error/error.h
        src/chunk/disassembler.cpp
        src/chunk/disassembler.h
        src/compile/functionContext.cpp
        src/compile/functionContext.h
        src/compile/ast.cpp
        src/error/ariaException.h
        src/util/fileTable.h
        src/object/objFunction.cpp
        src/object/objFunction.h
        src/object/funDef.h
        src/runtime/vm.cpp
        src/runtime/vm.h
        src/compile/compiler.cpp
        src/compile/compiler.h
        src/runtime/callFrame.h
        src/error/ErrorCode.h
        src/runtime/exceptionFrame.h
        src/object/objUpvalue.cpp
        src/object/objUpvalue.h
        src/compile/byteCodeGenerator.cpp
        src/compile/byteCodeGenerator.h
        src/compile/astVisitor.h
        src/util/lock.h
        src/object/objNativeFn.cpp
        src/object/objNativeFn.h
        src/runtime/native.cpp
        src/runtime/native.h
        src/object/objClass.cpp
        src/object/objClass.h
        src/object/objInstance.cpp
        src/object/objInstance.h
        src/object/objBoundMethod.cpp
        src/object/objBoundMethod.h
        src/object/objMapBuiltin.cpp
        src/object/objListBuiltin.cpp
        src/object/objStringBuiltin.cpp
        src/object/iterator.cpp
        src/object/iterator.h
        src/object/objIterator.cpp
        src/object/objIterator.h
        src/object/objIteratorBuiltin.cpp
        src/util/nativeUtil.h
        src/object/objModule.cpp
        src/object/objModule.h
        src/ariaApi.h
        src/runtime/vmState.h
        src/runtime/vmState.cpp
        src/debugger/debugger.cpp
        src/debugger/debugger.h
        src/debugger/breakpoint.cpp
        src/debugger/breakpoint.h
)

# 设置头文件路径
target_include_directories(aria_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(aria_core PUBLIC ${CMAKE_SOURCE_DIR})

# 在 Debug 模式下设置 DEBUG_MODE 宏
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(aria_core PRIVATE DEBUG_MODE)
    set(BUILD_TESTS ON CACHE BOOL "Enable building tests in Debug mode" FORCE)
endif ()

# 在 Release 模式下取消一些 debug 宏定义
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_TRACE_EXECUTION)
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_PRINT_COMPILED_CODE)
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_PRINT_CODE_AST)
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_PRINT_SRC_CODE)
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_TRACE_STRING_OBJECT_CREATE)
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_STRESS_GC)
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_LOG_GC)
    target_compile_definitions(aria_core PRIVATE DISABLE_DEBUG_PRINT_IMPORT_MODULE_PATH)
    set(RELEASE_CXX_FLAGS
            -O3
            -DNDEBUG
            -march=native
            -mtune=native
            -flto
            -fno-stack-protector
            -fomit-frame-pointer
    )
    set(RELEASE_LINK_FLAGS
            -flto
            -s
            -Wl,--gc-sections
    )
endif ()

# 链接库
target_link_libraries(aria_core PUBLIC m)
target_compile_features(aria_core PUBLIC cxx_std_20)


#############
# aria 构建 ##
#############
add_executable(aria
        src/main.cpp)
target_include_directories(aria PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(aria PRIVATE aria_core)

# 可选开启 Readline 支持
option(USE_READLINE "Enable readline support" ON)

if (USE_READLINE)
    find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h)
    find_library(READLINE_LIBRARY NAMES readline)

    if (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
        message(STATUS "Found readline: ${READLINE_LIBRARY}")
        target_include_directories(aria PRIVATE ${READLINE_INCLUDE_DIR})
        target_link_libraries(aria PRIVATE ${READLINE_LIBRARY})
        target_compile_definitions(aria PRIVATE ENABLE_READLINE=1)
    else ()
        message(WARNING "readline not found — disabling readline support")
        target_compile_definitions(aria PRIVATE ENABLE_READLINE=0)
    endif ()
else ()
    message(STATUS "USE_READLINE is OFF — readline support disabled")
    target_compile_definitions(aria PRIVATE ENABLE_READLINE=0)
endif ()

################
# debugger 构建 #
################
add_executable(ariadb
        src/debugger/main.cpp)
target_include_directories(ariadb PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(ariadb PRIVATE aria_core)


# 测试
if (BUILD_TESTS)
    enable_testing()

    # 下载 GoogleTest（如果未提前下载）
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    find_package(Threads REQUIRED)

    ###########
    # 测试构建 #
    ###########
    add_executable(all_tests
            tests/value/test_value.cpp
            tests/util/test_print.cpp
            tests/util/test_fileop.cpp
            tests/value/test_valueArray.cpp
            tests/gc/gc_init.h
            tests/object/test_ObjString.cpp
            tests/value/test_valueHashTable.cpp
            tests/value/test_valueStack.cpp
            tests/compile/test_token.cpp
            tests/compile/test_lexer.cpp
            tests/compile/test_parser.cpp
            tests/compile/test_generateByteCode.cpp
            tests/util/test_util.cpp)

    target_include_directories(all_tests PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(all_tests PRIVATE aria_core)
    target_link_libraries(all_tests PRIVATE GTest::gtest_main Threads::Threads)

    # 启用测试发现
    include(GoogleTest)
    gtest_discover_tests(all_tests
            # 可选参数：
            TEST_PREFIX "unit_"          # 测试名字前缀
            PROPERTIES LABELS "unit"     # 测试标签
    )
endif ()