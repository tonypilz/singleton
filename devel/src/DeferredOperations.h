#pragma once

#include <list>
#include <functional>

namespace global {

enum class DeferredOperationState{
    pending,
    finished
};

namespace detail {

template<typename TargetInstance>
class DeferredOperations {
public:

    template<typename Op >
    void addDeferredOperationWithArgBefore(Op func){
        operations.emplace_back(func);
    }

    template<typename Op >
    void addDeferredOperation(Op func){
        addDeferredOperationWithArgBefore(
                    [func](TargetInstance* /*before*/, TargetInstance* current) {
                        return func(current);
        });
    }

    template<typename Func >
    void ifAvailable(Func func){
        addDeferredOperation(
                    [func](TargetInstance* current) {
                        if (current==nullptr) return DeferredOperationState::pending;
                        func(*current);
                        return DeferredOperationState::finished;
        });
    }

    template<typename Func >
    void ifUnavailable(Func func){
        addDeferredOperation(
                    [func](TargetInstance* current) {
                        if (current!=nullptr) return DeferredOperationState::pending;
                        func();
                        return DeferredOperationState::finished;
        });
    }


    void conditionsChanged(TargetInstance* before, TargetInstance* current){ //while find
        auto copy = std::move(operations);
        operations.clear();
        for(auto const& op:copy){
             //op(t) might add new operations to 'operations', but only non with state pending since direct operations will be executed instantly!
            if (op(before,current)==DeferredOperationState::pending) operations.push_back(std::move(op));
        }

        if (copy.size()!=operations.size()) conditionsChanged(before,current);
    }

private:

    using Operation = DeferredOperationState(TargetInstance*,TargetInstance*); //todo remove const
    using Operations = std::list<std::function<Operation>>;
    Operations operations;

};

}
}
