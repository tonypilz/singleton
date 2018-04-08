#include "operatorNew.h"
#include <src/throwImpl.h>
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

void *operator new(std::size_t, const std::nothrow_t &)
{
    global::detail::do_throw(std::exception{});
}

void *operator new[](std::size_t, const std::nothrow_t &)
{
  global::detail::do_throw(std::exception{});
}

int &newCallCount()
{
    static int i = 0;
    return i;
}
