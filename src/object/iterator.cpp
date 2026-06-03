#include "object/iterator.h"

#include "object/objList.h"
#include "object/objMap.h"
#include "object/objString.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"

namespace aria {

ListIterator::ListIterator(ObjList *list)
    : obj_{list}
    , next_index_{0}
{}
ListIterator::~ListIterator() = default;

void ListIterator::blacken()
{
    obj_->blacken();
}

String ListIterator::typeString()
{
    return value_type_string(NanBox::fromObj(obj_));
}

bool ListIterator::hasNext()
{
    return obj_->list_->size() > next_index_;
}

Value ListIterator::next()
{
    if (!hasNext()) {
        return NanBox::NilValue;
    }
    return (*obj_->list_)[next_index_++];
}

MapIterator::MapIterator(ObjMap *map)
    : obj_{map}
    , next_index_{-1}
{
    next_index_ = obj_->map_->get_next_index(next_index_);
}

MapIterator::~MapIterator() = default;

void MapIterator::blacken()
{
    obj_->blacken();
}

String MapIterator::typeString()
{
    return value_type_string(NanBox::fromObj(obj_));
}

bool MapIterator::hasNext()
{
    return next_index_ != -2;
}

Value MapIterator::next()
{
    if (!hasNext()) {
        return NanBox::NilValue;
    }
    Value value = obj_->map_->get_by_index(next_index_);
    next_index_ = obj_->map_->get_next_index(next_index_);
    return value;
}

StringIterator::StringIterator(ObjString *str)
    : obj_{str}
    , next_index_{0}
{}
StringIterator::~StringIterator() = default;

void StringIterator::blacken()
{
    obj_->blacken();
}

String StringIterator::typeString()
{
    return value_type_string(NanBox::fromObj(obj_));
}

bool StringIterator::hasNext()
{
    return obj_->length_ > next_index_;
}

Value StringIterator::next()
{
    if (!hasNext()) {
        return NanBox::NilValue;
    }
    return NanBox::fromObj(new_ObjString(obj_->c_str()[next_index_++], obj_->gc_));
}

} // namespace aria
