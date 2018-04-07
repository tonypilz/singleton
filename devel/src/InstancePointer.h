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


    explicit operator const T*() const{  return operator ->(); }
    explicit operator T*() {  return operator ->(); }

    const T& operator*() const& { return *instancePtr;}
    T& operator*() & { return *instancePtr;}


    const T* operator->() const{ return instancePtr!=nullptr ? instancePtr : handleNull(); }
    T* operator->(){ return instancePtr!=nullptr ? instancePtr : handleNull(); }


    template<typename DeferredOperation>
    void addDeferredOperation(DeferredOperation op){
        deferredOperations.addDeferredOperation(op);
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

    std::function<T*()> onNullPtrAccess;
    std::function<void()> onNullPtrAccessUntyped;

private:

    T* handleNull() const{
        if (onNullPtrAccess) return onNullPtrAccess();
        if (onNullPtrAccessUntyped) onNullPtrAccessUntyped(); //if this returns we execute global handler
        detail::staticValue<NullptrAccessHandler>().handler(); //global handler should always be there
        return nullptr; //shouldnt be reached
    }

    InstancePointer& operator=(T* t){
        if (instancePtr == t) return *this; //nothing changed
        instancePtr = t;
        deferredOperations.conditionsChanged(instancePtr);
        return *this;
    }

    template<typename, typename>
    friend class ReplacingInstanceRegistration;

    using ClassType = InstancePointer<T>;
    InstancePointer(ClassType const&) = delete;
    ClassType const& operator=(ClassType const&) = delete;

    detail::DeferredOperations<T> deferredOperations; //todo

    T* instancePtr = nullptr;
};


} //detail
} //global
