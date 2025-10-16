好——我把你最初给出的 57 条指令 **一一补全并严格对齐到你现有 AriaVM 实现**（包含 `Cframes`/`Eframes`/`Rmodules`、`callFunction`、`callModule`、`returnFromCurrentFrame`、`packVarargs`、`captureUpvalue`、`closeUpvalues`、`unwindToCatchPoint`、`E_REG`、异常处理等细节）。表格每行都说明了：

- 显式操作数（字节码读取）
- 隐式操作数（从栈读取）
- 精确的栈变化
- 是否改变 CallFrame / ExceptionFrame / RunningModule（以及如何改变）
- 在 VM 层面的具体行为（与现有函数如何交互）

> 我尽量保持简洁但精确 — 如果你想我也可以把每条指令的**伪码实现**（`switch` 分支内的近乎逐行实现）生成出来。现在先把完整表贴出。

------











## AriaVM 指令（与实现精确对齐）

| 编号 | 指令                | 操作码（8-bit ） | 操作数                                                       | 隐式操作数（从栈读取）                 | 栈变化                                                       | CFrame / EFrame / Rmodule 变化                               | VM 行为（实现级别）                                          |
| ---- | ------------------- | ---------------- | ------------------------------------------------------------ | -------------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 1    | `LOAD_CONST`        | 0x01             | 16-bit const index                                           | —                                      | `push(const)`                                                | —                                                            | `read_constant(frame)` 并 `stack.push(...)`。                |
| 2    | `LOAD_NIL`          | 0x02             | —                                                            | —                                      | `push(nil)`                                                  | —                                                            | `stack.push(nil_val)`。                                      |
| 3    | `LOAD_TRUE`         | 0x03             | —                                                            | —                                      | `push(true)`                                                 | —                                                            | `stack.push(bool_val(true))`。                               |
| 4    | `LOAD_FALSE`        | 0x04             | —                                                            | —                                      | `push(false)`                                                | —                                                            | `stack.push(bool_val(false))`。                              |
| 5    | `LOAD_LOCAL`        | 0x05             | 16-bit offset                                                | —                                      | `push(frame->stakBase[offset])`                              | —                                                            | 从 `frame->stakBase + offset` 读取并 `stack.push`。          |
| 6    | `STORE_LOCAL`       | 0x06             | 16-bit offset                                                | stack top                              | — (`frame->stakBase[offset] = stack.peek()`)                 | —                                                            | 把 `stack.peek()` 写入 `frame->stakBase[offset]`（不弹栈）。 |
| 7    | `LOAD_UPVALUE`      | 0x07             | 16-bit slot                                                  | —                                      | `push(*(upvalue->location))`                                 | —                                                            | 从 `frame->function->upvalues[slot]->location` 读取并 `stack.push`。 |
| 8    | `STORE_UPVALUE`     | 0x08             | 16-bit slot                                                  | stack top                              | — (`*upvalue->location = stack.peek()`)                      | —                                                            | 写入闭包上值（不弹栈）。                                     |
| 9    | `CLOSE_UPVALUE`     | 0x09             | —                                                            | —                                      | （无栈变化）                                                 | —                                                            | `closeUpvalues(stack.getTopPtr()-1)`，并 `stack.pop()`（函数中为捕获变量弹出）。 |
| 10   | `DEF_GLOBAL`        | 0x0A             | 16-bit string const                                          | stack top                              | `pop()`                                                      | —                                                            | 将栈顶值定义到当前 chunk 的 globals（`chunk->globals->insert(name, value)`）；若已存在，`THROW_EXCEPTION RUNTIME_EXISTED_VARIABLE`（通过 `newException`/`throwException`）。 |
| 11   | `LOAD_GLOBAL`       | 0x0B             | 16-bit string const                                          | —                                      | `push(value)`                                                | —                                                            | 先在 `chunk->globals` 查找，再查 `builtIn`。找不到则 `THROW_EXCEPTION RUNTIME_UNDEFINED_VARIABLE`。 |
| 12   | `STORE_GLOBAL`      | 0x0C             | 16-bit string const                                          | stack top                              | —                                                            | —                                                            | 若 `chunk->globals->insert(name, stack.peek())` 返回 true（表示未定义），抛 `RUNTIME_UNDEFINED_VARIABLE`。（实现上以 insert/remove 语义判定） |
| 13   | `LOAD_FIELD`        | 0x0D             | 16-bit string const                                          | stack top (obj)                        | `setTopVal(field_value)`                                     | —                                                            | 检查 `is_obj(stack.peek())`，调用 `obj->getByField(name, value)`。若不存在或返回 false，抛 `RUNTIME_INVALID_FIELD_OP`。若取到函数/方法，调用 `bindMethodIfNeeded` 将其封成 bound method。 |
| 14   | `STORE_FIELD`       | 0x0E             | 16-bit string const                                          | stack top: val, next: obj              | `pop(1)`（把对象原来弹出或替换）                             | —                                                            | `obj->setByField(propertyName, value)`，若返回 false 抛 `RUNTIME_INVALID_FIELD_OP`。 |
| 15   | `LOAD_SUBSCR`       | 0x0F             | —                                                            | stack top: index, next: obj            | `pop(2)` then `push(value)`                                  | —                                                            | `obj->getByIndex(index, value)`；如果返回 false 抛 `RUNTIME_INVALID_INDEX_OP`；如果返回是异常（特殊 Value），`CHECK_EXCEPTION` 会触发 `throwException`。 |
| 16   | `STORE_SUBSCR`      | 0x10             | —                                                            | stack top: index, next: obj, next: val | `pop(2)`                                                     | —                                                            | `obj->setByIndex(index, val)`；失败抛 `RUNTIME_INVALID_INDEX_OP`；若返回异常，`CHECK_EXCEPTION` 触发。 |
| 17   | `EQUAL`             | 0x11             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(bool)`                                   | —                                                            | 使用 `valuesSame(a,b)` 做相等比较，`stack.push(bool_val(...))`。 |
| 18   | `NOT_EQUAL`         | 0x12             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(bool)`                                   | —                                                            | `push(bool_val(!valuesSame(a,b)))`。                         |
| 19   | `GREATER`           | 0x13             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(bool)`                                   | —                                                            | 检查两操作数为 number，否则 `RUNTIME_TYPE_ERROR`。比较并 `push(bool)`。 |
| 20   | `GREATER_EQUAL`     | 0x14             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(bool)`                                   | —                                                            | 同上，`a >= b`。                                             |
| 21   | `LESS`              | 0x15             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(bool)`                                   | —                                                            | `a < b`（类型检查同上）。                                    |
| 22   | `LESS_EQUAL`        | 0x16             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(bool)`                                   | —                                                            | `a <= b`。                                                   |
| 23   | `ADD`               | 0x17             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(result)`                                 | —                                                            | 支持 number+number 或 string+string。字符串拼接用 `concatenateString`（可能返回 null -> fatalError RESOURCE_STRING_OVERFLOW）。否则抛 `RUNTIME_TYPE_ERROR`。 |
| 24   | `SUBTRACT`          | 0x18             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(number)`                                 | —                                                            | 数值减法，类型检查，否则 `RUNTIME_TYPE_ERROR`。              |
| 25   | `MULTIPLY`          | 0x19             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(number)`                                 | —                                                            | 数值乘法，类型检查。                                         |
| 26   | `DIVIDE`            | 0x1A             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(number)`                                 | —                                                            | 检查除数是否为零（`isZero(b)`），若零则 `RUNTIME_DIVISION_BY_ZERO`。 |
| 27   | `MOD`               | 0x1B             | —                                                            | stack top: b, next: a                  | `pop(2)` then `push(number)`                                 | —                                                            | 使用 `std::fmod(a,b)`，若 b == 0 抛 `RUNTIME_MODULO_BY_ZERO`。 |
| 28   | `NOT`               | 0x1C             | —                                                            | stack top: a                           | `pop()` then `push(bool)`                                    | —                                                            | `stack.push(bool_val(isFalsey(a)))`。                        |
| 29   | `NEGATE`            | 0x1D             | —                                                            | stack top: a                           | `pop()` then `push(number)`                                  | —                                                            | 若非 number 抛 `RUNTIME_TYPE_ERROR`，否则 `push(-a)`。       |
| 30   | `POP`               | 0x1E             | —                                                            | —                                      | `pop()`                                                      | —                                                            | `stack.pop()`。                                              |
| 31   | `POP_N`             | 0x1F             | 8-bit n                                                      | —                                      | `pop(n)`                                                     | —                                                            | `stack.pop_n(n)`（8位操作数）。                              |
| 32   | `PRINT`             | 0x20             | —                                                            | stack top                              | `pop()`                                                      | —                                                            | 将 `valueString(stack.pop())` 输出到 `stdout`（用 `std::cout`），等价于内建 `println` 的同步行为（注意：若你想 `println` 调用脚本 `str()`，需要 `runFunction` 支持局部执行/VMState）。 |
| 33   | `NOP`               | 0x21             | —                                                            | —                                      | —                                                            | —                                                            | 空操作。                                                     |
| 34   | `JUMP_FWD`          | 0x22             | 16-bit offset                                                | —                                      | —                                                            | —                                                            | `frame->ip -= offset`（你定义的“向前跳转”是 ip 减）。用于跳过字节码段。 |
| 35   | `JUMP_BWD`          | 0x23             | 16-bit offset                                                | —                                      | —                                                            | —                                                            | `frame->ip += offset`（向后跳回）。                          |
| 36   | `JUMP_TRUE`         | 0x24             | 16-bit offset                                                | stack top cond                         | `pop()`                                                      | —                                                            | 如果 `!isFalsey(cond)` 则 `frame->ip += offset`。            |
| 37   | `JUMP_TRUE_NOPOP`   | 0x25             | 16-bit offset                                                | stack top cond                         | —                                                            | —                                                            | 条件为真则跳转但保留栈顶（`if (!isFalsey(peek)) frame->ip+=offset`)。 |
| 38   | `JUMP_FALSE`        | 0x26             | 16-bit offset                                                | stack top cond                         | `pop()`                                                      | —                                                            | 如果 `isFalsey(cond)` 则跳转。                               |
| 39   | `JUMP_FALSE_NOPOP`  | 0x27             | 16-bit offset                                                | stack top cond                         | —                                                            | —                                                            | 条件假则跳转且不弹栈。                                       |
| 40   | `CALL`              | 0x28             | 8-bit argCount                                               | n args + callee (callee at depth n)    | *语义上被 callee consume*：根据 callee 类型会改变栈结构      | ✅ 创建 CallFrame（`callValue` → `callFunction`/`callNativeFn`/`callNewInstance`/`callBoundMethod`） | 读取 argCount，`callee = stack.peek(argCount)`；调用 `callValue(callee,argCount)`：- 若是 `FUNCTION`：`callFunction()` 创建新 frame（`pushCallFrame`），并 `run()` 会继续执行新 frame（在你实现中通常是 `run(retFrame)` 或同一 loop）；- 若是 `NATIVE_FN`：调用 `callNativeFn`（会直接执行 native, packVarargs 如需），native 返回 result，stack 调整（`stack.pop_n(arity+1); stack.push(result)`）；- 若是 `CLASS`：`callNewInstance` 替换 callee slot 为新 instance 并可能调用 `initMethod`（会再 `callFunction`）；- 若是 `BOUND_METHOD`：`callBoundMethod` 会把 receiver 替入 callee slot，然后根据 methodType 调用 function/native。任何 `callXXX` 返回异常 `Value` 都会被 `CHECK_EXCEPTION` 检测并在 switch 外处理（即 `throwException`）。 |
| 41   | `CLOSURE`           | 0x29             | 16-bit constant index + for each upvalue: (8-bit isLocal + 16-bit index) | —                                      | `push(ObjClosure)`                                           | —                                                            | `fun = as_ObjFunction(read_constant(frame))`；为每个 upvalue 如果 isLocal 则 `captureUpvalue(frame->stakBase + index)`，否则 `fun->upvalues[i] = frame->function->upvalues[index]`。最后 `stack.push(obj_val(new ObjClosure(...)))`（你的实现直接修改 `fun->upvalues` 然后 push closure）。 |
| 42   | `MAKE_CLASS`        | 0x2A             | 16-bit name const                                            | —                                      | `push(ObjClass)`                                             | —                                                            | `newObjClass(name, gc)` 并 `stack.push(obj_val(klass))`。    |
| 43   | `INHERIT`           | 0x2B             | —                                                            | stack top: super, next: class          | `pop()`                                                      | —                                                            | 检查 `is_ObjClass(stack.peek())`，将 `klass->superKlass = superKlass` 并 `klass->methods.copy(&superKlass->methods)`。 |
| 44   | `MAKE_METHOD`       | 0x2C             | 16-bit name const                                            | stack top: method, next: klass         | `pop()`                                                      | —                                                            | 把方法插入 `klass->methods`：`klass->methods.insert(methodName, method)`。 |
| 45   | `MAKE_INIT_METHOD`  | 0x2D             | —                                                            | stack top: method, next: klass         | `pop()`                                                      | —                                                            | `klass->initMethod = as_ObjFunction(method)`。               |
| 46   | `INVOKE_METHOD`     | 0x2E             | 8-bit argCount                                               | `receiver, args...`                    | 视调用情况（同 CALL）                                        | ✅ 可能创建 CallFrame                                         | 未实现分支。                                                 |
| 47   | `LOAD_SUPER_METHOD` | 0x2F             | 16-bit name const                                            | stack top instance                     | `pop()` then `push(boundSuperMethod)`                        | —                                                            | 取 `superKlass = instance->klass->superKlass`，在 `superKlass->methods` 找方法；若找不到但 `init` 存在并名为 "init" 则处理；把方法包装成 `ObjBoundMethod`（receiver = instance）并 `stack.setTopVal(superMethod)`。 |
| 48   | `MAKE_LIST`         | 0x30             | 16-bit list size n                                           | n elems on stack                       | `pop(n)` then `push(ObjList)`                                | —                                                            | `newObjList(stack.getTopPtr() - n, n, gc)`；`stack.pop_n(n)`；`stack.push(obj_val(list))`。 |
| 49   | `MAKE_MAP`          | 0x31             | 16-bit map size n                                            | 2n elems (k,v)                         | `pop(2n)` then `push(ObjMap)`                                | —                                                            | `newObjMap(stack.getTopPtr() - 2n, n, gc)`；`stack.pop_n(2n)`；`stack.push(obj_val(map))`。 |
| 50   | `IMPORT`            | 0x32             | 16-bit module name const                                     | —                                      | `push(module)`                                               | ✅ 可能创建 Rmodule & CFrame                                  | 读取 `moduleName`，计算 `absoluteModulePath = resolveRelativePath(getRunningModule_Cstr(), moduleName)`；若 invalid 抛 `RUNTIME_MODULE_INIT_ERROR`；检查 `isModuleRunning(absoluteModulePath)` 防止循环导入 -> 抛 `RUNTIME_MODULE_INIT_ERROR`；若 `cachedModules` 存在直接 `push(obj_val(module))`；否则 `moduleFn = loadModule(path, moduleName)` 并 `callFunction(moduleFn, 0)`（注意：你的实现是 `stack.push(obj_val(moduleFn)); callModule(moduleFn)` —— 会 `pushRunningModule` 并 `callFunction`）。 |
| 51   | `GET_ITER`          | 0x33             | —                                                            | stack top obj                          | `pop()` then `push(iter)`                                    | —                                                            | 检查 `is_obj(obj)`；`iter = as_obj(obj)->createIter(gc)`；若 `is_nil(iter)` 抛 `RUNTIME_TYPE_ERROR`；`stack.setTopVal(iter)`。 |
| 52   | `ITER_HAS_NEXT`     | 0x34             | —                                                            | stack top iter                         | `setTopVal(bool)`                                            | —                                                            | 必须是 `ObjIterator`，然后 `push(bool_val(iterator->iter->hasNext()))`（在实现里是 `setTopVal`）。 |
| 53   | `ITER_GET_NEXT`     | 0x35             | —                                                            | stack top iter                         | `pop()` then `push(nextVal)`                                 | —                                                            | `nextVal = iterator->iter->next()`；弹出迭代器，压入 next 值。 |
| 54   | `SETUP_EXCEPT`      | 0x36             | 16-bit offset                                                | —                                      | —                                                            | ✅ push ExceptionFrame                                        | 计算 `newIp = frame->ip + offset`；`pushExceptionFrame(CframeCount, RmoduleCount, newIp, stack.size())`；如果 `EframeCount==FRAME_SIZE` 报 `RUNTIME_STACK_OVERFLOW`。 |
| 55   | `END_EXCEPT`        | 0x37             | —                                                            | —                                      | —                                                            | ✅ pop ExceptionFrame                                         | `popExceptionFrame()`（减少 `EframeCount`）。                |
| 56   | `THROW`             | 0x38             | —                                                            | stack top err                          | `pop()` then (stack.push(E_REG) 在 unwind)                   | — (但会触发 unwindToCatchPoint)                              | `set_err_flag(); E_REG = stack.pop(); if (EframeCount==0) reportRuntimeFatalError(RUNTIME_UNCAUGHT_EXCEPTION, valueString(E_REG)) ; unwindToCatchPoint()`。`unwindToCatchPoint()`：恢复 `CframeCount = ef->CFrameCount; updateCFrame(); RmoduleCount = ef->RmoduleCount; frame->ip = ef->ip; stack.resize(ef->stackSize); stack.push(E_REG); E_REG=nil; unset_err_flag();` |
| 57   | `RETURN`            | 0x39             | —                                                            | stack top result                       | `pop()` then restore caller stack and `push(result)` if not retFrame | ✅ pop CallFrame (可能影响 Rmodule)                           | 从当前 frame 读 `result = stack.pop()`；`stack.resize(frame->stakBase - stack.base())`（回收当前 frame 的栈空间）；`result = returnFromCurrentFrame(result)`（该函数：`closeUpvalues(frame->stakBase); if (frame->function->type==SCRIPT) result = returnFromCurrentModule(result); popCallFrame();`）；若 `CframeCount == retFrame` 则 `run()` 返回 `result`（局部执行模式）；否则 `stack.push(result)` 到上层帧。 |

