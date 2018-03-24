#pragma once

#include "staticValue.h"
#include "ObservableValue.h"
#include "NullptrAccessHandler.h"
#include <functional>
#include <cassert>

namespace global {


class ReadFilterNotExpected : public std::exception {};


namespace detail {

template<typename T, typename Sub>
ObservableValue<T*>& initializedInstance(){

    static bool firstCall = true;
    if (firstCall==false) return staticValue<ObservableValue<T*>,Sub>();
    firstCall = false;

    auto& value = staticValue<ObservableValue<T*>,Sub>();
    value = nullptr; //no invalid reads

    if (value.readFilter) throw ReadFilterNotExpected();

    value.readFilter = [](T* const& t){
        if (t==nullptr) {

            {
                auto& h = staticValue<NullptrAccessHandlerT<T*>, Sub>().handler;
                if (h) return h();
            }

            {
                auto& h = staticValue<NullptrAccessHandler,Sub>().handler;
                if (h) h();
            }

            {
                auto& h = staticValue<NullptrAccessHandler>().handler;
                h(); //throws if not set!
            }
        }

        return t;
    };

    return value;
}

} //detail


template<typename T, typename Sub = detail::staticValueSubDefault>
T* instancePtrOr(T* alt){ //this method retreives the value withouth filtering
    auto& i = detail::initializedInstance<T,Sub>();
    return i==nullptr ? alt : i.unfilteredValue();
}

template<typename T, typename Sub = detail::staticValueSubDefault>
T* instancePtr(){ return detail::initializedInstance<T,Sub>().filteredValue(); }

template<typename T, typename Sub = detail::staticValueSubDefault>
T& instance(){ return *instancePtr<T,Sub>(); }

template<typename T, typename Sub = detail::staticValueSubDefault>
bool isInstanceDefined(){return detail::initializedInstance<T,Sub>()!=nullptr;} //value is always set since its initialized

template<typename T, typename Sub = detail::staticValueSubDefault>
typename NullptrAccessHandlerT<T*>::type& onNullptrAccess(){ return detail::staticValue<NullptrAccessHandlerT<T*>,Sub>().handler; }

inline NullptrAccessHandler::type& onNullptrAccess(){ return detail::staticValue<NullptrAccessHandler>().handler; }





} //global
