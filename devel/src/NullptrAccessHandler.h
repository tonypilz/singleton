#pragma once

#include <functional>
#include "throwImpl.h"

namespace global {

class NullptrAccess : public std::exception {};

inline std::function<void()>& onNullptrAccess(){
    static std::function<void()> f = [](){ detail::throwImpl(NullptrAccess{});};
    return f;

} //global handler

}
