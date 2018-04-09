#pragma once

#include <cstdlib> //for exit(1);

namespace global {
namespace detail {

template<typename T>
T& throwImpl(T t){

#ifdef __cpp_exceptions
    throw t;
#else // __cpp_exceptions
    (void)t; //avoid unused
     exit(1);
#endif // __cpp_exceptions
}

} //detail
} //global


