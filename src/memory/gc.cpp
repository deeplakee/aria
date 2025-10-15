#include "memory/gc.h"

#include "compile/functionContext.h"
#include "memory/stringPool.h"
#include "object/objFunction.h"
#include "object/objIterator.h"
#include "object/objList.h"
#include "object/objMap.h"
#include "object/objString.h"
#include "object/objUpvalue.h"
#include "object/object.h"
#include "runtime/vm.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

namespace aria {

GC::GC()
    : bytesAllocated{0}
    , nextGC{GC_INITIAL_SIZE}
    , objList{nullptr}
    , strList{nullptr}
    , tempVars{new ValueStack{}}
    , inGC{false}
    , runningVM{nullptr}
    , compilingContext{nullptr}
{
    conStrPool = new StringPool{this};
    builtinStrs = new ValueArray{this};
    listBuiltins = new ValueHashTable{this};
    mapBuiltins = new ValueHashTable{this};
    stringBuiltins = new ValueHashTable{this};
    iteratorBuiltins = new ValueHashTable{this};
    ObjList::init(this, listBuiltins);
    ObjMap::init(this, mapBuiltins);
    ObjString::init(this, stringBuiltins);
    ObjIterator::init(this, iteratorBuiltins);
#ifdef DEBUG_LOG_GC
    println("=== start up GC ===");
#endif
}

GC::~GC()
{
    delete tempVars;
    delete listBuiltins;
    delete mapBuiltins;
    delete stringBuiltins;
    delete iteratorBuiltins;
    delete builtinStrs;
    delete conStrPool;
    free_objects();
#ifdef DEBUG_LOG_GC
    println("=== shut down GC ===");
#endif
}

void GC::markRoots()
{
    if (runningVM != nullptr) {
        runningVM->stack.mark();

        markValue(runningVM->E_REG);

        for (int i = 0; i < runningVM->CframeCount; i++) {
            runningVM->Cframes[i].function->mark();
        }

        for (int i = 0; i < runningVM->RmoduleCount; i++) {
            markValue(runningVM->Rmodules[i]);
        }

        for (ObjUpvalue *p = runningVM->openUpvalues; p != nullptr; p = p->nextUpvalue) {
            p->mark();
        }

        runningVM->builtIn->mark();
        runningVM->cachedModules->mark();
        if (runningVM->globals != nullptr) {
            runningVM->globals->mark();
        }
    }

    if (compilingContext != nullptr) {
        compilingContext->mark();
    }
    listBuiltins->mark();
    mapBuiltins->mark();
    stringBuiltins->mark();
    iteratorBuiltins->mark();
    tempVars->mark();
}

void GC::traceReferences()
{
    while (!greyStack.empty()) {
        Obj *obj = greyStack.top();
        greyStack.pop();
#ifdef DEBUG_LOG_GC
        println("{:p} blacken {}", toVoidPtr(obj), obj->toString());
#endif
        obj->blacken();
    }
}

void GC::sweep()
{
    Obj *previous = nullptr;
    Obj *object = objList;
    while (object != nullptr) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj *unreached = object;
            object = object->next;
            if (previous != nullptr) {
                previous->next = object;
            } else {
                objList = object;
            }

            free_object(unreached);
        }
    }
}

void GC::collectGarbage()
{
    if (!gcLock.available() || inGC) {
        return;
    }
    inGC = true;

#ifdef DEBUG_LOG_GC
    println("=== gc begin ===");
    size_t before = bytesAllocated;
#endif

    markRoots();

    traceReferences();

    // conStrPool->removeWhite();

    sweep();

    nextGC = bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    println("=== gc end ===");
    println(
        "   collected {} bytes (from {} to {}) next at {}",
        before - bytesAllocated,
        before,
        bytesAllocated,
        nextGC);
#endif

    inGC = false;
}

void GC::free_object(Obj *obj)
{
    auto obj_size = obj->objSize();
#ifdef DEBUG_LOG_GC
    println("{:p} free {} bytes (object {})", toVoidPtr(obj), obj_size, Obj::type2Str(obj->type));
#endif
    bytesAllocated -= obj_size;
    delete obj;
}

void GC::free_objects()
{
    while (objList != nullptr) {
        Obj *next = objList->next;
        free_object(objList);
        objList = next;
    }

    while (strList != nullptr) {
        Obj *next = strList->next;
        free_object(strList);
        strList = next;
    }
}

bool GC::insertStr(ObjString *obj)
{
    cache(obj_val(obj));
    bool res = conStrPool->insert(obj);
    releaseCache();
    return res;
}

ObjString *GC::getStr(const char *chars, size_t length, uint32_t hash)
{
    return conStrPool->findExist(chars, length, hash);
}

} // namespace aria
