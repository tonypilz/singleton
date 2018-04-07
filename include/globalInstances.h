#pragma once

#include <exception>
#include <functional>
#include <list>
#include <utility>

namespace global {

enum class DeferredOperationState { pending, finished };
namespace detail {

template <typename TargetInstance> class DeferredOperations {
public:
  template <typename Op> void addDeferredOperationWithArgBefore(Op func) {
    operations.emplace_back(func);
  }

  template <typename Op> void addDeferredOperation(Op func) {
    addDeferredOperationWithArgBefore(
        [func](TargetInstance * /*before*/, TargetInstance *current) {
          return func(current);
        });
  }

  template <typename Func> void ifAvailable(Func func) {
    addDeferredOperation([func](TargetInstance *current) {
      if (current == nullptr)
        return DeferredOperationState::pending;
      func(*current);
      return DeferredOperationState::finished;
    });
  }

  template <typename Func> void ifUnavailable(Func func) {
    addDeferredOperation([func](TargetInstance *current) {
      if (current != nullptr)
        return DeferredOperationState::pending;
      func();
      return DeferredOperationState::finished;
    });
  }

  void conditionsChanged(TargetInstance *before,
                         TargetInstance *current) { // while find
    auto copy = std::move(operations);
    operations.clear();
    for (auto const &op : copy) {
      // op(t) might add new operations to 'operations', but only non with state
      // pending since direct operations will be executed instantly!
      if (op(before, current) == DeferredOperationState::pending)
        operations.push_back(std::move(op));
    }

    if (copy.size() != operations.size())
      conditionsChanged(before, current);
  }

private:
  using Operation = DeferredOperationState(
      TargetInstance *, TargetInstance *); // todo remove const
  using Operations = std::list<std::function<Operation>>;
  Operations operations;
};

} // namespace detail

class NullptrAccess : public std::exception {};
struct NullptrAccessHandler {
  using type = std::function<void()>;
  type handler = []() { throw NullptrAccess(); };
};

namespace detail {

struct staticValueSubDefault {};

template <typename T, typename Sub = staticValueSubDefault> T &staticValue() {
  static T t;
  return t;
}

class bad_optional_access_impl : public std::exception {};

using bad_optional_access =
    bad_optional_access_impl; // use c++17 version if available

template <typename T> class OptionalValueImpl {

public:
  explicit OptionalValueImpl() {}

  OptionalValueImpl &operator=(T const &t) {
    val = t;
    m_hasValue = true;
    return *this;
  }

  explicit operator T() const {
    if (!m_hasValue)
      throw bad_optional_access();
    return val;
  }

  bool has_value() const { return m_hasValue; }
  void reset() { m_hasValue = false; }

private:
  T val;
  bool m_hasValue = false;
};

template <typename T>
using optional = OptionalValueImpl<T>; // use c++17 version if available

template <typename T> class InstancePointer {

public:
  explicit InstancePointer() {}

  operator bool() const { return instancePtr != nullptr; }

  bool operator==(T const *t) const { return instancePtr == t; }
  bool operator!=(T const *t) const { return instancePtr != t; }

  explicit operator const T *() const { return operator->(); }
  explicit operator T *() { return operator->(); }

  const T &operator*() const & { return *instancePtr; }
  T &operator*() & { return *instancePtr; }

  const T *operator->() const {
    return instancePtr != nullptr ? instancePtr : handleNull();
  }
  T *operator->() {
    return instancePtr != nullptr ? instancePtr : handleNull();
  }

  template <typename Op> void addDeferredOperationWithArgBefore(Op func) {
    deferredOperations.addDeferredOperationWithArgBefore(func);
    deferredOperations.conditionsChanged(instancePtr, instancePtr);
  }

  template <typename DeferredOperation>
  void addDeferredOperation(DeferredOperation op) {
    deferredOperations.addDeferredOperation(op);
    deferredOperations.conditionsChanged(instancePtr, instancePtr);
  }

  template <typename Func> void ifAvailable(Func func) {
    deferredOperations.ifAvailable(func);
    deferredOperations.conditionsChanged(instancePtr, instancePtr);
  }

  template <typename Func> void ifUnavailable(Func func) {
    deferredOperations.ifUnavailable(func);
    deferredOperations.conditionsChanged(instancePtr, instancePtr);
  }

  std::function<T *()> onNullPtrAccess;
  std::function<void()> onNullPtrAccessUntyped;

private:
  T *handleNull() const {
    if (onNullPtrAccess)
      return onNullPtrAccess();
    if (onNullPtrAccessUntyped)
      onNullPtrAccessUntyped(); // if this returns we execute global handler
    detail::staticValue<NullptrAccessHandler>()
        .handler(); // global handler should always be there
    return nullptr; // shouldnt be reached
  }

  InstancePointer &operator=(T *t) {
    if (instancePtr == t)
      return *this; // nothing changed
    auto before = instancePtr;
    instancePtr = t;
    deferredOperations.conditionsChanged(before, instancePtr);
    return *this;
  }

  template <typename, typename> friend class ReplacingInstanceRegistration;

  using ClassType = InstancePointer<T>;
  InstancePointer(ClassType const &) = delete;
  ClassType const &operator=(ClassType const &) = delete;

  detail::DeferredOperations<T> deferredOperations; // todo

  T *instancePtr = nullptr;
};

} // namespace detail

template <typename T, typename Sub = detail::staticValueSubDefault>
detail::InstancePointer<T> &instance() {
  return detail::staticValue<detail::InstancePointer<T>>();
}
template <typename T, typename Sub = detail::staticValueSubDefault>
T &instanceRef() {
  return *detail::staticValue<detail::InstancePointer<T>>();
}
inline NullptrAccessHandler::type &onNullptrAccess() {
  return detail::staticValue<NullptrAccessHandler>().handler;
} // global handler

class InstanceReplacementNotAllowed : public std::exception {};
class RegisteringNullNotAllowed : public std::exception {};
namespace detail {

// replaces existing for the time it exised
template <typename T, typename Sub = detail::staticValueSubDefault>
class ReplacingInstanceRegistration {

public:
  ReplacingInstanceRegistration() {}
  ReplacingInstanceRegistration(T *t) { registerInstance(t); }
  void operator()(T *t) { registerInstance(t); }
  virtual ~ReplacingInstanceRegistration() { deregisterInstance(); }

