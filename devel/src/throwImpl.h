#pragma once

#ifdef __cpp_exceptions

namespace global {
namespace detail {

template<typename T>
T& do_throw(T t){
    throw t;
}

} //detail
} //global

#else

namespace global {
namespace detail {

#include <cstdlib>

template<typename T>
T& do_throw(T){
    exit(1);
}

} //detail
} //global

#endif // __cpp_exceptions

