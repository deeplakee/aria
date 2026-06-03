#include <gtest/gtest.h>

#include "src/runtime/vm.h"

using namespace aria;
using testing::internal::CaptureStdout;
using testing::internal::GetCapturedStdout;

class VMTest : public ::testing::Test
{
public:
    void SetUp() override { vm = new AriaVM(); }
    void TearDown() override { delete vm; }

    // 运行代码并捕获 stdout，返回输出中是否包含 expected
    bool runAndExpect(const char *source, const char *expected)
    {
        CaptureStdout();
        auto result = vm->interpret(String{source});
        auto output = GetCapturedStdout();
        if (result != InterpretResult::SUCCESS) {
            ADD_FAILURE() << "Expected SUCCESS but got error. Output:\n" << output;
            return false;
        }
        bool found = output.find(expected) != std::string::npos;
        if (!found) {
            ADD_FAILURE() << "Expected to find '" << expected << "' in output.\nActual output:\n"
                          << output;
        }
        return found;
    }

    // 运行代码并期望编译错误
    void runAndExpectCompileError(const char *source)
    {
        CaptureStdout();
        auto result = vm->interpret(String{source});
        GetCapturedStdout();
        EXPECT_EQ(result, InterpretResult::COMPILE_ERROR);
    }

    // 运行代码并期望运行时错误
    void runAndExpectRuntimeError(const char *source)
    {
        CaptureStdout();
        auto result = vm->interpret(String{source});
        GetCapturedStdout();
        EXPECT_EQ(result, InterpretResult::RUNTIME_ERROR);
    }

    AriaVM *vm = nullptr;
};

// ==================== 基本输出 ====================

TEST_F(VMTest, PrintNumber)
{
    EXPECT_TRUE(runAndExpect("print 42;", "42"));
}

TEST_F(VMTest, PrintString)
{
    EXPECT_TRUE(runAndExpect("print \"hello\";", "hello"));
}

TEST_F(VMTest, PrintBool)
{
    EXPECT_TRUE(runAndExpect("print true;", "true"));
    EXPECT_TRUE(runAndExpect("print false;", "false"));
}

TEST_F(VMTest, PrintNil)
{
    EXPECT_TRUE(runAndExpect("print nil;", "nil"));
}

// ==================== 算术运算 ====================

TEST_F(VMTest, Arithmetic)
{
    EXPECT_TRUE(runAndExpect("print 1 + 2;", "3"));
    EXPECT_TRUE(runAndExpect("print 10 - 3;", "7"));
    EXPECT_TRUE(runAndExpect("print 4 * 5;", "20"));
    EXPECT_TRUE(runAndExpect("print 10 / 4;", "2.5"));
    EXPECT_TRUE(runAndExpect("print 10 % 3;", "1"));
    EXPECT_TRUE(runAndExpect("print -5;", "-5"));
}

TEST_F(VMTest, ArithmeticPrecedence)
{
    EXPECT_TRUE(runAndExpect("print 2 + 3 * 4;", "14"));
    EXPECT_TRUE(runAndExpect("print (2 + 3) * 4;", "20"));
    EXPECT_TRUE(runAndExpect("print 10 - 2 - 3;", "5"));
}

// ==================== 比较运算 ====================

TEST_F(VMTest, Comparison)
{
    EXPECT_TRUE(runAndExpect("print 1 < 2;", "true"));
    EXPECT_TRUE(runAndExpect("print 2 > 1;", "true"));
    EXPECT_TRUE(runAndExpect("print 1 >= 1;", "true"));
    EXPECT_TRUE(runAndExpect("print 1 <= 2;", "true"));
    EXPECT_TRUE(runAndExpect("print 1 == 1;", "true"));
    EXPECT_TRUE(runAndExpect("print 1 != 2;", "true"));
    EXPECT_TRUE(runAndExpect("print 2 < 1;", "false"));
}

// ==================== 逻辑运算 ====================

TEST_F(VMTest, Logic)
{
    EXPECT_TRUE(runAndExpect("print true and true;", "true"));
    EXPECT_TRUE(runAndExpect("print true and false;", "false"));
    EXPECT_TRUE(runAndExpect("print false or true;", "true"));
    EXPECT_TRUE(runAndExpect("print !true;", "false"));
    EXPECT_TRUE(runAndExpect("print !false;", "true"));
}

TEST_F(VMTest, ShortCircuit)
{
    // and 短路：左侧为 false 时右侧不执行
    EXPECT_TRUE(runAndExpect("print false and (1 / 0 == 0);", "false"));
}

// ==================== 字符串操作 ====================

TEST_F(VMTest, StringConcat)
{
    EXPECT_TRUE(runAndExpect("print \"hello\" + \" \" + \"world\";", "hello world"));
}

TEST_F(VMTest, StringEquality)
{
    EXPECT_TRUE(runAndExpect("print \"abc\" == \"abc\";", "true"));
    EXPECT_TRUE(runAndExpect("print \"abc\" == \"xyz\";", "false"));
}

// ==================== 变量 ====================

TEST_F(VMTest, Variable)
{
    EXPECT_TRUE(runAndExpect(R"(
var x = 10;
var y = 20;
print x + y;
)",
        "30"));
}

TEST_F(VMTest, VariableReassign)
{
    EXPECT_TRUE(runAndExpect(R"(
var x = 1;
x = 2;
print x;
)",
        "2"));
}

// ==================== 控制流 ====================

TEST_F(VMTest, IfElse)
{
    EXPECT_TRUE(runAndExpect(R"(
var x = 10;
if (x > 5) {
    print "big";
} else {
    print "small";
}
)",
        "big"));
}