------



















## 额外说明（与你的实现对应的细节与注意点）

### CALL / callFunction / callNativeFn / callBoundMethod

- `CALL` 读取 `argCount`，并以 `callee = stack.peek(argCount)` 进行类型分发（`callValue`）。
- `callFunction`：检查参数个数或打包 varargs（`packVarargs`），随后 `createCallFrame`（即 `pushCallFrame(function, codes, stackTop - arity)`）。**不直接执行**，而是由 `run(retFrame)` 驱动到新 frame 返回。
- `callNativeFn`：native 期望其 args pointer 指向第一个参数（而非包含 callee），实现中 `arity = native->acceptsVarargs ? native->arity + 1 : native->arity; Value result = native->function(this, argCount, stack.getTopPtr() - arity); stack.resize(stack.size() - arity - 1); stack.push(result);` —— 注意这里 native 可以同步调用脚本，但要使用 `runFunction` / `VMState` 协作（你的实现使用 `run(CframeCount - 1)` 局部执行模式以避免 run() 重入）。
- `callBoundMethod`：如果 bound 是 function，先把 receiver 放到 call slot 再 `callFunction`；如果是 native 则把 receiver 放到 args 然后 `callNativeFn`。

### 模块导入（IMPORT）

- 你用了 `runningModules`（`Rmodules`）来追踪模块导入路径，防止循环导入。`callModule` 会 `pushRunningModule` 并 `callFunction(moduleFn,0)`。
- 成功执行后脚本顶层 `RETURN` 时会触发 `returnFromCurrentModule` 来把模块封装成 `ObjModule` 并缓存到 `cachedModules`。

