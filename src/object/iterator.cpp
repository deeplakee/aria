#include "object/iterator.h"

#include "object/objList.h"
#include "object/objMap.h"
#include "object/objString.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"

namespace aria {

ListIterator::ListIterator(ObjList *list)
    : obj{list}
    , nextIndex{0}
{}
ListIterator::~ListIterator() = default;

void ListIterator::blacken()
{
    obj->blacken();
}

String ListIterator::typeString()
{
    return valueTypeString(obj_val(obj));
}

bool ListIterator::hasNext()
{
    return obj->list->size() > nextIndex;
}

Value ListIterator::next()
{
    if (!hasNext()) {
        return nil_val;
    }
    return (*obj->list)[nextIndex++];
}

MapIterator::MapIterator(ObjMap *map)
    : obj{map}
    , nextIndex{-1}
{
    nextIndex = obj->map->getNextIndex(nextIndex);
}

MapIterator::~MapIterator() = default;

void MapIterator::blacken()
{
    obj->blacken();
}

String MapIterator::typeString()
{
    return valueTypeString(obj_val(obj));
}

bool MapIterator::hasNext()
{
    return nextIndex != -2;
}

Value MapIterator::next()
{
    if (!hasNext()) {
        return nil_val;
    }
    Value value = obj->map->getByIndex(nextIndex);
    nextIndex = obj->map->getNextIndex(nextIndex);
    return value;
}

StringIterator::StringIterator(ObjString *str)
    : obj{str}
    , nextIndex{0}
{}
StringIterator::~StringIterator() = default;

void StringIterator::blacken()
{
    obj->blacken();
}

String StringIterator::typeString()
{
    return valueTypeString(obj_val(obj));
}

bool StringIterator::hasNext()
{
    return obj->length > nextIndex;
}

Value StringIterator::next()
{
    if (!hasNext()) {
        return nil_val;
    }
    return obj_val(newObjString(obj->C_str_ref()[nextIndex++], obj->gc));
}

} // namespace aria
