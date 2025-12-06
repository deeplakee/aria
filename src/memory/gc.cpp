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

#include <cstring>

namespace aria {

GC::GC()
    : bytesAllocated{0}
    , nextGC{GC_INITIAL_SIZE}
    , objectList{nullptr}
    , internedStringList{nullptr}
    , tempRootStack{new ValueStack{}}
    , stringOpBuffer{new char[GC_BUFFER_SIZE]}
    , inGC{false}
    , runningVM{nullptr}
    , compilingContext{nullptr}
{
    internPool = new StringPool{this};
    listMethods = new ValueHashTable{this};
    mapMethods = new ValueHashTable{this};
    stringMethods = new ValueHashTable{this};
    iteratorMethods = new ValueHashTable{this};
    ObjList::init(this, listMethods);
    ObjMap::init(this, mapMethods);
    ObjString::init(this, stringMethods);
    ObjIterator::init(this, iteratorMethods);
#ifdef DEBUG_LOG_GC
    println("=== start up GC ===");
#endif
}

GC::~GC()
{
    delete[] stringOpBuffer;
    delete tempRootStack;
    delete listMethods;
    delete mapMethods;
    delete stringMethods;
    delete iteratorMethods;
    delete internPool;
    freeAllObjects();
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
    listMethods->mark();
    mapMethods->mark();
    stringMethods->mark();
    iteratorMethods->mark();
    tempRootStack->mark();
    //conStrPool->mark();
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
    Obj *object = objectList;
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
                objectList = object;
            }

            freeObject(unreached);
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

void GC::freeObject(Obj *obj)
{
    auto obj_size = obj->objSize();
#ifdef DEBUG_LOG_GC
    println("{:p} free {} bytes (object {})", toVoidPtr(obj), obj_size, Obj::type2Str(obj->type));
#endif
    bytesAllocated -= obj_size;
    delete obj;
}

void GC::freeAllObjects()
{
    while (objectList != nullptr) {
        Obj *next = objectList->next;
        freeObject(objectList);
        objectList = next;
    }

    while (internedStringList != nullptr) {
        Obj *next = internedStringList->next;
        freeObject(internedStringList);
        internedStringList = next;
    }
}

bool GC::internString(ObjString *obj)
{
    pushTempRoot(NanBox::fromObj(obj));
    bool res = internPool->insert(obj);
    popTempRoot(1);
    return res;
}

ObjString *GC::findInternedString(const char *chars, size_t length, uint32_t hash)
{
    return internPool->findExist(chars, length, hash);
}

} // namespace aria
