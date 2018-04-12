#pragma once

#include "NullptrAccessHandler.h"
#include <functional>
#include <list>

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
    T* operator->() const{ if (instancePtr==nullptr) global::onNullPtrAccess<>(); return instancePtr; }

    template<typename Func >
    void ifAvailable(Func func){
        if (instancePtr!=nullptr) { func(*instancePtr); return; }
        ifAvailableOps.emplace_back(func);
    }

    template<typename Func >
    void becomesUnavailable(Func func){
        becomesUnavailableOps.emplace_back(func); //never directly
    }

private:

    InstancePointer& operator=(T* t){
        if (instancePtr == t) return *this; //nothing changed
        auto before = instancePtr;
        instancePtr = t;

        if (instancePtr!=nullptr) { for(auto const& op:ifAvailableOps) op(*instancePtr); ifAvailableOps.clear();}
        if (before!=nullptr && instancePtr==nullptr ) { for(auto const& op:becomesUnavailableOps) op(*before); becomesUnavailableOps.clear(); }
        return *this;
    }

    template<typename>
    friend class ReplacingInstanceRegistration;

    using ClassType = InstancePointer<T>;

    InstancePointer(ClassType const&) = delete;
    ClassType const& operator=(ClassType const&) = delete;

    using DeferredOperation = std::list<std::function<void(T&)>>;
    DeferredOperation ifAvailableOps;
    DeferredOperation becomesUnavailableOps;

    T* instancePtr = nullptr;
};


} //detail
} //global
