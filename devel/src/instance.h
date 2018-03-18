#pragma once

#include "instanceHandler.h"
#include <functional>
#include <cassert>

namespace global {

class NoSub{};

class NullptrAccess : public std::exception {};

//use Sub for mor than one instance, ints can be converted to types (todo demonstration>, alias those instances, dynamic instance count via extra class , multiple subs via extra class
template<typename T, typename Sub = NoSub >
class Instance {

public:
    using TType = T;
    using SubType = Sub;
    using Classtype = Instance<T,Sub>;
    using TPtr = T*;

    static T& get() {
        {
            TPtr p = ptr();
            if (p!=nullptr) return *p;
        }

        using namespace instanceHandler;

        {
            auto& h = nullptrAccessHandler<T,Sub>();
            if (h) return *h();
        }


        {
            auto& h = nullptrAccessHandler();
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
    friend class ReplacingScopedRegistration;

    static void set(TPtr t) {

        if (t==ptr()) return; //nothing changed;

        ptr() = t;

        using namespace instanceHandler;

        {
            auto& h = instanceChangedHandler<T,Sub>();
            if (h) h(t);
        }


        {
            auto& h = instanceChangedHandler();
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
T& instance(){return Instance<T,Sub>::get();} //just another name

template<typename T, typename Sub = typename Instance<T>::SubType>
bool isInstanceDefined(){return Instance<T,Sub>::isDefined();} //just another name

}
