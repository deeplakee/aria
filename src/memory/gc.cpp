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
    : bytes_allocated_{0}
    , next_gc_{k_gc_initial_size}
    , object_list_{nullptr}
    , interned_string_list_{nullptr}
    , temp_root_stack_{new ValueStack{}}
    , string_op_buffer_{new char[k_gc_buffer_size]}
    , in_gc_{false}
    , running_vm_{nullptr}
    , compiling_context_{nullptr}
{
    intern_pool_ = new StringPool{this};
    list_methods_ = new ValueHashTable{this};
    map_methods_ = new ValueHashTable{this};
    string_methods_ = new ValueHashTable{this};
    iterator_methods_ = new ValueHashTable{this};
    ObjList::init(this, list_methods_);
    ObjMap::init(this, map_methods_);
    ObjString::init(this, string_methods_);
    ObjIterator::init(this, iterator_methods_);
#ifdef DEBUG_LOG_GC
    println("=== start up GC ===");
#endif
}

GC::~GC()
{
    delete[] string_op_buffer_;
    delete temp_root_stack_;
    delete list_methods_;
    delete map_methods_;
    delete string_methods_;
    delete iterator_methods_;
    delete intern_pool_;
    free_all_objects();
#ifdef DEBUG_LOG_GC
    println("=== shut down GC ===");
#endif
}

void GC::mark_roots()
{
    if (running_vm_ != nullptr) {
        running_vm_->mark_gc_roots();
    }

    if (compiling_context_ != nullptr) {
        compiling_context_->mark();
    }
    list_methods_->mark();
    map_methods_->mark();
    string_methods_->mark();
    iterator_methods_->mark();
    temp_root_stack_->mark();
    //conStrPool->mark();
}

void GC::trace_references()
{
    while (!grey_stack_.empty()) {
        Obj *obj = grey_stack_.top();
        grey_stack_.pop();
#ifdef DEBUG_LOG_GC
        println("{:p} blacken {}", to_void_ptr(obj), obj->to_string());
#endif
        obj->blacken();
    }
}

void GC::sweep()
{
    Obj *previous = nullptr;
    Obj *object = object_list_;
    while (object != nullptr) {
        if (object->is_marked_) {
            object->is_marked_ = false;
            previous = object;
            object = object->next_;
        } else {
            Obj *unreached = object;
            object = object->next_;
            if (previous != nullptr) {
                previous->next_ = object;
            } else {
                object_list_ = object;
            }

            free_object(unreached);
        }
    }
}

void GC::collect_garbage()
{
    if (!gc_lock_.available() || in_gc_) {
        return;
    }
    in_gc_ = true;

#ifdef DEBUG_LOG_GC
    println("=== gc begin ===");
    size_t before = bytes_allocated_;
#endif

    mark_roots();

    trace_references();

    sweep();

    next_gc_ = bytes_allocated_ * k_gc_heap_grow_factor;

#ifdef DEBUG_LOG_GC
    println("=== gc end ===");
    println(
        "   collected {} bytes (from {} to {}) next at {}",
        before - bytes_allocated_,
        before,
        bytes_allocated_,
        next_gc_);
#endif

    in_gc_ = false;
}

void GC::free_object(Obj *obj)
{
    auto size = obj->obj_size();
#ifdef DEBUG_LOG_GC
    println("{:p} free {} bytes (object {})", to_void_ptr(obj), size, Obj::type_to_str(obj->type_));
#endif
    bytes_allocated_ -= size;
    delete obj;
}

void GC::free_all_objects()
{
    while (object_list_ != nullptr) {
        Obj *next = object_list_->next_;
        free_object(object_list_);
        object_list_ = next;
    }

    while (interned_string_list_ != nullptr) {
        Obj *next = interned_string_list_->next_;
        free_object(interned_string_list_);
        interned_string_list_ = next;
    }
}

bool GC::intern_string(ObjString *obj)
{
    push_temp_root(NanBox::fromObj(obj));
    bool res = intern_pool_->insert(obj);
    pop_temp_root(1);
    return res;
}

ObjString *GC::find_interned_string(const char *chars, size_t length, uint32_t hash)
{
    return intern_pool_->find_exist(chars, length, hash);
}

} // namespace aria
