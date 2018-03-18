#pragma once

#include <functional>

namespace global {
namespace instanceHandler {



inline std::function<void()>& nullptrAccessHandler(){
    static std::function<void()> h;
    return h;
}

template<typename T, typename Sub>
std::function<T*()>& nullptrAccessHandler(){ //todo remove T
    static std::function<T*()> h;
    return h;
}

inline std::function<void()>& instanceChangedHandler(){
    static std::function<void()> h; //todo auto
    return h;
}

template<typename T, typename Sub>
std::function<void(T*)>& instanceChangedHandler(){
    static std::function<void(T*)> h;
    return h;
}

}
}
