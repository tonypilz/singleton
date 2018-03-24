#pragma once

#include "instance.h"
#include <cassert>

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
        replacedInstance = detail::initializedInstance<T,Sub>().unfilteredValue();
        detail::initializedInstance<T,Sub>() = t; //possibly deregisters again
    }

    virtual void deregisterInstance(){
        if (replacedInstance.isValueSet()==false) return; //noting to do
        T* tmp = replacedInstance;
        replacedInstance.unsetValue();
        detail::initializedInstance<T,Sub>() = tmp; //possibly registers again
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

        if (isInstanceDefined<T,Sub>()) throw InstanceReplacementNotAllowed();
        if (t==nullptr) throw RegisteringNullNotAllowed();

        Superclass::registerInstance(t);
    }


};

}//global
