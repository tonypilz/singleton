#pragma once

#include "OptionalValue.h"
#include "instance.h"
#include <utility>

namespace global {


class InstanceReplacementNotAllowed : public std::exception {};
class RegisteringNullNotAllowed: public std::exception {};



namespace detail {

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
        replacedInstance = instance<T,Sub>().instancePtr;
        instance<T,Sub>() = t; //possibly deregisters again
    }

    virtual void deregisterInstance(){
        if (replacedInstance.has_value()==false) return; //noting to do
        T *tmp = static_cast<T*>(replacedInstance);
        replacedInstance.reset();
        instance<T,Sub>() = tmp; //possibly registers again
    }

private:

    ReplacingInstanceRegistration(ReplacingInstanceRegistration const&) = delete; //no copy

    detail::optional<T*> replacedInstance;

};







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
    typename R,
    typename Sub,
    typename T>
class RegisterdInstanceT {

    T t;
    RegistrationType<R,Sub> reg;

public:
    template<typename... Args>
    RegisterdInstanceT(Args&&... args):t(std::forward<Args>(args)...),reg(&t){}

};

} //detail

template<typename R, typename T = R, typename Sub = detail::staticValueSubDefault>
using Instance = detail::RegisterdInstanceT<detail::InstanceRegistration, R, Sub, T>;

template<typename R, typename T = R, typename Sub = detail::staticValueSubDefault>
using TestInstance = detail::RegisterdInstanceT<detail::ReplacingInstanceRegistration, R, Sub, T>;

template<typename R, typename Sub, typename T = R>
using SubInstance = detail::RegisterdInstanceT<detail::InstanceRegistration, R, Sub, T>;

template<typename R, typename Sub, typename T = R>
using SubTestInstance = detail::RegisterdInstanceT<detail::ReplacingInstanceRegistration, R, Sub, T>;


#define GLOBAL_INSTANCE_IS_FRIEND template< template<typename, typename> class, typename , typename , typename > friend class ::global::detail::RegisterdInstanceT


}//global
