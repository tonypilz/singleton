#pragma once

namespace global {
namespace detail {

template<typename T>
T& staticValue(){
    static T t;
    return t;
}


} //detail
} //global
