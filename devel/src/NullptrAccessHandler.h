#pragma once

#include <functional>
#include "throwImpl.h"

namespace global {

class NullptrAccess : public std::exception {};


struct NullptrAccessHandler {
    using type = std::function<void()>;
    type handler = [](){ detail::do_throw(NullptrAccess{});};
};

}
