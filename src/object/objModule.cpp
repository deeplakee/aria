#include "object/objModule.h"

#include "chunk/chunk.h"
#include "object/objFunction.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"
#include "value/valueHashTable.h"

namespace aria {

ObjModule::ObjModule(ObjFunction *_module, GC *_gc)
    : Obj{ObjType::MODULE, hashObj(this, ObjType::MODULE), _gc}
    , name{_module->name}
    , module{_module->chunk->globals}
{}

ObjModule::~ObjModule()
{
    delete module;
}

String ObjModule::toString(ValueStack *printStack)
{
    return format("<module {}>", name->C_str_ref());
}

Value ObjModule::getByField(ObjString *name, Value &value)
{
    if (module->get(obj_val(name), value)) {
        return true_val;
    }
    return false_val;
}

void ObjModule::blacken()
{
    module->mark();
    name->mark();
}

ObjModule *newObjModule(ObjFunction *module, GC *gc)
{
    auto obj = gc->allocate_object<ObjModule>(module, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object MODULE)", toVoidPtr(obj), sizeof(ObjModule));
#endif
    return obj;
}

} // namespace aria