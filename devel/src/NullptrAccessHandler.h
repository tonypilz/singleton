#pragma once

#include <functional>

namespace global {

class NullptrAccess : public std::exception {};


struct NullptrAccessHandler {
    using type = std::function<void()>;
    type handler = [](){throw NullptrAccess();};
};

}
