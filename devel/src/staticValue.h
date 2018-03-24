#pragma once

namespace global {
namespace detail {

struct staticValueSubDefault{};

template<typename T, typename Sub = staticValueSubDefault>
T& staticValue(){
    static T t;
    return t;
}


} //detail
} //global
