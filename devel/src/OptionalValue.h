#pragma once

#include <exception>


namespace global {
namespace detail {

class InvalidRead : public std::exception {};

template<typename T>
class OptionalValue {

public:

    explicit OptionalValue(){}

    OptionalValue& operator=(T const& t){
        val = t;
        isSet = true;
        return *this;
    }

    explicit operator T() const{
        if (!isSet)  throw InvalidRead();
        return val;
    }

    bool isValueSet() const{return isSet;}
    void unsetValue(){ isSet = false;}

private:
    T val;
    bool isSet = false;
};

}
}