TEST_F(VMTest, WhileLoop)
{
    EXPECT_TRUE(runAndExpect(R"(
var i = 0;
var sum = 0;
while (i < 5) {
    sum = sum + i;
    i = i + 1;
}
print sum;
)",
        "10"));
}

TEST_F(VMTest, ForLoop)
{
    EXPECT_TRUE(runAndExpect(R"(
var sum = 0;
for (var i = 0; i < 5; i = i + 1) {
    sum = sum + i;
}
print sum;
)",
        "10"));
}

TEST_F(VMTest, BreakStatement)
{
    EXPECT_TRUE(runAndExpect(R"(
var i = 0;
while (true) {
    if (i == 3) { break; }
    i = i + 1;
}
print i;
)",
        "3"));
}

TEST_F(VMTest, ContinueStatement)
{
    EXPECT_TRUE(runAndExpect(R"(
var sum = 0;
for (var i = 0; i < 5; i = i + 1) {
    if (i == 2) { continue; }
    sum = sum + i;
}
print sum;
)",
        "8"));
}

// ==================== 函数 ====================

TEST_F(VMTest, FunctionCall)
{
    EXPECT_TRUE(runAndExpect(R"(
fun add(a, b) {
    return a + b;
}
print add(3, 4);
)",
        "7"));
}

TEST_F(VMTest, Recursion)
{
    EXPECT_TRUE(runAndExpect(R"(
fun fib(n) {
    if (n <= 1) { return n; }
    return fib(n - 1) + fib(n - 2);
}
print fib(10);
)",
        "55"));
}

TEST_F(VMTest, Closure)
{
    EXPECT_TRUE(runAndExpect(R"(
fun make_counter() {
    var count = 0;
    fun counter() {
        count = count + 1;
        return count;
    }
    return counter;
}
var c = make_counter();
print c();
print c();
print c();
)",
        "3"));
}

TEST_F(VMTest, DefaultReturnNil)
{
    EXPECT_TRUE(runAndExpect(R"(
fun noop() {}
print noop();
)",
        "nil"));
}

// ==================== 类 ====================

TEST_F(VMTest, ClassBasic)
{
    EXPECT_TRUE(runAndExpect(R"(
class Animal {
    init(name) {
        this.name = name;
    }
    speak() {
        return this.name + " speaks";
    }
}
var a = Animal("Cat");
print a.speak();
)",
        "Cat speaks"));
}

TEST_F(VMTest, ClassInheritance)
{
    EXPECT_TRUE(runAndExpect(R"(
class Base {
    init(x) { this.x = x; }
}
class Derived : Base {
    init(x, y) {
        super.init(x);
        this.y = y;
    }
    sum() { return this.x + this.y; }
}
var d = Derived(3, 4);
print d.sum();
)",
        "7"));
}

// ==================== 列表 ====================

TEST_F(VMTest, ListBasic)
{
    EXPECT_TRUE(runAndExpect(R"(
var list = [1, 2, 3];
print list[0];
print list[2];
print list.size();
)",
        "3"));
}

TEST_F(VMTest, ListMethods)
{
    EXPECT_TRUE(runAndExpect(R"(
var list = [1, 2, 3];
list.append(4);
print list.size();
print list[3];
)",
        "4"));
}

// ==================== Map ====================

TEST_F(VMTest, MapBasic)
{
    EXPECT_TRUE(runAndExpect(R"(
var m = {"a": 1, "b": 2};
print m["a"];
print m["b"];
)",
        "2"));
}

// ==================== 异常 ====================

TEST_F(VMTest, TryCatch)
{
    EXPECT_TRUE(runAndExpect(R"(
try {
    throw "error";
} catch (e) {
    print e;
}
)",
        "error"));
}

TEST_F(VMTest, TryCatchFinally)
{
    EXPECT_TRUE(runAndExpect(R"(
var result = "none";
try {
    result = "try";
    throw "oops";
} catch (e) {
    result = "catch";
}
print result;
)",
        "catch"));
}

// ==================== For-in 循环 ====================

TEST_F(VMTest, ForInList)
{
    EXPECT_TRUE(runAndExpect(R"(
var sum = 0;
for (x in [1, 2, 3, 4, 5]) {
    sum = sum + x;
}
print sum;
)",
        "15"));
}

// ==================== 编译错误 ====================

TEST_F(VMTest, CompileError)
{
    runAndExpectCompileError("var 123abc;");
}

// ==================== 运行时错误 ====================

TEST_F(VMTest, DivisionByZero)
{
    runAndExpectRuntimeError("print 1 / 0;");
}

TEST_F(VMTest, ModuloByZero)
{
    runAndExpectRuntimeError("print 1 % 0;");
}

// ==================== 综合测试 ====================

TEST_F(VMTest, FizzBuzz)
{
    EXPECT_TRUE(runAndExpect(R"(
for (var i = 1; i <= 15; i = i + 1) {
    if (i % 3 == 0 and i % 5 == 0) {
        print "FizzBuzz";
    } else if (i % 3 == 0) {
        print "Fizz";
    } else if (i % 5 == 0) {
        print "Buzz";
    } else {
        print i;
    }
}
)",
        "FizzBuzz"));
}

TEST_F(VMTest, HigherOrderFunction)
{
    EXPECT_TRUE(runAndExpect(R"(
fun apply(f, x) {
    return f(x);
}
fun double(x) { return x * 2; }
print apply(double, 5);
)",
        "10"));
}

TEST_F(VMTest, MultipleClosures)
{
    EXPECT_TRUE(runAndExpect(R"(
fun make_adder(n) {
    fun adder(x) { return x + n; }
    return adder;
}
var add5 = make_adder(5);
var add10 = make_adder(10);
print add5(3);
print add10(3);
)",
        "13"));
}
