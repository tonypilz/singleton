#pragma once

#include "OptionalValue.h"

#include <functional>

namespace global {
namespace detail {

template<typename T>
class ObservableValue {

public:
    using ValueType = T;
    using Classtype = ObservableValue<T>;

    using ReadFilter = std::function<T(T const&)>;
    using ValueChanged = std::function<void (T const&)>;

    ReadFilter readFilter;
    ValueChanged valueChanged;

    explicit ObservableValue(){}
    ObservableValue(ObservableValue<T>const&) = delete;
    ObservableValue<T>const& operator=(ObservableValue<T>const&) = delete;

    bool operator==(T const& t) const{ return val == t;}
    bool operator!=(T const& t) const{ return val != t;}

    ObservableValue& operator=(T const& t){
        if (val == t) return *this; //nothing changed
        val = t;
        if (valueChanged) valueChanged(T(val));
        return *this;
    }

    T filteredValue() const{
        if (readFilter) return readFilter(T(val));
        return T(val);
    }

    T unfilteredValue() const{
        return T(val);
    }

private:

    OptionalValue<T> val;


};

}
}
