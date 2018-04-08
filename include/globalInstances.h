#pragma once

#include <cstdlib>
#include <exception>
#include <functional>
#include <list>
#include <utility>

namespace global {

#ifdef __cpp_exceptions
namespace detail {

template <typename T> T &do_throw(T t) { throw t; }

} // namespace detail
#else
namespace detail {

template <typename T> T &do_throw(T) { exit(1); }

} // namespace detail
#endif // __cpp_exceptions

enum class DeferredOperationState { pending, finished };
namespace detail {

template <typename TargetInstance> class DeferredOperations {
public:
  template <typename Op>
  void addDeferredOperationWithArgBefore(Op func, TargetInstance *current) {
    if (func(current, current) == DeferredOperationState::finished)
      return;
    operations.emplace_back(func);
  }

  template <typename Op>
  void addDeferredOperation(Op func, TargetInstance *current) {
    addDeferredOperationWithArgBefore(
        [func](TargetInstance * /*before*/, TargetInstance *current) {
          return func(current);
        },
        current);
  }

  template <typename Func>
  void ifAvailable(Func func, TargetInstance *current) {
    addDeferredOperation(
        [func](TargetInstance *current) {
          if (current == nullptr)
            return DeferredOperationState::pending;
          func(*current);
          return DeferredOperationState::finished;
        },
        current);
  }

  template <typename Func>
  void ifUnavailable(Func func, TargetInstance *current) {
    addDeferredOperation(
        [func](TargetInstance *current) {
          if (current != nullptr)
            return DeferredOperationState::pending;
          func();
          return DeferredOperationState::finished;
        },
        current);
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
  type handler = []() { detail::do_throw(NullptrAccess{}); };
};

namespace detail {

template <typename T> T &staticValue() {
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
      do_throw(bad_optional_access{});
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
    deferredOperations.addDeferredOperationWithArgBefore(func, instancePtr);
  }

  template <typename DeferredOperation>
  void addDeferredOperation(DeferredOperation op) {
    deferredOperations.addDeferredOperation(op, instancePtr);
  }

  template <typename Func> void ifAvailable(Func func) {
    deferredOperations.ifAvailable(func, instancePtr);
  }

  template <typename Func> void ifUnavailable(Func func) {
    deferredOperations.ifUnavailable(func, instancePtr);
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

  template <typename> friend class ReplacingInstanceRegistration;

  using ClassType = InstancePointer<T>;
  InstancePointer(ClassType const &) = delete;
  ClassType const &operator=(ClassType const &) = delete;

  detail::DeferredOperations<T> deferredOperations; // todo

  T *instancePtr = nullptr;
};

} // namespace detail

template <typename T> detail::InstancePointer<T> &instance() {
  return detail::staticValue<detail::InstancePointer<T>>();
}
template <typename T> T &instanceRef() {
  return *detail::staticValue<detail::InstancePointer<T>>();
}
inline NullptrAccessHandler::type &onNullptrAccess() {
  return detail::staticValue<NullptrAccessHandler>().handler;
} // global handler

class InstanceReplacementNotAllowed : public std::exception {};
class RegisteringNullNotAllowed : public std::exception {};
namespace detail {

// replaces existing for the time it exised
template <typename T> class ReplacingInstanceRegistration {

public:
  ReplacingInstanceRegistration() {}
  ReplacingInstanceRegistration(T *t) { registerInstance(t); }
  void operator()(T *t) { registerInstance(t); }
  virtual ~ReplacingInstanceRegistration() { deregisterInstance(); }

  virtual void registerInstance(T *t) {
    deregisterInstance();
    replacedInstance = instance<T>().instancePtr;
    instance<T>() = t; // possibly deregisters again
  }

  virtual void deregisterInstance() {
    if (replacedInstance.has_value() == false)
      return; // noting to do
    T *tmp = static_cast<T *>(replacedInstance);
    replacedInstance.reset();
    instance<T>() = tmp; // possibly registers again
  }

private:
  ReplacingInstanceRegistration(ReplacingInstanceRegistration const &) =
      delete; // no copy

  detail::optional<T *> replacedInstance;
};

// expects nullptr to be registered beforehand
// expects registration-target not to be null
template <typename T>
class InstanceRegistration : ReplacingInstanceRegistration<T> {
public:
  using Superclass = ReplacingInstanceRegistration<T>;
  using Superclass::operator();

  InstanceRegistration() : Superclass() {}
  InstanceRegistration(T *t) : Superclass() { registerInstance(t); }

  void registerInstance(T *t) override {

    if (instance<T>() != nullptr)
      do_throw(InstanceReplacementNotAllowed{});
    if (t == nullptr)
      do_throw(RegisteringNullNotAllowed{});

    Superclass::registerInstance(t);
  }
};

template <template <typename> class RegistrationType, typename AccessType,
          typename InstanceType>
class RegisterdInstanceT {

  InstanceType t;
  RegistrationType<AccessType> reg;

public:
  template <typename... Args>
  RegisterdInstanceT(Args &&... args)
      : t(std::forward<Args>(args)...), reg(&t) {}
};

} // namespace detail
template <typename AccessType, typename InstanceType = AccessType>
using Instance = detail::RegisterdInstanceT<detail::InstanceRegistration,
                                            AccessType, InstanceType>;
template <typename AccessType, typename InstanceType = AccessType>
using TestInstance =
    detail::RegisterdInstanceT<detail::ReplacingInstanceRegistration,
                               AccessType, InstanceType>;
#define GLOBAL_INSTANCE_IS_FRIEND                                              \
  template <template <typename, typename> class, typename, typename>           \
  friend class ::global::detail::RegisterdInstanceT

} // namespace global
