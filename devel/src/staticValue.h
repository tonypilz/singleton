#pragma once

#include <functional>

//flache hierarchie
//umbenennen in global detail staticvalue

//in detail?

template<typename T, typename Sub = void>
T& staticValue(){
    static T t;
    return t;
}

template<typename T>
class ObservableStaticValue {

public:
    using Type = T;
    using Classtype = ObservableStaticValue<T>;

    using ReadFilter = std::function<const T&(T&)>;
    using Asignment = std::function<void (T&, T&&)>;

    static const T& get() {
        const auto& filter = staticValue<ReadFilter>();
        return filter ? filter(staticValue<T,PrivateType>()) : staticValue<T,PrivateType>();
    }

    static void set(T&& t) {
        const auto& assign= staticValue<Asignment>();
        if (assign)
            assign(staticValue<T,PrivateType>(),std::forward<T>(t));
        else
            staticValue<T,PrivateType>() = t;
    }

private:
    struct PrivateType{};
    ObservableStaticValue() = delete;
};
