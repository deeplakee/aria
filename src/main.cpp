#include "ariaApi.h"
#if ENABLE_READLINE
#include "readline/history.h"
#include "readline/readline.h"
#endif

static void repl()
{
    aria::AriaVM vm;
    for (;;) {
#if ENABLE_READLINE
        char *line_c_str = readline("> ");
        if (!line_c_str) {
            std::cout << std::endl;
            break;
        }
        std::string line(line_c_str);
        if (!line.empty()) {
            add_history(line_c_str);
        }
        free(line_c_str);
#else
        std::string line;
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }
#endif
        vm.interpret(line);
    }
}

static void runFile(const char *path)
{
    std::string source;
    try {
        source = aria::readFile(path);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl << std::endl;
        exit(EXIT_FAILURE);
    }

    aria::AriaVM vm;
    aria::interpretResult result = vm.interpret(path, source);
    if (result != aria::interpretResult::SUCCESS) {
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char *argv[])
{
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << aria::format("Usage: {} [path]\n", aria::AriaProgramName);
        exit(EXIT_FAILURE);
    }
    return 0;
}
