#pragma once
#include "instance.h"
#include <functional>
#include <list>
#include <cassert>

namespace global {


//use Sub for mor than one instance, ints can be converted to types (todo demonstration>, alias those instances, dynamic instance count via extra class , multiple subs via extra class
template<typename T, typename Sub = typename Instance<T>::SubType >
class PendingCalls {

public:

    using Classtype = PendingCalls<T,Sub>;

    template<typename Cond, typename Func>
    static void call(Cond c, Func func){
        T* p = isInstanceDefined<T,Sub>() ? &Instance<T,Sub>::get() : nullptr;

        if (c(p)){
            func(p);
            return;
        }

        registerRefrenceChangedHandler();
        callbacks().emplace_back(CC{c,func});
    }

private:

    static void registerRefrenceChangedHandler(){

        static bool firstCall = true;
        if (firstCall==false) return;
        firstCall = false;

        auto& r = instanceHandler::instanceChangedHandler<T,Sub>();
        assert(!r);
        r = [](T* t){applyCallbacks(t);};
    }

    using Condition = std::function<bool(T*)>;
    using Callback = std::function<void(T*)>;
    using CC = std::pair<Condition,Callback>;
    using CondCallbacks = std::list<CC>;

    static CondCallbacks& callbacks(){
        static CondCallbacks c;
        return c;
    }

    static void applyCallbacks(T* instance) {
        CondCallbacks copy = callbacks();
        callbacks().clear();
        copy.remove_if([instance](CC const& p){if (p.first(instance)) {p.second(instance); return true; } return false;});
        callbacks().insert(callbacks().end(),copy.begin(),copy.end());
    }



};



template<typename T, typename Cond, typename Func, typename Sub = typename Instance<T>::SubType>
void instanceCall(Cond c, Func func){
    PendingCalls<T,Sub>::call(c,func);
}

template<typename T, typename Func, typename Sub = typename Instance<T>::SubType>
void instanceCall(Func func){
    auto notNull = [](T* t){return t!=nullptr;};
    auto pfunc = [func](T* t){assert(t!=nullptr); func(*t);};
    PendingCalls<T,Sub>::call(notNull,pfunc);
}

}