### 异常处理（SETUP_EXCEPT / THROW / END_EXCEPT）

- `SETUP_EXCEPT` push 一个 `ExceptionFrame`：保存 `CframeCount/RmoduleCount/newIp/stackSize`。
- `THROW`：把异常对象放到 `E_REG`，若没有 `Eframes`（即 `EframeCount==0`）则 `reportRuntimeFatalError`（会打印栈回溯并抛 `ariaRuntimeException`）。否则 `unwindToCatchPoint` 恢复到对应的 `Eframe` 状态并把异常放回栈供 catch 处理。
- `END_EXCEPT`：`popExceptionFrame()`。

### 闭包与上值

- `CLOSURE` 在创建闭包时会为每个 upvalue 调用 `captureUpvalue`（当 `isLocal`）或引用父函数的 upvalue。
- `captureUpvalue` 将 `openUpvalues` 链按地址从高到低排序插入，避免重复创建。
- `CLOSE_UPVALUE` 在函数返回或明确请求时把 `openUpvalues` 中 `location >= last` 的 upvalue 关闭（把值拷贝到 `upvalue->closed` 并把 location 指向 closed）。

### 迭代器

- `GET_ITER` 通过 `as_obj(obj)->createIter(gc)` 得到迭代器对象（非 nil），否则抛 `RUNTIME_TYPE_ERROR`。
- `ITER_HAS_NEXT` 与 `ITER_GET_NEXT` 都在 `ObjIterator` 上调用其内置迭代器对象的 `hasNext()` / `next()`。

