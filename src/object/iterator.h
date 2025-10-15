#ifndef ITERATOR_H
#define ITERATOR_H

#include "common.h"
#include "value/value.h"

namespace aria {
class GC;
class ObjList;
class ObjMap;
class ObjString;
class Iterator
{
public:
    Iterator() = default;
    virtual ~Iterator() = default;

    virtual void blacken() = 0;
    virtual String typeString() = 0;
    virtual size_t getSize() { return sizeof(Iterator); }
    virtual bool hasNext() { return false; }
    virtual Value next() = 0;
};

class ListIterator : public Iterator
{
public:
    ListIterator() = delete;
    explicit ListIterator(ObjList *list);
    ~ListIterator() override;

    void blacken() override;
    String typeString() override;
    size_t getSize() override { return sizeof(ListIterator); }
    bool hasNext() override;
    Value next() override;

    ObjList *obj;
    int nextIndex;
};

class MapIterator : public Iterator
{
public:
    MapIterator() = delete;
    explicit MapIterator(ObjMap *map);
    ~MapIterator() override;

    void blacken() override;
    String typeString() override;
    size_t getSize() override { return sizeof(MapIterator); }
    bool hasNext() override;
    Value next() override;

    ObjMap *obj;
    // -1: begin
    // -2: reach the end
    // >0: index of next value
    int64_t nextIndex;
};

class StringIterator : public Iterator
{
public:
    StringIterator() = delete;
    explicit StringIterator(ObjString *str);
    ~StringIterator() override;

    void blacken() override;
    String typeString() override;
    size_t getSize() override { return sizeof(StringIterator); }
    bool hasNext() override;
    Value next() override;

    ObjString *obj;
    int nextIndex;
};

} // namespace aria

#endif //ITERATOR_H
