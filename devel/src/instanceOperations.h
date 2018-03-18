#pragma once
#include "instance.h"
#include <functional>
#include <list>
#include <cassert>

namespace global {


//use Sub for mor than one instance, ints can be converted to types (todo demonstration>, alias those instances, dynamic instance count via extra class , multiple subs via extra class
template<typename T, typename Sub = typename Instance<T>::SubType >
class PendingOperations {

public:

    using Classtype = PendingOperations<T,Sub>;

    template<typename Cond, typename Func>
    static void call(Cond cond, Func func){

        T* p = isInstanceDefined<T,Sub>() ? &Instance<T,Sub>::get() : nullptr;

        if (cond(p)){
            func(p);
            return;
        }

        registerRefrenceChangedHandler();
        operations().emplace_back(cond,func);
    }

private:

    static void registerRefrenceChangedHandler(){

        static bool firstCall = true;
        if (firstCall==false) return;
        firstCall = false;

        auto& r = instanceHandler::instanceChangedHandler<T,Sub>(); //todo check each time that handler is installed otherwise throw because somebody changed registration
        assert(!r);
        r = [](T* t){callOperations(t);};
    }

    using Condition = std::function<bool(T*)>;
    using Operation = std::function<void(T*)>;
    using Pair = std::pair<Condition,Operation>;
    using Operations = std::list<Pair>;

    static Operations& operations(){
        static Operations c;
        return c;
    }

    static void callOperations(T* instance) {
        Operations copy = operations();
        operations().clear();
        copy.remove_if([instance](Pair const& p){if (p.first(instance)) {p.second(instance); return true; } return false;});
        operations().insert(operations().end(),copy.begin(),copy.end());
    }

};



template<typename T, typename Sub, typename Cond, typename Func>
void onInstance(Cond c, Func func){
    PendingOperations<T,Sub>::call(c,func);
}

template<typename T, typename Cond, typename Func>
void onInstance(Cond c, Func func){
    PendingOperations<T>::call(c,func);
}

template<typename T, typename Func>
void onInstance(Func func){
    auto notNull = [](T* t){return t!=nullptr;};
    auto pfunc = [func](T* t){assert(t!=nullptr); func(*t);};
    PendingOperations<T>::call(notNull,pfunc);
}

template<typename T, typename Sub, typename Func >
void onInstance(Func func){
    auto notNull = [](T* t){return t!=nullptr;};
    auto pfunc = [func](T* t){assert(t!=nullptr); func(*t);};
    PendingOperations<T,Sub>::call(notNull,pfunc);
}


} //global
