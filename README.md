# Aria Language Interpreter

**Aria** is a lightweight programming language interpreter written in **C++20**.
It consists of an independent **core runtime library (`aria_core`)**, a **command-line interpreter (`aria`)**, and a comprehensive test suite based on **GoogleTest**.

------

## ⚙️ Build Guide

### ✅ Requirements

| Dependency       | Minimum Version        | Description                                                 |
| ---------------- | ---------------------- | ----------------------------------------------------------- |
| **CMake**        | 3.27                   | Build system                                                |
| **C++ Compiler** | C++20 support required | Tested with g++, clang++, and MSVC                          |
| **GoogleTest**   | v1.14.0                | Automatically downloaded (no manual installation needed)    |
| **Readline**     | Optional               | Enables interactive command-line input (enabled by default) |

------

## 🚀 Quick Build

### 1️⃣ Clone the Repository

```bash
git clone https://github.com/deeplakee/Aria.git
cd Aria
```

### 2️⃣  Generate the Build System

#### 🔹 Release Mode (default)

```bash
cmake -B ./build/release -S . -DCMAKE_BUILD_TYPE=Release
cd ./build/release
```

> Tests (`BUILD_TESTS=OFF`) are disabled by default in Release mode, and several optimization macros are defined automatically.

#### 🔹 Debug Mode (with tests enabled)

```bash
cmake -B ./build/debug -S . -DCMAKE_BUILD_TYPE=Debug
cd ./build/debug
```

> Debug mode automatically enables `BUILD_TESTS=ON` and defines the `DEBUG_MODE` macro.

------

### 3️⃣ Build the Project

```bash
cmake --build . --config Release   # or Debug
```

Generated files:

```
./bin/aria             # Main interpreter executable
./bin/all_tests        # All unit tests
./lib/libaria_core.a   # Static core library
```

------

## 🧰 Optional Build Options

| Option             | Default   | Description                                                 |
| ------------------ | --------- | ----------------------------------------------------------- |
| `BUILD_TESTS`      | `OFF`     | Enable building unit tests (automatically ON in Debug mode) |
| `USE_READLINE`     | `ON`      | Enable interactive command-line input                       |
| `CMAKE_BUILD_TYPE` | `Release` | Choose between `Debug` and `Release` modes                  |

Example:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSE_READLINE=OFF
```

------

## 🧪 Running Tests

### 1️⃣ Run All C++ Unit Tests

```bash
ctest --output-on-failure
```

### 2️⃣ Run Aria Source Code Tests

The `.aria` files under `tests/ariaCode` are real Aria source files used to verify the interpreter’s functionality.

You can run them manually with:

```bash
./bin/aria <project path>/tests/ariaCode/functional/XXX.aria
```

------

## 📜 License

This project is licensed under the [MIT License](./LICENSE).

------

