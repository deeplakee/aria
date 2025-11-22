# 🧭 Aria Debugger

**Aria Debugger** is an interactive debugger (`ariadb`) for the Aria Virtual Machine.
It allows you to set breakpoints, step through code, and inspect execution state during script execution.
The debugger checks for breakpoints before every VM instruction (via the `hookBeforeExec` hook), enabling fine-grained
debugging control.

------

## 🔧 Architecture Overview

```
┌───────────────────────────────────────────────┐
│                   AriaDebugger                │
│                                               │
│  ┌─────────────┐    ┌────────────────────┐   │
│  │  REPL (CLI) │<-->| handleCommand()     │   │
│  └─────────────┘    └────────────────────┘   │
│         ↑                     ↑               │
│         │                     │               │
│         ▼                     ▼               │
│  addPendingBreakpoint()   hookBeforeExec()     │
│         │                     │               │
│         ▼                     ▼               │
│  pendingBreakpoints     activeBreakpoints      │
│                                               │
└───────────────────────────────────────────────┘
             │
             ▼
       ┌────────────┐
       │   AriaVM   │
       │ (Execution)│
       └────────────┘
```

- **AriaDebugger** is attached to an **AriaVM** instance.
- Before each bytecode instruction executes, the VM calls `maybeDebugStep` to determine whether to enter
  `hookBeforeExec`.
- If a breakpoint is hit or single-step mode is active, the debugger enters **REPL mode**.
- Users can interact via command-line commands such as `break`, `step`, or `continue`.

------

## 💬 Command List

At the `(ariadb)` prompt, you can enter the following commands:

| Command                 | Description                                  |
|-------------------------|----------------------------------------------|
| `help`                  | Show available commands                      |
| `break <line>`          | Set a breakpoint in the current file         |
| `break <module>:<line>` | Set a breakpoint in the specified module     |
| `run`                   | Start script execution                       |
| `step` / `s`            | Step to the next line                        |
| `continue` / `c`        | Continue execution until the next breakpoint |
| `quit` / `q`            | Exit the debugger                            |

------

## 🚀 Example Session

```bash
$ ariadb test.aria
debugging file:/home/user/test.aria
Aria Debugger 0.1
Type 'help' for commands.
(ariadb) break 10
Breakpoint set at:
/home/user/test.aria:10
(ariadb) run
[*] Breakpoint hit at line 10
(ariadb) step
[*] Step at line 11
(ariadb) continue
Program finished.
```

------

