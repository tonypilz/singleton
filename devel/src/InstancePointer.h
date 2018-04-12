#pragma once

#include "NullptrAccessHandler.h"
#include "DeferredOperations.h"
#include <functional>

namespace global {
namespace detail {

template<typename T>
class InstancePointer {

public:

    explicit InstancePointer(){}

    operator bool() const{ return instancePtr!=nullptr; }

    bool operator==(T const* t) const{  return instancePtr==t;}
    bool operator!=(T const* t) const{  return instancePtr!=t;}

    explicit operator T*() const{  return operator ->(); }

    T& operator*() const& { return *instancePtr;}
    T* operator->() const{ return instancePtr!=nullptr ? instancePtr : handleNull(); }


    template<typename Op >
    void addDeferredOperationWithArgBefore(Op func){
        deferredOperations.addDeferredOperationWithArgBefore(func,instancePtr);
    }

    template<typename DeferredOperation>
    void addDeferredOperation(DeferredOperation op){
        deferredOperations.addDeferredOperation(op,instancePtr);
    }

    template<typename Func >
    void ifAvailable(Func func){
        deferredOperations.ifAvailable(func,instancePtr);
    }

    template<typename Func >
    void becomesUnavailable(Func func){
        deferredOperations.becomesUnavailable(func,instancePtr);
    }

    std::function<T*()> onNullPtrAccess;
    std::function<void()> onNullPtrAccessUntyped;

private:

    T* handleNull() const{
        if (onNullPtrAccess) return onNullPtrAccess();
        if (onNullPtrAccessUntyped) onNullPtrAccessUntyped(); //if this returns we execute global handler
        onNullptrAccess(); //global handler should always be there
        return nullptr; //shouldnt be reached
    }

    InstancePointer& operator=(T* t){
        if (instancePtr == t) return *this; //nothing changed
        auto before = instancePtr;
        instancePtr = t;
        deferredOperations.conditionsChanged(before, instancePtr);
        return *this;
    }

    template<typename>
    friend class ReplacingInstanceRegistration;

    using ClassType = InstancePointer<T>;
    InstancePointer(ClassType const&) = delete;
    ClassType const& operator=(ClassType const&) = delete;

    detail::DeferredOperations<T> deferredOperations; //todo

    T* instancePtr = nullptr;
};


} //detail
} //global
