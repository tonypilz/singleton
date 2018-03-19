#pragma once
#include "instance.h"
#include <functional>
#include <list>
#include <cassert>

namespace global {


template<typename T, typename Sub = typename Instance<T>::SubType >
class PendingOperations {

public:

    using Classtype = PendingOperations<T,Sub>;

    template<typename Cond, typename Func>
    static void onInstanceChange(Cond cond, Func func){

        T* p = isInstanceDefined<T,Sub>() ? &Instance<T,Sub>::get() : nullptr;

        if (cond(p)){
            func(p);
            return;
        }

        connectToHook();
        operations().emplace_back(cond,func);
    }

private:

    static void connectToHook(){

        static bool firstCall = true;
        if (firstCall==false) return;
        firstCall = false;

        auto& r = instanceHooks::instanceChangedHook<T,Sub>(); //todo check each time that handler is installed otherwise throw because somebody messed with the hook
        assert(!r);
        r = [](T* t){instanceChanged(t);};
    }

    using Condition = std::function<bool(T*)>;
    using Operation = std::function<void(T*)>;
    using Pair = std::pair<Condition,Operation>;
    using Operations = std::list<Pair>;

    static Operations& operations(){
        static Operations c;
        return c;
    }

    static void instanceChanged(T* instance) {
        Operations copy = operations();
        operations().clear();
        copy.remove_if([instance](Pair const& p){if (p.first(instance)) {p.second(instance); return true; } return false;});
        operations().insert(operations().end(),copy.begin(),copy.end());
    }

};




template<typename T, typename Sub, typename Cond, typename Func>
void onInstanceChange(Cond c, Func func){
    PendingOperations<T,Sub>::onInstanceChange(c,func);
}

template<typename T, typename Cond, typename Func>
void onInstanceChange(Cond c, Func func){
    PendingOperations<T>::onInstanceChange(c,func);
}



template<typename T, typename Func>
void onInstanceDefine(Func func){
    auto notNull = [](T* t){return t!=nullptr;};
    auto pfunc = [func](T* t){assert(t!=nullptr); func(*t);};
    PendingOperations<T>::onInstanceChange(notNull,pfunc);
}

template<typename T, typename Sub, typename Func >
void onInstanceDefine(Func func){
    auto notNull = [](T* t){return t!=nullptr;};
    auto pfunc = [func](T* t){assert(t!=nullptr); func(*t);};
    PendingOperations<T,Sub>::onInstanceChange(notNull,pfunc);
}



template<typename T, typename Sub, typename Func >
void onInstanceUndefine(Func func){
    auto null = [](T* t){return t==nullptr;};
    auto pfunc = [func](T* t){assert(t==nullptr); func();};
    PendingOperations<T,Sub>::onInstanceChange(null,pfunc);
}

template<typename T, typename Func >
void onInstanceUndefine(Func func){
    auto null = [](T* t){return t==nullptr;};
    auto pfunc = [func](T* t){assert(t==nullptr); func();};
    PendingOperations<T>::onInstanceChange(null,pfunc);
}

} //global
