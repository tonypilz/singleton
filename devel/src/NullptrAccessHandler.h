#pragma once

#include <functional>
#include "throwImpl.h"

namespace global {

class NullptrAccess : public std::exception {};

template<typename T = void>
void onNullPtrAccess(){ detail::throwImpl(NullptrAccess{}); }

//override by spcializing
//template<> void onNullPtrAccess<>(){ exit(1); }


} //global

