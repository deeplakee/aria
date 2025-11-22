------

# 🧩 aria bytecode

------

## 🧱 Group 1 — Basic Load & Store Instructions (1–16)

这些是 VM 的最核心指令，用于加载常量、局部变量、全局变量、字段和下标等。

------

### 1. `LOAD_CONST`

#### Instruction

- **Opcode (8-bit):** `0x01`
- **Operands (16-bit):** constant index

#### Work

Retrieve the constant from the current function’s constant pool at the given index and push it onto the stack.

#### Stack Effect

```
push(constant)
```

------

### 2. `LOAD_NIL`

#### Instruction

- **Opcode (8-bit):** `0x02`
- **Operands:** none

#### Work

Push the special `nil` value onto the stack.

#### Stack Effect

```
push(nil)
```

------

### 3. `LOAD_TRUE`

#### Instruction

- **Opcode (8-bit):** `0x03`
- **Operands:** none

#### Work

Push the boolean value `true` onto the stack.

#### Stack Effect

```
push(true)
```

------

### 4. `LOAD_FALSE`

#### Instruction

- **Opcode (8-bit):** `0x04`
- **Operands:** none

#### Work

Push the boolean value `false` onto the stack.

#### Stack Effect

```
push(false)
```

------

### 5. `LOAD_LOCAL`

#### Instruction

- **Opcode (8-bit):** `0x05`
- **Operands (16-bit):** local index

#### Work

Load a local variable from the current call frame’s stack base plus the given offset, and push its value onto the stack.

#### Stack Effect

```
push(local[offset])
```

------

### 6. `STORE_LOCAL`

#### Instruction

- **Opcode (8-bit):** `0x06`
- **Operands (16-bit):** local index

#### Work

Store the top stack value into the local variable at the given offset of the current call frame (without popping it).

#### Stack Effect

No Effect.

------

### 7. `LOAD_UPVALUE`

#### Instruction

- **Opcode (8-bit):** `0x07`
- **Operands (16-bit):** upvalue index

#### Work

Load a captured variable (upvalue) from the closure’s upvalue array by index and push it onto the stack.

#### Stack Effect

```
push(upvalue)
```

------

### 8. `STORE_UPVALUE`

#### Instruction

- **Opcode (8-bit):** `0x08`
- **Operands (16-bit):** upvalue index

#### Work

Store the top stack value into the closure’s captured upvalue at the given index.

#### Stack Effect

No Effect.

------

### 9. `CLOSE_UPVALUE`

#### Instruction

- **Opcode (8-bit):** `0x09`
- **Operands:** none

#### Work

Close all open upvalues referencing local variables at or above the current stack top (This is usually emitted before
`RETURN` to ensure closures capture correct values).

#### Stack Effect

No Effect.

------

### 10. `DEF_GLOBAL`

#### Instruction

- **Opcode (8-bit):** `0x0A`
- **Operands (16-bit):** name constant index

#### Work

Define a new global variable in the VM’s global table.
The variable name is fetched from the constant pool, and the top of stack is used as the value.

#### Stack Effect

```
pop()
```

------

### 11. `LOAD_GLOBAL`

#### Instruction

- **Opcode (8-bit):** `0x0B`
- **Operands (16-bit):** name constant index

#### Work

Retrieve a global variable by name (from constant pool) and push its value onto the stack.

#### Stack Effect

```
push(global[name])
```

------

### 12. `STORE_GLOBAL`

#### Instruction

- **Opcode (8-bit):** `0x0C`
- **Operands (16-bit):** name constant index

#### Work

Set the value of a global variable using the top value of the stack.

#### Stack Effect

No Effect.

------

### 13. `LOAD_FIELD`

#### Instruction

- **Opcode (8-bit):** `0x0D`
- **Operands (16-bit):** field name constant index

#### Work

Pop an object from the stack, retrieve the field value using the field name (from constant pool),
push the retrieved value back into stack.

#### Stack Effect

```
pop(obj) + push(field_value)
```

------

### 14. `STORE_FIELD`

#### Instruction

- **Opcode (8-bit):** `0x0E`
- **Operands (16-bit):** field name constant index

#### Work

Read `value` from the third-from-top slot, `object` from the second-from-top, and the field name (from constant pool),
sssign `object.field = value`,
pop only the `object`.

#### Stack Effect

```
pop(obj)
```

------

### 15. `LOAD_SUBSCR`

#### Instruction

- **Opcode (8-bit):** `0x0F`
- **Operands:** none

#### Work

Read `index` from the top of the stack and `container` from the second-from-top slot.
Retrieve the element value by evaluating `container[index]`,
then pop both `index` and `container`, and push the retrieved value onto the stack.

#### Stack Effect

```
pop(2) + push(value)
```

------

