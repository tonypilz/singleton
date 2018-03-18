#pragma once

#include "instance.h"
#include <cassert>

namespace global {

class InstanceReplacementNotAllowed : public std::exception {};
class RegisteringNullNotAllowed: public std::exception {};


//replaces existing for the time it exised
template<typename T, typename Sub = typename Instance<T>::SubType>
class ReplacingScopedRegistration {

public:

    ReplacingScopedRegistration(){}
    ReplacingScopedRegistration(T* t){registerInstance(t);}
    void operator()(T* t){registerInstance(t);}
    virtual ~ReplacingScopedRegistration(){deregisterInstance();}

    virtual void registerInstance(T* t){
        deregisterInstance();
        instanceHasBeenReplaced = true;

        assert(replacedInstance==nullptr);

        if (isInstanceDefined<T,Sub>()){
            replacedInstance = &Instance<T,Sub>::get();
        } else {
            replacedInstance = nullptr;
        }

        Instance<T,Sub>::set(t); //possibly deregisters again
    }

    virtual void deregisterInstance(){
        if (instanceHasBeenReplaced==false) return; //noting to do
        instanceHasBeenReplaced = false;

        auto tmp = replacedInstance;
        replacedInstance = nullptr;

        Instance<T,Sub>::set(tmp); //possibly registers again
    }

private:

    ReplacingScopedRegistration(ReplacingScopedRegistration const&) = delete; //no copy

    T* replacedInstance = nullptr;
    bool instanceHasBeenReplaced = false;

};



//expects nullptr to be registered beforehand
//expects instance which is to be registered to be not null
template<typename T, typename Sub = typename Instance<T>::SubType>
class ExclusiveRegistration : ReplacingScopedRegistration<T,Sub> {
public:

    using Superclass = ReplacingScopedRegistration<T,Sub>;

    using ReplacingScopedRegistration<T,Sub>::operator();

    ExclusiveRegistration(): ReplacingScopedRegistration<T,Sub>(){}
    ExclusiveRegistration(T* t): ReplacingScopedRegistration<T,Sub>(){registerInstance(t);}
    void operator()(T* t){registerInstance(t);}

    void registerInstance(T* t) override{

        if (isInstanceDefined<T,Sub>()) throw InstanceReplacementNotAllowed();
        if (t==nullptr) throw RegisteringNullNotAllowed();

        Superclass::registerInstance(t);
    }


};

}
