#include "object/objModule.h"

#include "chunk/chunk.h"
#include "object/objFunction.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"
#include "value/valueHashTable.h"

namespace aria {

ObjModule::ObjModule(ObjFunction *module, GC *gc)
    : Obj{ObjType::MODULE, hash_obj(this, ObjType::MODULE), gc}
    , name_{module->name_}
    , module_{module->chunk_->globals_}
{}

ObjModule::~ObjModule()
{
    delete module_;
}

String ObjModule::to_string()
{
    return format("<module {}>", name_->c_str());
}

Value ObjModule::get_by_field(ObjString *name, Value &value)
{
    if (module_->get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

void ObjModule::blacken()
{
    name_->mark();
    module_->mark();
}

ObjModule *new_ObjModule(ObjFunction *module, GC *gc)
{
    auto obj = gc->allocate_object<ObjModule>(module, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