  virtual void registerInstance(T *t) {
    deregisterInstance();
    replacedInstance = instance<T, Sub>().instancePtr;
    instance<T, Sub>() = t; // possibly deregisters again
  }

  virtual void deregisterInstance() {
    if (replacedInstance.has_value() == false)
      return; // noting to do
    T *tmp = static_cast<T *>(replacedInstance);
    replacedInstance.reset();
    instance<T, Sub>() = tmp; // possibly registers again
  }

private:
  ReplacingInstanceRegistration(ReplacingInstanceRegistration const &) =
      delete; // no copy

  detail::optional<T *> replacedInstance;
};

// expects nullptr to be registered beforehand
// expects registration-target not to be null
template <typename T, typename Sub = detail::staticValueSubDefault>
class InstanceRegistration : ReplacingInstanceRegistration<T, Sub> {
public:
  using Superclass = ReplacingInstanceRegistration<T, Sub>;
  using Superclass::operator();

  InstanceRegistration() : Superclass() {}
  InstanceRegistration(T *t) : Superclass() { registerInstance(t); }

  void registerInstance(T *t) override {

    if (instance<T, Sub>() != nullptr)
      throw InstanceReplacementNotAllowed();
    if (t == nullptr)
      throw RegisteringNullNotAllowed();

    Superclass::registerInstance(t);
  }
};

template <template <typename, typename> class RegistrationType, typename R,
          typename Sub, typename T>
class RegisterdInstanceT {

  T t;
  RegistrationType<R, Sub> reg;

public:
  template <typename... Args>
  RegisterdInstanceT(Args &&... args)
      : t(std::forward<Args>(args)...), reg(&t) {}
};

} // namespace detail
template <typename R, typename T = R,
          typename Sub = detail::staticValueSubDefault>
using Instance =
    detail::RegisterdInstanceT<detail::InstanceRegistration, R, Sub, T>;
template <typename R, typename T = R,
          typename Sub = detail::staticValueSubDefault>
using TestInstance =
    detail::RegisterdInstanceT<detail::ReplacingInstanceRegistration, R, Sub,
                               T>;
template <typename R, typename Sub, typename T = R>
using SubInstance =
    detail::RegisterdInstanceT<detail::InstanceRegistration, R, Sub, T>;
template <typename R, typename Sub, typename T = R>
using SubTestInstance =
    detail::RegisterdInstanceT<detail::ReplacingInstanceRegistration, R, Sub,
                               T>;
#define GLOBAL_INSTANCE_IS_FRIEND                                              \
  template <template <typename, typename> class, typename, typename, typename> \
  friend class ::global::detail::RegisterdInstanceT

} // namespace global
