#ifndef GC_INIT_H
#define GC_INIT_H

#include <gtest/gtest.h>

#include "src/memory/gc.h"

class GCFixture : public ::testing::Test
{
public:
    void SetUp() override { gc = new aria::GC(); }
    void TearDown() override { delete gc; }
    aria::GC *gc = nullptr;
};

class ValueTestFixture : public GCFixture
{
};

class ObjectTestFixture : public GCFixture
{
};

class CompileGenByteCodeTest : public GCFixture
{
};

#endif //GC_INIT_H
