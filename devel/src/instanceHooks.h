#pragma once

#include <functional>

namespace global {
namespace instanceHooks {



inline std::function<void()>& nullptrAccessHook(){
    static std::function<void()> h;
    return h;
}

template<typename T, typename Sub>
std::function<T*()>& nullptrAccessHook(){ //todo remove T
    static std::function<T*()> h;
    return h;
}

inline std::function<void()>& instanceChangedHook(){
    static std::function<void()> h; //todo auto
    return h;
}

template<typename T, typename Sub>
std::function<void(T*)>& instanceChangedHook(){
    static std::function<void(T*)> h;
    return h;
}

}
}
