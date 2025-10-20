#include "debugger/debugger.h"
#include "chunk/chunk.h"
#include "chunk/disassembler.h"
#include "runtime/vm.h"
#include <ranges>
#include <sstream>

namespace aria {

AriaDebugger::AriaDebugger() {}

void AriaDebugger::attach(AriaVM *v)
{
    vm = v;
    vm->setDebugger(this);
}

void AriaDebugger::runScript(const String &path)
{
    String ariaDir = getProgramDirectory();
    srcPath = getAbsolutePath(getWorkingDirectory(), path);
    println("debugging file:{}", srcPath);
    println("Aria Debugger 0.1");
    println("Type 'help' for commands.");
    repl();

    vm->interpretFromFile(path);
    println("Program finished.");
}

void AriaDebugger::hookBeforeExec(CallFrame *frame, uint32_t offset)
{
    checkAndBindBreakpoint(frame, offset);
    const auto line = frame->function->chunk->lines[offset];
    const auto ins = frame->function->chunk->codes + offset;
    const auto module = frame->function->location->C_str_ref();
    if (activeBreakpoints[module].contains(line) && activeBreakpoints[module][line].ins == ins) {
        println("[*] Breakpoint hit at line {}", line);
        stepping = false;
        repl();
    } else if (stepping) {
        println("[*] Step at line {}", line);
        stepping = false;
        repl();
    }
}

void AriaDebugger::repl()
{
    clearFlags();
    String cmd;
    while (true) {
        print("(ariadb) ");
        if (!std::getline(std::cin, cmd))
            break;
        if (cmd.empty())
            continue;

        handleCommand(cmd);
        if (running || stepping)
            break;
    }
}

void AriaDebugger::handleCommand(const String &cmd)
{
    std::istringstream iss(cmd);
    String token;
    iss >> token;

    if (token == "help") {
        println("Commands:");
        println("  break <module><:><line>   - Set breakpoint");
        println("  run                       - Run script");
        println("  step                      - Step one line");
        println("  continue                  - Continue execution");
        println("  quit                      - Exit debugger");
        return;
    }
    if (token == "break") {
        String arg;
        if (!(iss >> arg)) {
            println("Usage: break <line> or break <module>:<line>");
            return;
        }
        String module;
        uint32_t line;
        try {
            if (size_t colonPos = arg.find(':') != String::npos) {
                module = arg.substr(0, colonPos);
                module = getAbsoluteModulePath(srcPath, module);
                line = std::stoi(arg.substr(colonPos + 1));
            } else {
                module = srcPath;
                line = std::stoi(arg);
            }
            println("bpModule:{}", module);
            println("bpLine:{}", line);
            Breakpoint bp = Breakpoint(module, line);
            addPendingBreakpoint(bp);
        } catch (...) {
            println("Usage: break <line> or break <module>:<line>");
        }
        return;
    }
    if (token == "run") {
        running = true;
        return;
    }
    if (token == "continue" || token == "c") {
        running = true;
        return;
    }
    if (token == "step" || token == "s") {
        stepping = true;
        return;
    }
    if (token == "quit" || token == "q") {
        std::exit(0);
    }
    println("Unknown command: {}", token);
}

void AriaDebugger::addPendingBreakpoint(Breakpoint bp)
{
    pendingBreakpoints[bp.path][bp.line] = bp;
    println("Breakpoint set at:\n{}:{}", bp.path, bp.line);
}

void AriaDebugger::checkAndBindBreakpoint(CallFrame *frame, uint32_t offset)
{
    auto line = frame->function->chunk->lines[offset];
    auto ins = frame->function->chunk->codes + offset;
    auto module = frame->function->location->C_str_ref();

    if (isActiveBreakpoint(module, line)) {
        return;
    }

    auto bpIter = pendingBreakpoints[module].find(line);
    if (bpIter != pendingBreakpoints[module].end()) {
        bindBreakpoint(bpIter->second, ins);
        return;
    }

    auto nextOffset = Disassembler::readInstruction(frame->function->chunk, offset);
    if (nextOffset == 0) {
        return;
    }
    auto nextLine = frame->function->chunk->lines[nextOffset];
    // find breakpoint in empty or comment lines
    auto bpLines = findPendingBreakpoints(module, line, nextLine);

    if (!bpLines.empty()) {
        // just need to set one breakpoint
        bindBreakpoint(Breakpoint{module, line}, ins);
        for (auto bpLine : bpLines) {
            pendingBreakpoints[module].erase(bpLine);
        }
    }
}

bool AriaDebugger::isActiveBreakpoint(const String &module, uint32_t line)
{
    if (activeBreakpoints[module].contains(line)) {
        return true;
    }
    return false;
}

void AriaDebugger::bindBreakpoint(Breakpoint bp, uint8_t *ins)
{
    bp.ins = ins;
    activeBreakpoints[bp.path][bp.line] = bp;
    pendingBreakpoints[bp.path].erase(bp.line);
}

List<uint32_t> AriaDebugger::findPendingBreakpoints(
    const String &module, uint32_t startLine, uint32_t endLine)
{
    List<uint32_t> result;
    if (startLine >= endLine || startLine + 1 == endLine) {
        return result;
    }

    for (const auto &[a, _] : pendingBreakpoints[module]) {
        if (a > startLine && a < endLine) {
            result.push_back(a);
        }
    }

    return result;
}

uint32_t AriaDebugger::findNearestPendingBefore(const String &module, uint32_t line)
{
    auto modIt = pendingBreakpoints.find(module);
    if (modIt == pendingBreakpoints.end())
        return UINT32_MAX;

    uint32_t nearest = UINT32_MAX;
    for (const auto &l : modIt->second | std::views::keys) {
        if (l < line && (nearest == UINT32_MAX || l > nearest))
            nearest = l;
    }
    return nearest;
}

void AriaDebugger::clearFlags()
{
    running = false;
    stepping = false;
}

} // namespace aria