### 16. `STORE_SUBSCR`

#### Instruction

- **Opcode (8-bit):** `0x10`
- **Operands:** none

#### Work

Read `value` from the third-from-top stack slot, `container` from the second-from-top, and `index` from the top; perform
the assignment `container[index] = value`, then pop the `container` and `index`, leaving `value` on the stack.

#### Stack Effect

```
pop(2)
```

------

## ⚙️ Group 2 — Arithmetic & Logical Instructions (17–29)

这一组是 VM 的“计算核心”，包括比较运算、四则运算和逻辑运算。

------

### 17. `EQUAL`

#### Instruction

- **Opcode (8-bit):** `0x11`
- **Operands:** none

#### Work

Pop two operands `b` and `a` from the stack,
compare them for equality (`a == b`), and push the result (boolean).

#### Stack Effect

```
pop(2) + push(result)
```

------

### 18. `NOT_EQUAL`

#### Instruction

- **Opcode (8-bit):** `0x12`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compare them for inequality (`a != b`),
and push the boolean result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 19. `GREATER`

#### Instruction

- **Opcode (8-bit):** `0x13`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compare whether `a > b`,
and push the boolean result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 20. `GREATER_EQUAL`

#### Instruction

- **Opcode (8-bit):** `0x14`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compare whether `a >= b`,
and push the boolean result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 21. `LESS`

#### Instruction

- **Opcode (8-bit):** `0x15`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compare whether `a < b`,
and push the boolean result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 22. `LESS_EQUAL`

#### Instruction

- **Opcode (8-bit):** `0x16`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compare whether `a <= b`,
and push the boolean result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 23. `ADD`

#### Instruction

- **Opcode (8-bit):** `0x17`
- **Operands:** none

#### Work

Pop two operands `b` and `a` from the stack,
perform addition (`a + b`) if both are numeric;
if either is a string, perform concatenation.

#### Stack Effect

```
pop(2) + push(sum or concatenation)
```

------

### 24. `SUBTRACT`

#### Instruction

- **Opcode (8-bit):** `0x18`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compute numeric subtraction (`a - b`),
and push the result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 25. `MULTIPLY`

#### Instruction

- **Opcode (8-bit):** `0x19`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compute multiplication (`a * b`),
and push the result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 26. `DIVIDE`

#### Instruction

- **Opcode (8-bit):** `0x1A`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
check for division by zero,
then compute division (`a / b`).

#### Stack Effect

```
pop(2) + push(result)
```

------

### 27. `MOD`

#### Instruction

- **Opcode (8-bit):** `0x1B`
- **Operands:** none

#### Work

Pop two operands `b` and `a`,
compute remainder (`a % b`),
and push the result.

#### Stack Effect

```
pop(2) + push(result)
```

------

### 28. `NOT`

#### Instruction

- **Opcode (8-bit):** `0x1C`
- **Operands:** none

#### Work

Pop one operand `a`,
perform boolean negation (`!a`),
and push the result.

#### Stack Effect

```
pop() + push(result)
```

------

### 29. `NEGATE`

#### Instruction

- **Opcode (8-bit):** `0x1D`
- **Operands:** none

#### Work

Pop one operand `a`,
perform numeric negation (`-a`),
and push the result.

#### Stack Effect

```
pop() + push(result)
```

------

## 🧱 Group 3 — Stack & I/O Control Instructions (30–32)

这些指令主要负责管理 **操作数栈** 和 **输出行为**，属于虚拟机运行时的“辅助控制指令”。

------

### 30. `POP`

#### Instruction

- **Opcode (8-bit):** `0x1E`
- **Operands:** none

#### Work

Remove the top value from the stack and discard it.

#### Stack Effect

```
pop(1)
```

------

### 31. `POP_N`

#### Instruction

- **Opcode (8-bit):** `0x1F`
- **Operands:** `count (8-bit)`

#### Work

Pop and discard **N** values from the stack.

#### Stack Effect

```
pop(N)
```

------

### 32. `PRINT`

#### Instruction

- **Opcode (8-bit):** `0x20`
- **Operands:** none

#### Work

Pop the top value from the stack, convert it to a string,
and print it to standard output.

#### Stack Effect

```
pop(1)
```

------

### 33. `NOP`

#### Instruction

- **Opcode (8-bit):** `0x21`
- **Operands:** none

#### Work

Do nothing. Instruction placeholder for alignment or patching.

#### Stack Effect

```
none
```

------

## 🔀 Group 4 — Branch & Jump Control (34–39)

这些指令控制程序的执行流，包括无条件与条件跳转，

------

### 34. `JUMP_FWD`

#### Instruction

- **Opcode (8-bit):** `0x22`
- **Operands (16-bit):** `offset`

#### Work

Move the instruction pointer **forward** by `offset` bytes.

