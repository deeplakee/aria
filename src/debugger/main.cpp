#include "ariaApi.h"
#include "debugger/debugger.h"

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: ariadb <script.aria>\n";
        return 1;
    }

    aria::AriaVM vm;
    aria::AriaDebugger debugger;
    debugger.attach(&vm);
    debugger.runScript(argv[1]);
    return 0;
}