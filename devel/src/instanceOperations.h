#pragma once
#include "ConditionalSingleShotOperations.h"
#include "staticValue.h"
#include "ObservableValue.h"
#include "instance.h"


namespace global {

class ValueChangedFilterNotExpected : public std::exception {};

namespace detail {


template<typename T, typename Sub>
ConditionalSingleShotOperations<T*>& registeredConditionalSingleShotOperations(){

    using Ops = ConditionalSingleShotOperations<T*>;

    static bool firstCall = true;
    if (firstCall==false) return staticValue<Ops,Sub>();
    firstCall = false;

    auto& value = staticValue<ObservableValue<T*>,Sub>();



    if (value.valueChanged) throw ValueChangedFilterNotExpected();
    value.valueChanged = [](T* const& t){staticValue<Ops,Sub>().execute(t);};

    return staticValue<Ops,Sub>();

}

} //detail


template<typename T, typename Sub, typename Cond, typename Func>
void onInstanceChange(Cond c, Func func){
    {
        auto val = instancePtrOr<T,Sub>(nullptr);
        if (c(val)) {func(val); return;} // direct call if condition is met!s
    }
    detail::registeredConditionalSingleShotOperations<T,Sub>().add([c,func](T* const& t){ if (c(t)) {func(t); return true;} return false;});
}

template<typename T, typename Cond, typename Func>
void onInstanceChange(Cond c, Func func){
    onInstanceChange<T,detail::staticValueSubDefault,Cond,Func>(c,func);
}




template<typename T, typename Sub, typename Func >
void onInstanceDefine(Func func){
    auto notNull = [](T* const& t){return t!=nullptr;};
    auto pfunc = [func](T* const& t){assert(t!=nullptr); func(*t);};
    onInstanceChange<T,Sub>(notNull,pfunc);

}

template<typename T, typename Func>
void onInstanceDefine(Func func){
    onInstanceDefine<T,detail::staticValueSubDefault,Func>(func);
}





template<typename T, typename Sub, typename Func >
void onInstanceUndefine(Func func){
    auto null = [](T* t){return t==nullptr;};
    auto pfunc = [func](T* t){assert(t==nullptr); func();};
    onInstanceChange<T,Sub>(null,pfunc);
}

template<typename T, typename Func >
void onInstanceUndefine(Func func){
    onInstanceUndefine<T,detail::staticValueSubDefault,Func>(func);
}

} //global
