#include "operatorNew.h"
#include <src/globalInstances.h>
#include <memory>


int &newCallCount()
{
    static int i = 0;
    return i;
}


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

//void *operator new(std::size_t count, const std::nothrow_t &)
//{
//    global::detail::do_throw(std::exception{}); //unexpected
//    return malloc(count);
//}

//void *operator new[](std::size_t count, const std::nothrow_t &)
//{
//    global::detail::do_throw(std::exception{}); //unexpected
//    return malloc(count);
//}


