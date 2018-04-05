#pragma once

#include <list>
#include <functional>

namespace global {
namespace detail {


template<typename T>
class ConditionalSingleShotOperations {
public:

    template<typename Op>
    void add(Op op){
        operations.emplace_back(op);
    }


    void operator()(T const& t){
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

};

}
}
