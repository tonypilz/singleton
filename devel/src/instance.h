#pragma once

#include "staticValue.h"
#include "InstancePointer.h"


namespace global {


template<typename T>
detail::InstancePointer<T>& instance(){ return detail::staticValue<detail::InstancePointer<T>>();}

template<typename T>
T& instanceRef(){ return *detail::staticValue<detail::InstancePointer<T>>(); }

inline NullptrAccessHandler::type& onNullptrAccess(){ return detail::staticValue<NullptrAccessHandler>().handler; } //global handler



} //global
