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
    String cwd = aria::getWorkingDirectory();
    EXPECT_FALSE(cwd.empty());
    EXPECT_TRUE(fs::exists(cwd));
}

TEST(FileUtilsTest, GetAbsolutePath_Basic) {
    String cwd = aria::getWorkingDirectory();

    String absPath = aria::getAbsolutePath(cwd, "test.txt");
    EXPECT_TRUE(absPath.find("test.txt") != String::npos);

    // 绝对路径输入
    String absInput = fs::absolute("test.txt").string();
    String abs2 = aria::getAbsolutePath(cwd, absInput);
    EXPECT_EQ(absInput, abs2);
}

TEST(FileUtilsTest, GetAbsolutePath_EmptyInputs) {
    EXPECT_EQ(aria::getAbsolutePath("", "x.txt"), "");
    EXPECT_EQ(aria::getAbsolutePath("abc", ""), "");
}



TEST(FileUtilsTest, IsFilePath_ValidAndInvalid) {
    EXPECT_TRUE(aria::isFilePath("folder/file.txt"));
    EXPECT_TRUE(aria::isFilePath("/usr/local/bin"));
    EXPECT_FALSE(aria::isFilePath("http://example.com"));
    EXPECT_FALSE(aria::isFilePath("not_a_path"));
    EXPECT_FALSE(aria::isFilePath(""));
}


TEST(FileUtilsTest, ReadFile_Success) {
    String tempPath = createTempFile("read_test.txt", "Hello, world!");
    String cwd = aria::getWorkingDirectory();

    String relative = fs::relative(tempPath, cwd).string();
    String content = aria::readFile(relative);

    EXPECT_EQ(content, "Hello, world!");

    removeTempFile(tempPath);
}

TEST(FileUtilsTest, ReadFile_FileNotFound) {
    EXPECT_THROW(aria::readFile("nonexistent.txt"), std::runtime_error);
}


TEST(FileUtilsTest, ResolveRelativePath_Relative) {
    // 创建虚拟结构
    fs::path base = fs::temp_directory_path() / "resolve_test";
    fs::create_directories(base / "sub");
    fs::path target = base / "file.aria";
    std::ofstream(target) << "dummy";

    String resolved = aria::resolveRelativePath((base / "sub" / "current.aria").string(), "../file.aria");
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
    String resolved = aria::resolveRelativePath(currentFile, "core.utils");
    EXPECT_EQ(fs::canonical(indexFile).string(), resolved);

    fs::remove_all(base);
}


TEST(FileUtilsTest, GetAbsoluteModulePath_Fallback) {
    // 模拟程序目录结构
    fs::path libdir = fs::temp_directory_path() / "aria_lib";
    fs::create_directories(libdir / "math");
    fs::path moduleFile = libdir / "math/index.aria";
    std::ofstream(moduleFile) << "module";

    // 暂时替代 getProgramDirectory()
    String current = (libdir / "main.aria").string();
    String resolved = aria::getAbsoluteModulePath(current, "math");

    // 由于 resolveRelativePath 用 canonical
    EXPECT_TRUE(fs::exists(resolved));

    fs::remove_all(libdir);
}


TEST(FileUtilsTest, GetProgramDirectory_NotEmpty) {
    String dir = aria::getProgramDirectory();
    EXPECT_FALSE(dir.empty());
    EXPECT_TRUE(fs::exists(dir));
}
