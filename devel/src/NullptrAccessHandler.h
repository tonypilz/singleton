#pragma once

#include <functional>

namespace global {

class NullptrAccess : public std::exception {};


template<typename T>
struct NullptrAccessHandlerT{
    using type = std::function<T()>;
    type handler;
};

struct NullptrAccessHandler {
    using type = std::function<void()>;
    type handler = [](){throw NullptrAccess();};
};

}
