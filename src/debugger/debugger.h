#ifndef ARIA_DEBUGGER_H
#define ARIA_DEBUGGER_H

#include "common.h"
#include "debugger/breakpoint.h"
#include "object/objString.h"

namespace aria {

class AriaVM;
struct CallFrame;
enum class OpCode : uint8_t;

class AriaDebugger
{
public:
    AriaDebugger();

    void attach(AriaVM *vm);

    void runScript(const String &path);

    void hookBeforeExec(CallFrame *frame, uint32_t offset);

private:
    void repl(); // 命令行交互

    void handleCommand(const String &cmd);

    void addPendingBreakpoint(Breakpoint bp);

    void checkAndBindBreakpoint(CallFrame *frame, uint32_t offset);

    bool isActiveBreakpoint(const String &module, uint32_t line);

    void bindBreakpoint(Breakpoint bp, uint8_t *ins);

    // find bp between startLine and endLine in module
    List<uint32_t> findPendingBreakpoints(const String &module, uint32_t startLine, uint32_t endLine);

    uint32_t findNearestPendingBefore(const String &module, uint32_t line);

    void clearFlags();

    static constexpr const char *version = "0.1";

    AriaVM *vm = nullptr;
    String srcPath;
    Map<String, Map<uint32_t, Breakpoint>> activeBreakpoints;
    Map<String, Map<uint32_t, Breakpoint>> pendingBreakpoints;
    bool running = false;
    bool stepping = false;
};

} // namespace aria

#endif //ARIA_DEBUGGER_H
