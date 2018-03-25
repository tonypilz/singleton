#pragma once

#include "OptionalValue.h"
#include "instance.h"
#include <utility>

namespace global {



//replaces existing for the time it exised
template<typename T, typename Sub = detail::staticValueSubDefault>
class ReplacingInstanceRegistration {

public:

    ReplacingInstanceRegistration(){}
    ReplacingInstanceRegistration(T* t){registerInstance(t);}
    void operator()(T* t){registerInstance(t);}
    virtual ~ReplacingInstanceRegistration(){deregisterInstance();}

    virtual void registerInstance(T* t){
        deregisterInstance();
        replacedInstance = instance<T,Sub>().ptr();
        instance<T,Sub>() = t; //possibly deregisters again
    }

    virtual void deregisterInstance(){
        if (replacedInstance.isValueSet()==false) return; //noting to do
        T* tmp = replacedInstance;
        replacedInstance.unsetValue();
        instance<T,Sub>() = tmp; //possibly registers again
    }

private:

    ReplacingInstanceRegistration(ReplacingInstanceRegistration const&) = delete; //no copy

    detail::OptionalValue<T*> replacedInstance;

};




class InstanceReplacementNotAllowed : public std::exception {};
class RegisteringNullNotAllowed: public std::exception {};



//expects nullptr to be registered beforehand
//expects registration-target not to be null
template<typename T, typename Sub = detail::staticValueSubDefault>
class InstanceRegistration : ReplacingInstanceRegistration<T,Sub> {
public:

    using Superclass = ReplacingInstanceRegistration<T,Sub>;
    using Superclass::operator();

    InstanceRegistration(): Superclass(){}
    InstanceRegistration(T* t): Superclass(){registerInstance(t);}

    void registerInstance(T* t) override{

        if (instance<T,Sub>()!=nullptr) throw InstanceReplacementNotAllowed();
        if (t==nullptr) throw RegisteringNullNotAllowed();

        Superclass::registerInstance(t);
    }


};


template<
    template<typename, typename> class RegistrationType,
    typename T,
    typename Sub,
    typename R>
class RegisterdInstanceT {

    T t;
    RegistrationType<R,Sub> reg;

public:
    template<typename... Args>
    RegisterdInstanceT(Args&&... args):t(std::forward<Args>(args)...),reg(&t){}

};

//default sub type
template<typename T, typename R = T, typename Sub = detail::staticValueSubDefault >
using RegisterdInstance = RegisterdInstanceT<InstanceRegistration, T, Sub, R>;

template<typename T, typename R = T, typename Sub = detail::staticValueSubDefault >
using RRegisterdInstance = RegisterdInstanceT<ReplacingInstanceRegistration, T, Sub, R>;

//default registration type
template<typename T, typename Sub = detail::staticValueSubDefault, typename R = T >
using RegisterdInstanceS = RegisterdInstanceT<InstanceRegistration, T, Sub, R>;

template<typename T, typename Sub = detail::staticValueSubDefault, typename R = T >
using RRegisterdInstanceS = RegisterdInstanceT<ReplacingInstanceRegistration, T, Sub, R>;

}//global
