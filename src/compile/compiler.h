#ifndef COMPILER_H
#define COMPILER_H

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

#endif //COMPILER_H
