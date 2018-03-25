#pragma once

#include "staticValue.h"
#include "NullptrAccessHandler.h"
#include "ConditionalSingleShotOperations.h"
#include <functional>
#include <cassert>

namespace global {
namespace detail {

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

    Ptr ptr() const{
        return val;
    }


    template<typename Cond, typename Func>
    void visitIf(Cond c, Func func){
        if (c(val)) {func(val); return;} // direct call if condition is met!s
        changeOperations.add([c,func](Ptr const& t){ if (c(t)) {func(t); return true;} return false;});
    }

    template<typename Func >
    void visitIfNotNull(Func func){
        auto notNull = [](Ptr const& t){return t!=nullptr;};
        auto pfunc = [func](Ptr const& t){assert(t!=nullptr); func(*t);};
        visitIf(notNull,pfunc);
    }

    template<typename Func >
    void visitIfNull(Func func){
        auto null = [](Ptr const& t){return t==nullptr;};
        auto pfunc = [func](Ptr const& t){assert(t==nullptr); func();};
        visitIf(null,pfunc);
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

template<typename T, typename Sub = detail::staticValueSubDefault>
detail::InstancePointer<T*>& instance(){ return detail::staticValue<detail::InstancePointer<T*>>();}

inline NullptrAccessHandler::type& onNullptrAccess(){ return detail::staticValue<NullptrAccessHandler>().handler; } //global handler



} //global
