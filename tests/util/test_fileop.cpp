#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "src/util/util.h"

namespace fs = std::filesystem;
using String = std::string;

static String createTempFile(const String &name, const String &content) {
    fs::path temp = fs::temp_directory_path() / name;
    std::ofstream ofs(temp);
    ofs << content;
    ofs.close();
    return temp.string();
}

static void removeTempFile(const String &path) {
    if (fs::exists(path)) fs::remove(path);
}


TEST(FileUtilsTest, GetWorkingDirectory_NotEmpty) {
    String cwd = aria::get_working_directory();
    EXPECT_FALSE(cwd.empty());
    EXPECT_TRUE(fs::exists(cwd));
}

TEST(FileUtilsTest, GetAbsolutePath_Basic) {
    String cwd = aria::get_working_directory();

    String absPath = aria::get_absolute_path(cwd, "test.txt");
    EXPECT_TRUE(absPath.find("test.txt") != String::npos);

    // 绝对路径输入
    String absInput = fs::absolute("test.txt").string();
    String abs2 = aria::get_absolute_path(cwd, absInput);
    EXPECT_EQ(absInput, abs2);
}

TEST(FileUtilsTest, GetAbsolutePath_EmptyInputs) {
    EXPECT_EQ(aria::get_absolute_path("", "x.txt"), "");
    EXPECT_EQ(aria::get_absolute_path("abc", ""), "");
}



TEST(FileUtilsTest, IsFilePath_ValidAndInvalid) {
    EXPECT_TRUE(aria::is_file_path("folder/file.txt"));
    EXPECT_TRUE(aria::is_file_path("/usr/local/bin"));
    EXPECT_FALSE(aria::is_file_path("http://example.com"));
    EXPECT_FALSE(aria::is_file_path("not_a_path"));
    EXPECT_FALSE(aria::is_file_path(""));
}


TEST(FileUtilsTest, ReadFile_Success) {
    String cwd = aria::get_working_directory();
    // 在工作目录下创建临时文件，确保相对路径可用（避免 Windows 跨盘符问题）
    fs::path tempPath = fs::path(cwd) / "__read_test_tmp__.txt";
    {
        std::ofstream ofs(tempPath);
        ofs << "Hello, world!";
    }

    // 测试通过绝对路径读取
    String content1 = aria::read_file(tempPath.string());
    EXPECT_EQ(content1, "Hello, world!");

    // 测试通过相对路径读取
    String relative = fs::relative(tempPath, cwd).string();
    String content2 = aria::read_file(relative);
    EXPECT_EQ(content2, "Hello, world!");

    fs::remove(tempPath);
}

TEST(FileUtilsTest, ReadFile_FileNotFound) {
    EXPECT_THROW(aria::read_file("nonexistent.txt"), std::runtime_error);
}


TEST(FileUtilsTest, ResolveRelativePath_Relative) {
    // 创建虚拟结构
    fs::path base = fs::temp_directory_path() / "resolve_test";
    fs::create_directories(base / "sub");
    fs::path target = base / "file.aria";
    std::ofstream(target) << "dummy";

    String resolved = aria::resolve_relative_path((base / "sub" / "current.aria").string(), "../file.aria");
    EXPECT_EQ(fs::canonical(target).string(), resolved);

    fs::remove_all(base);
}

TEST(FileUtilsTest, ResolveRelativePath_ModuleLike) {
    // 模拟模块结构
    fs::path base = fs::temp_directory_path() / "modules";
    fs::create_directories(base / "core/utils");
    fs::path indexFile = base / "core/utils/index.aria";
    std::ofstream(indexFile) << "module content";

    // 模拟 currentFilePath
    String currentFile = (base / "main.aria").string();
    String resolved = aria::resolve_relative_path(currentFile, "core.utils");
    EXPECT_EQ(fs::canonical(indexFile).string(), resolved);

    fs::remove_all(base);
}


TEST(FileUtilsTest, GetAbsoluteModulePath_Fallback) {
    // 模拟程序目录结构
    fs::path libDir = fs::temp_directory_path() / "aria_lib";
    fs::create_directories(libDir / "math");
    fs::path moduleFile = libDir / "math/index.aria";
    std::ofstream(moduleFile) << "module";

    // 暂时替代 getProgramDirectory()
    String current = (libDir / "main.aria").string();
    String resolved = aria::get_absolute_module_path(current, "math");

    // 由于 resolveRelativePath 用 canonical
    EXPECT_TRUE(fs::exists(resolved));

    fs::remove_all(libDir);
}


TEST(FileUtilsTest, GetProgramDirectory_NotEmpty) {
    String dir = aria::get_program_directory();
    EXPECT_FALSE(dir.empty());
    EXPECT_TRUE(fs::exists(dir));
}
