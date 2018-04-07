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
    void addDeferredOperation(Op func){
        operations.emplace_back(func);
    }

    template<typename Func >
    void ifAvailable(Func func){
        operations.emplace_back(
                    [func](TargetInstance* t) {
                        if (t==nullptr) return DeferredOperationState::pending;
                        func(*t);
                        return DeferredOperationState::finished;
        });
    }

    template<typename Func >
    void ifUnavailable(Func func){
        operations.emplace_back(
                    [func](TargetInstance* t) {
                        if (t!=nullptr) return DeferredOperationState::pending;
                        func();
                        return DeferredOperationState::finished;
        });
    }


    void conditionsChanged(TargetInstance* t){ //while find
        auto copy = std::move(operations);
        operations.clear();
        for(auto const& op:copy){
             //op(t) might add new operations, but only non with state condition_not_met since direct operations will be executed instantly!
            if (op(t)==DeferredOperationState::pending) operations.push_back(std::move(op));
        }
    }

private:

    using Operation = DeferredOperationState(TargetInstance*); //todo remove const
    using Operations = std::list<std::function<Operation>>;
    Operations operations;

};

}
}
