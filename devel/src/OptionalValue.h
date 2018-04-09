#pragma once

#include <exception>
#include "throwImpl.h"


namespace global {
namespace detail {

class bad_optional_access_impl : public std::exception {};

using bad_optional_access = bad_optional_access_impl; //use c++17 version if available

template<typename T>
class OptionalValueImpl {

public:

    explicit OptionalValueImpl(){}

    OptionalValueImpl& operator=(T const& t){
        val = t;
        m_hasValue = true;
        return *this;
    }

    explicit operator T() const{
        if (!m_hasValue)  throwImpl(bad_optional_access{});
        return val;
    }

    bool has_value() const{return m_hasValue;}
    void reset(){ m_hasValue = false;}

private:
    T val;
    bool m_hasValue = false;
};

template<typename T>
using optional = OptionalValueImpl<T>; //use c++17 version if available

}
}
