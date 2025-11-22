#ifndef ARIA_FUNDEF_H
#define ARIA_FUNDEF_H

#include "aria.h"
#include "value/value.h"

namespace aria {

enum class FunctionType { FUNCTION, METHOD, INIT_METHOD, SCRIPT };

enum class BoundMethodType { FUNCTION, NATIVE_FN };

using NativeFn_t = Value (*)(AriaEnv *env, int argCount, Value *args);

} // namespace aria

#endif //ARIA_FUNDEF_H
