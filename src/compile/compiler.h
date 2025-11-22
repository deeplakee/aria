#ifndef ARIA_COMPILER_H
#define ARIA_COMPILER_H

#include "common.h"

namespace aria {

class GC;
class ObjFunction;
class ValueHashTable;

class Compiler
{
public:
    // in interpret
    static ObjFunction *compile(
        String sourceLocation, String sources, GC *gc, ValueHashTable *global);

    // in compile and loadModule
    static ObjFunction *compile(
        String moduleLocation,
        String moduleName,
        String source,
        GC *gc,
        ValueHashTable *globals = nullptr);
};

} // namespace aria

#endif //ARIA_COMPILER_H
