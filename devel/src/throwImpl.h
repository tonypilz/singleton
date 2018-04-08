#pragma once

namespace global {
namespace detail {

template<typename T>
T& do_throw(T t){
    throw t;
}

} //detail
} //global
