#pragma once

#include <cstdlib> //for exit(1);
#include "exceptionsAvailableDetection.h"

namespace global {
namespace detail {

template<typename T>
void throwImpl(T t){

#ifdef EXCEPTIONS_DISABLED
    (void)t; //avoid unused
     exit(1);
#else
    throw t;
#endif
}

}
} //global

