#pragma once

#include "NullptrAccessHandler.h"
#include "DeferredOperations.h"
#include <functional>

namespace global {
namespace detail {

template<typename Ptr>
class InstancePointer {

public:

    explicit InstancePointer(){}

    bool operator==(Ptr const& t) const{ return instancePtr == t;}
    bool operator!=(Ptr const& t) const{ return instancePtr != t;}

    operator bool() const{ return instancePtr!=nullptr; }
    operator Ptr() const{  return operator ->(); }

    Ptr operator->() const{ return instancePtr!=nullptr ? instancePtr : handleNull(); }

    template<typename Cond, typename Func>
    void addDeferredOperation(Cond c, Func func){
        deferredOperations.addDeferredOperation(c,func);
        deferredOperations.conditionsChanged(instancePtr);
    }

    template<typename Func >
    void ifAvailable(Func func){
        deferredOperations.ifAvailable(func);
        deferredOperations.conditionsChanged(instancePtr);
    }

    template<typename Func >
    void ifUnavailable(Func func){
        deferredOperations.ifUnavailable(func);
        deferredOperations.conditionsChanged(instancePtr);
    }

    std::function<Ptr()> onNullPtrAccess;
    std::function<void()> onNullPtrAccessUntyped;

private:

    Ptr handleNull() const{
        if (onNullPtrAccess) return onNullPtrAccess();
        if (onNullPtrAccessUntyped) onNullPtrAccessUntyped(); //if this returns we execute global handler
        detail::staticValue<NullptrAccessHandler>().handler(); //global handler should always be there
        return nullptr; //shouldnt be reached
    }

    InstancePointer& operator=(Ptr const& t){
        if (instancePtr == t) return *this; //nothing changed
        instancePtr = t;
        deferredOperations.conditionsChanged(instancePtr);
        return *this;
    }

    template<typename, typename>
    friend class ReplacingInstanceRegistration;

    InstancePointer(InstancePointer<Ptr>const&) = delete;
    InstancePointer<Ptr>const& operator=(InstancePointer<Ptr>const&) = delete;

    detail::DeferredOperations<Ptr> deferredOperations;

    Ptr instancePtr = nullptr;
};


} //detail
} //global