#### Stack Effect

```
none
```

------

### 35. `JUMP_BWD`

#### Instruction

- **Opcode (8-bit):** `0x23`
- **Operands (16-bit):** `offset`

#### Work

Move the instruction pointer **backward** by `offset` bytes.

#### Stack Effect

```
none
```

------

### 36. `JUMP_TRUE`

#### Instruction

- **Opcode (8-bit):** `0x24`
- **Operands (16-bit):** `offset`

#### Work

Pop the top value `cond`.
If `isTruthy(cond)` is true, jump **backward** by `offset`.

#### Stack Effect

```
pop(1)
```

------

### 37. `JUMP_TRUE_NOPOP`

#### Instruction

- **Opcode (8-bit):** `0x25`
- **Operands (16-bit):** `offset`

#### Work

Check top value `cond` (without popping).
If `isTruthy(cond)` is true, jump **backward** by `offset`.

#### Stack Effect

No Effect.

------

### 38. `JUMP_FALSE`

#### Instruction

- **Opcode (8-bit):** `0x26`
- **Operands (16-bit):** `offset`

#### Work

Pop the top value `cond`.
If `isFalsy(cond)` is true, jump **backward** by `offset`.

#### Stack Effect

```
pop(1)
```

------

### 39. `JUMP_FALSE_NOPOP`

#### Instruction

- **Opcode (8-bit):** `0x27`
- **Operands (16-bit):** `offset`

#### Work

Check top value `cond` (without popping).
If `isFalsy(cond)` is true, jump **backward** by `offset`.

#### Stack Effect

No Effect.



------

## 📞 Group 5 — Function Call & Closure Creation (40–41)

这组指令是虚拟机的核心之一，控制函数调用栈帧、闭包捕获等运行时机制。
它们直接对应编译器生成的函数调用和 lambda（闭包）表达式。

------

### 40. `CALL`

#### Instruction

- **Opcode (8-bit):** `0x28`
- **Operands (8-bit):** `argCount`

#### Work

Initiate a function or callable invocation.
The callee is located below the top `argCount` arguments on the stack.
If the callee is a user-defined function, a new call frame is created and execution continues within that function’s
bytecode.
If the callee is a native function or callable object, it executes directly and may later adjust the stack and push a
return value.

#### Stack Effect

Temporarily extends the active frame for the call.
Actual stack cleanup (removing the callee and arguments, and pushing the return value) occurs after the call completes.

------

### 41. `CLOSURE`

#### Instruction

- **Opcode (8-bit):** `0x29`
- **Operands:** `constIndex (16-bit)` + upvalue info (variable length)

#### Work

Read a function object from the constant pool.
For each upvalue entry emitted by the compiler, read `isLocal` and `index` and resolve the upvalue as follows:

- If `isLocal` is true, capture the local variable at `frame->stakBase + index` by calling `captureUpvalue(...)`.
- Otherwise, reference the parent function’s upvalue at `frame->function->upvalues[index]`.

Wire the resolved upvalues into the function/closure upvalue slots so the function can access captured variables, then
push the resulting closure/function value onto the stack.

#### Stack Effect

No Effect.

#### Notes

------

## 🏗️ Group 6 — Class & Method Definition (42–47)

这些指令控制 **类对象**、**继承关系** 与 **方法绑定** 的创建过程，
是 Aria VM 实现面向对象特性的核心。

------

### 42. `MAKE_CLASS`

#### Instruction

- **Opcode (8-bit):** `0x2A`
- **Operands (16-bit):** `constIndex` — class name in constant pool

#### Work

Create a new **class object** with the given name.
The class initially has no superclass or methods.

#### Stack Effect

```
push(classObj)
```

------

### 43. `INHERIT`

#### Instruction

- **Opcode (8-bit):** `0x2B`
- **Operands:** none

#### Work

Pop the superclass object from the stack.
Copy methods and metadata from the popped `superclass` into the class object immediately below it on the stack (
`class`).

#### Stack Effect

```
pop(superclass)
```

------

### 44. `MAKE_METHOD`

#### Instruction

- **Opcode (8-bit):** `0x2C`
- **Operands (16-bit):** `constIndex` — method name

#### Work

Pop the method object from the top of the stack.
Bind this method to the class object immediately below it on the stack (`class`).

#### Stack Effect

```
pop(method)
```

------

### 45. `MAKE_INIT_METHOD`

#### Instruction

- **Opcode (8-bit):** `0x2D`
- **Operands:** none

#### Work

Pop the initializer method (constructor) from the stack.
Bind it as the `init` method of the class object immediately below it on the stack (`class`).

#### Stack Effect

```
pop(initmethod)
```

------

### 46. `INVOKE_METHOD`

#### Instruction

- **Opcode (8-bit):** `0x2E`
- **Operands:**