### 局部 run / run(retFrame)

- 你的 `run(int retFrame)` 支持“局部执行”模式：当 `RETURN` 后 `CframeCount == retFrame` 时，`run` 返回该返回值。
- 因此 `runFunction` / `callModule` 等可以安全地在原生函数内调用脚本函数而**不重入外层 run()**（只要它们把正确的 `retFrame` 传入）。

### `CHECK_EXCEPTION` 宏

- 在某些 object 操作（比如 `getByIndex` / `setByIndex`）返回 `Value` 时可能该 Value 为异常（`vm->newException(...)` 之类），`CHECK_EXCEPTION(result)` 会检测 `get_err_flag()` 并调用 `throwException`（将 E_REG 设置并 unwind）。

------

## 我可以继续帮你做的事情（任选其一）

1. 为表中每条指令生成**直接可贴入 `run()` 的伪码实现**（把你的 switch 分支替换成更详尽的实现，包含所有错误分支与 `CHECK_EXCEPTION`）。
2. 给 `INVOKE_METHOD` 写一个完整实现范例（你标注未实现）。
3. 为 `CALL` 场景写出**原生函数内调用脚本函数**的示范（`runFunction` 的正确写法 + VMState 示例）。
4. 为 `YIELD/RESUME` 增加到表中并写出实现示例（协程支持）。

你想先做哪一项？