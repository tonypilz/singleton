#pragma once

#include "NullptrAccessHandler.h"
#include "DeferredOperations.h"
#include <functional>

namespace global {
namespace detail {

template<typename Ptr>
class InstancePointer {

public:

    using NullPtrAccessHandler = std::function<Ptr()>;

    explicit InstancePointer(){}

    bool operator==(Ptr const& t) const{ return val == t;}
    bool operator!=(Ptr const& t) const{ return val != t;}

    operator bool() const{
        return val!=nullptr;
    }
    operator Ptr() const{
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


    template<typename Cond, typename Func>
    void addDeferredOperation(Cond c, Func func){
        deferredOperations.addDeferredOperation(c,func);
        deferredOperations.conditionsChanged(val);
    }

    template<typename Func >
    void ifAvailable(Func func){
        deferredOperations.ifAvailable(func);
        deferredOperations.conditionsChanged(val);
    }


    template<typename Func >
    void ifUnavailable(Func func){
        deferredOperations.ifUnavailable(func);
        deferredOperations.conditionsChanged(val);
    }

    NullPtrAccessHandler onNullPtrAccess;

private:

    Ptr rawPtr() const{
        return val;
    }

    InstancePointer& operator=(Ptr const& t){
        if (val == t) return *this; //nothing changed
        val = t;
        deferredOperations.conditionsChanged(val);
        return *this;
    }

    template<typename, typename>
    friend class ReplacingInstanceRegistration;

    InstancePointer(InstancePointer<Ptr>const&) = delete;
    InstancePointer<Ptr>const& operator=(InstancePointer<Ptr>const&) = delete;

    detail::DeferredOperations<Ptr> deferredOperations;

    Ptr val = nullptr;
};


} //detail
} //global
