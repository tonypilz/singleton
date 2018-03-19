#pragma once

#include "instanceHooks.h"
#include <functional>
#include <cassert>

namespace global {

class NoSub{};

class NullptrAccess : public std::exception {};

template<typename T, typename Sub = NoSub >
class Instance {

public:
    using Type = T;
    using SubType = Sub;
    using Classtype = Instance<T,Sub>;
    using TPtr = T*;

    static T& get() {
        {
            TPtr p = ptr();
            if (p!=nullptr) return *p;
        }

        using namespace instanceHooks;

        {
            auto& h = nullptrAccessHook<T,Sub>();
            if (h) return *h();
        }


        {
            auto& h = nullptrAccessHook();
            if (h) h();
        }

        throw NullptrAccess(); //we cannot return a nullptr, so we throw

        assert(false); //we shouldnt be here
        T* nullptr_ = nullptr;
        return *nullptr_;
    }

    static bool isDefined(){return ptr()!=nullptr;}


private:

    template<typename,typename>
    friend class ReplacingInstanceRegistration;

    static void set(TPtr t) {

        if (t==ptr()) return; //nothing changed;

        ptr() = t;

        using namespace instanceHooks;

        {
            auto& h = instanceChangedHook<T,Sub>();
            if (h) h(t);
        }


        {
            auto& h = instanceChangedHook();
            if (h) h();
        }

    }


    Instance() = delete;


    static TPtr& ptr(){
        static TPtr p = nullptr;
        return p;
    }

};




template<typename T, typename Sub = typename Instance<T>::SubType>
T& instance(){return Instance<T,Sub>::get();}

template<typename T, typename Sub = typename Instance<T>::SubType>
bool isInstanceDefined(){return Instance<T,Sub>::isDefined();}

} //global
