#include "operatorNew.h"
#include <memory>



void *operator new(std::size_t count)
{
    ++newCallCount();
    return malloc(count);
}

void *operator new[](std::size_t count)
{
    ++newCallCount();
    return malloc(count);
}

void *operator new(std::size_t count, const std::nothrow_t &tag)
{
  throw "xxx";
}

void *operator new[](std::size_t count, const std::nothrow_t &tag)
{
  throw "xxx";
}

int &newCallCount()
{
    static int i = 0;
    return i;
}