#### Work

#### Stack Effect

```

```

#### Notes

相当于执行：

```aria
object.method(args...)
```

------

### 47. `LOAD_SUPER_METHOD`

#### Instruction

- **Opcode (8-bit):** `0x2F`
- **Operands (16-bit):** `constIndex` — method name

#### Work

Load a method with the given name from the superclass of the current class.
Pop the instance object, then push the resolved method.

#### Stack Effect

```
pop(instance) + push(method)
```

------

## 🏗️ Group 7 — Collection Construction (48–49)

这些指令用于创建 **列表（list）** 和 **字典（map）**，
是 Aria VM 支持集合类型的核心操作。

------

### 48. `MAKE_LIST`

#### Instruction

- **Opcode (8-bit):** `0x30`
- **Operands (16-bit):** `n` — number of elements

#### Work

Pop `n` values from the stack (in order),
create a **list object** containing these elements,
then push the list back onto the stack.

#### Stack Effect

```
pop(n) → push(list)
```

------

### 49. `MAKE_MAP`

#### Instruction

- **Opcode (8-bit):** `0x31`
- **Operands (16-bit):** `n` — number of key-value pairs

#### Work

Pop `2*n` values from the stack, treating them as consecutive key-value pairs.
The order on the stack is:

```
..., key1, value1, key2, value2, ..., keyn, valuen
```

Construct a **dictionary (map) object** using these pairs, then push the resulting dictionary back onto the stack.

#### Stack Effect

```
pop(2*n) → push(map)
```

------

## 📚 Group 8 — Module & Iterator Operations (50–53)

这些指令负责 **模块加载（import）** 与 **迭代协议（for / in 循环）** 的支持。

------

### 50. `IMPORT`

#### Instruction

- **Opcode (8-bit):** `0x32`
- **Operands (16-bit):** `constIndex` — module name constant

#### Work

Load a module by its name string from the constant pool.
If the module is not yet loaded, the VM compiles and executes it,
then pushes the module object onto the stack.

#### Stack Effect

```
push(module)
```

------

### 51. `GET_ITER`

#### Instruction

- **Opcode (8-bit):** `0x33`
- **Operands:** none

#### Work

Pop a value from the stack, and attempt to get its **iterator object**.
Push the resulting iterator back onto the stack.

#### Stack Effect

```
pop(1) → push(iter)
```

------

### 52. `ITER_HAS_NEXT`

#### Instruction

- **Opcode (8-bit):** `0x34`
- **Operands:** none

#### Work

Pop the iterator object,
check whether there is a next element available.
Push a boolean (`true` / `false`) indicating the result.

#### Stack Effect

```
pop(1) → push(bool)
```

------

### 53. `ITER_GET_NEXT`

#### Instruction

- **Opcode (8-bit):** `0x35`
- **Operands:** none

#### Work

Pop the iterator object,
retrieve the **next value** from it,
and push both the iterator (for reuse) and the value.

#### Stack Effect

```
pop(1) → push(iter, next_val)
```

------

## ⚙️ Group 9 — Exception Handling & Control Flow Termination (54–57)

这些指令负责 **异常框架（Eframe）** 管理、**异常传播** 以及 **函数返回**。
它们是虚拟机实现 `try / catch / throw` 的核心部分。

------

### 54. `SETUP_EXCEPT`

#### Instruction

- **Opcode (8-bit):** `0x36`
- **Operands (16-bit):** `offset` — jump offset to exception handler

#### Work

Create a new **exception frame (Eframe)**, recording:

- The current instruction pointer
- The stack base
- The handler location (`ip + offset`)

When an exception occurs, control will jump to that handler.

#### Stack Effect

No Effect.

------

### 55. `END_EXCEPT`

#### Instruction

- **Opcode (8-bit):** `0x37`
- **Operands:** none

#### Work

Exit the current exception frame.
Pop the topmost `Eframe` from the exception stack.

#### Stack Effect

No Effect.

------

### 56. `THROW`

#### Instruction

- **Opcode (8-bit):** `0x38`
- **Operands:** none

#### Work

Pop the top value (the exception object or message) from the stack,
mark the VM as in `THROWING` state,
and unwind stack frames until a matching exception frame is found.
Then jump to the exception handler recorded in that frame.

#### Stack Effect

```
pop(1)
```

------

### 57. `RETURN`

#### Instruction

- **Opcode (8-bit):** `0x39`
- **Operands:** none

#### Work

Pop the top value as the **return result**.
Close any active upvalues in the current frame.
Pop the current **CallFrame**.
Push the return result onto the caller’s stack.
If this is the top-level frame (script or function object), terminate execution and return the result.

#### Stack Effect

```
pop(1) → (restore caller frame) → push(result)/return result
```

------



