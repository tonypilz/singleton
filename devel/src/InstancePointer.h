#pragma once

#include "NullptrAccessHandler.h"
#include "ConditionalSingleShotOperations.h"
#include <functional>

namespace global {
namespace detail {

class UnexpectedNonNullInstance : public std::exception {};

template<typename Ptr>
class InstancePointer {

public:
    using ValueType = Ptr;
    using Classtype = InstancePointer<Ptr>;

    using NullPtrAccessHandler = std::function<Ptr()>;
    using ValueChanged = std::function<void (Ptr const&)>;

    explicit InstancePointer(){}

    bool operator==(Ptr const& t) const{ return val == t;}
    bool operator!=(Ptr const& t) const{ return val != t;}

    operator bool(){
        return val!=nullptr;
    }
    operator Ptr(){
        return operator ->();
    }

    Ptr operator->() const{
        if (val==nullptr){
            if (onNullPtrAccess) return onNullPtrAccess();
            detail::staticValue<NullptrAccessHandler>().handler(); //global handler is installed by default
            return nullptr;
        }
        return val;
    }

    Ptr rawPtr() const{
        return val;
    }


    template<typename Cond, typename Func>
    void ifAvailabilityChanged(Cond c, Func func){
        if (c(val)) {func(val); return;} // direct call if condition is met!s
        changeOperations.add([c,func](Ptr const& t){ if (c(t)) {func(t); return true;} return false;});
    }

    template<typename Func >
    void ifAvailable(Func func){
        auto notNull = [](Ptr const& t){return t!=nullptr;};
        auto pfunc = [func](Ptr const& t){if (t==nullptr) throw NullptrAccess(); func(*t);};
        ifAvailabilityChanged(notNull,pfunc);
    }

    template<typename Func >
    void ifUnavailable(Func func){
        auto null = [](Ptr const& t){return t==nullptr;};
        auto pfunc = [func](Ptr const& t){if (t!=nullptr) throw UnexpectedNonNullInstance(); func();};
        ifAvailabilityChanged(null,pfunc);
    }

    NullPtrAccessHandler onNullPtrAccess;

private:

    InstancePointer& operator=(Ptr const& t){
        if (val == t) return *this; //nothing changed
        val = t;
        changeOperations(val);
        return *this;
    }

    template<typename, typename>
    friend class ReplacingInstanceRegistration;

    InstancePointer(InstancePointer<Ptr>const&) = delete;
    InstancePointer<Ptr>const& operator=(InstancePointer<Ptr>const&) = delete;

    detail::ConditionalSingleShotOperations<Ptr> changeOperations;

    Ptr val = nullptr;
};


} //detail
} //global
