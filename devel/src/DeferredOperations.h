#pragma once

#include "NullptrAccessHandler.h"
#include <list>
#include <functional>

namespace global {
namespace detail {


class UnexpectedNonNullInstance : public std::exception {};

enum class ConditionState{
    condition_not_met,
    operation_executed
};

template<typename T>
class DeferredOperations {
public:

    template<typename Cond, typename Func>
    void addDeferredOperation(Cond c, Func func){
        operations.emplace_back([c,func](T const& t){ if (c(t)) {func(t); return true;} return false;});
    }

    template<typename Func >
    void ifAvailable(Func func){
        auto notNull = [](T const& t){return t!=nullptr;};
        auto pfunc = [func](T const& t){if (t==nullptr) throw NullptrAccess(); func(*t);};
        addDeferredOperation(notNull,pfunc);
    }

    template<typename Func >
    void ifUnavailable(Func func){
        auto null = [](T const& t){return t==nullptr;};
        auto pfunc = [func](T const& t){if (t!=nullptr) throw UnexpectedNonNullInstance(); func();};
        addDeferredOperation(null,pfunc);
    }


    void conditionsChanged(T const& t){
        auto copy = std::move(operations);
        operations.clear();
        for(auto const& op:copy){
            const bool executed = op(t); //this might change the variable 'operations', but only inverse operations since direct operations will be executed instantly!
            if (!executed) operations.push_back(std::move(op));
        }
    }

private:


    using Operation = bool(T const&);
    using Operations = std::list<std::function<Operation>>;

    Operations operations;

};  //if (c(val)) {func(val); return;} // direct call if condition is met!s

}
}
