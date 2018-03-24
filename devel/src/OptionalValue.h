#pragma once

#include <exception>


namespace global {
namespace detail {

class InvalidRead : public std::exception {};

template<typename T>
class OptionalValue {

public:
    using ValueType = T;
    using Classtype = OptionalValue<T>;

    explicit OptionalValue(){}
    OptionalValue(OptionalValue<T>const& other):val(other.val),isSet(other.isSet){}
    OptionalValue<T>const& operator=(OptionalValue<T>const& other) {val = other.val; isSet = other.isSet;}

    bool operator==(OptionalValue<T>const& other) const {return val == other.val && isSet == other.isSet;}
    bool operator==(T const& t) const {return val == t && isSet == true;}
    bool operator!=(T const& t) const {return !(*this == t);}

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

    T valueOr(T const& other){ return isSet ? val : other; }

    void unsetValue(){ isSet = false;}

private:
    T val;
    bool isSet = false;
};

}
}
