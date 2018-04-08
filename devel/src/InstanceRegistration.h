#pragma once

#include "OptionalValue.h"
#include "instance.h"
#include "throwImpl.h"
#include <utility>

namespace global {


class InstanceReplacementNotAllowed : public std::exception {};
class RegisteringNullNotAllowed: public std::exception {};



namespace detail {

//replaces existing for the time it exised
template<typename T>
class ReplacingInstanceRegistration {

public:

    ReplacingInstanceRegistration(){}
    ReplacingInstanceRegistration(T* t){registerInstance(t);}
    void operator()(T* t){registerInstance(t);}
    virtual ~ReplacingInstanceRegistration(){deregisterInstance();}

    virtual void registerInstance(T* t){
        deregisterInstance();
        replacedInstance = instance<T>().instancePtr;
        instance<T>() = t; //possibly deregisters again
    }

    virtual void deregisterInstance(){
        if (replacedInstance.has_value()==false) return; //noting to do
        T *tmp = static_cast<T*>(replacedInstance);
        replacedInstance.reset();
        instance<T>() = tmp; //possibly registers again
    }

private:

    ReplacingInstanceRegistration(ReplacingInstanceRegistration const&) = delete; //no copy

    detail::optional<T*> replacedInstance;

};







//expects nullptr to be registered beforehand
//expects registration-target not to be null
template<typename T>
class InstanceRegistration : ReplacingInstanceRegistration<T> {
public:

    using Superclass = ReplacingInstanceRegistration<T>;
    using Superclass::operator();

    InstanceRegistration(): Superclass(){}
    InstanceRegistration(T* t): Superclass(){registerInstance(t);}

    void registerInstance(T* t) override{

        if (instance<T>()!=nullptr) do_throw(InstanceReplacementNotAllowed{});
        if (t==nullptr) do_throw(RegisteringNullNotAllowed{});

        Superclass::registerInstance(t);
    }


};


template<
    template<typename> class RegistrationType,
    typename AccessType,
    typename InstanceType>
class RegisterdInstanceT {

    InstanceType t;
    RegistrationType<AccessType> reg;

public:
    template<typename... Args>
    RegisterdInstanceT(Args&&... args):t(std::forward<Args>(args)...),reg(&t){}

};

} //detail

template<typename AccessType, typename InstanceType = AccessType>
using Instance = detail::RegisterdInstanceT<detail::InstanceRegistration, AccessType, InstanceType>;

template<typename AccessType, typename InstanceType = AccessType>
using TestInstance = detail::RegisterdInstanceT<detail::ReplacingInstanceRegistration, AccessType, InstanceType>;


#define GLOBAL_INSTANCE_IS_FRIEND template< template<typename, typename> class, typename , typename > friend class ::global::detail::RegisterdInstanceT


}//global
