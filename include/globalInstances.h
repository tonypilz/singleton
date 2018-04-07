#pragma once

#include <exception>
#include <functional>
#include <list>
#include <utility>

namespace global {

namespace detail {

template <typename T> class ConditionalSingleShotOperations {
public:
  template <typename Op> void add(Op op) { operations.emplace_back(op); }

  void operator()(T const &t) {
    auto copy = std::move(operations);
    operations.clear();
    for (auto const &op : copy) {
      const bool executed = op(t); // this might change the variable
                                   // 'operations', but only inverse operations
                                   // since direct operations will be executed
                                   // instantly!
      if (!executed)
        operations.push_back(std::move(op));
    }
  }

private:
  using Operation = bool(T const &);
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

class bad_optional_access
 : public std::exception {};

template <typename T> class OptionalValue {

public:
  explicit OptionalValue() {}

  OptionalValue &operator=(T const &t) {
    val = t;
    isSet = true;
    return *this;
  }

  explicit operator T() const {
    if (!isSet)
      throw bad_optional_access
();
    return val;
  }

  bool isValueSet() const { return isSet; }
  void unsetValue() { isSet = false; }

private:
  T val;
  bool isSet = false;
};

class UnexpectedNonNullInstance : public std::exception {};

template <typename Ptr> class InstancePointer {

public:
  using ValueType = Ptr;
  using Classtype = InstancePointer<Ptr>;

  using NullPtrAccessHandler = std::function<Ptr()>;
  using ValueChanged = std::function<void(Ptr const &)>;

  explicit InstancePointer() {}

  bool operator==(Ptr const &t) const { return val == t; }
  bool operator!=(Ptr const &t) const { return val != t; }

  operator bool() { return val != nullptr; }
  operator Ptr() { return operator->(); }

  Ptr operator->() const {
    if (val == nullptr) {
      if (onNullPtrAccess)
        return onNullPtrAccess();
      detail::staticValue<NullptrAccessHandler>()
          .handler(); // global handler is installed by default
      return nullptr;
    }
    return val;
  }

  Ptr rawPtr() const { return val; }

  template <typename Cond, typename Func>
  void ifAvailabilityChanged(Cond c, Func func) {
    if (c(val)) {
      func(val);
      return;
    } // direct call if condition is met!s
    changeOperations.add([c, func](Ptr const &t) {
      if (c(t)) {
        func(t);
        return true;
      }
      return false;
    });
  }

  template <typename Func> void ifAvailable(Func func) {
    auto notNull = [](Ptr const &t) { return t != nullptr; };
    auto pfunc = [func](Ptr const &t) {
      if (t == nullptr)
        throw NullptrAccess();
      func(*t);
    };
    ifAvailabilityChanged(notNull, pfunc);
  }

  template <typename Func> void ifUnavailable(Func func) {
    auto null = [](Ptr const &t) { return t == nullptr; };
    auto pfunc = [func](Ptr const &t) {
      if (t != nullptr)
        throw UnexpectedNonNullInstance();
      func();
    };
    ifAvailabilityChanged(null, pfunc);
  }

  NullPtrAccessHandler onNullPtrAccess;

private:
  InstancePointer &operator=(Ptr const &t) {
    if (val == t)
      return *this; // nothing changed
    val = t;
    changeOperations(val);
    return *this;
  }

  template <typename, typename> friend class ReplacingInstanceRegistration;

  InstancePointer(InstancePointer<Ptr> const &) = delete;
  InstancePointer<Ptr> const &operator=(InstancePointer<Ptr> const &) = delete;

  detail::ConditionalSingleShotOperations<Ptr> changeOperations;

  Ptr val = nullptr;
};

} // namespace detail

template <typename T, typename Sub = detail::staticValueSubDefault>
detail::InstancePointer<T *> &instance() {
  return detail::staticValue<detail::InstancePointer<T *>>();
}
template <typename T, typename Sub = detail::staticValueSubDefault>
T &instanceRef() {
  return *static_cast<T *>(detail::staticValue<detail::InstancePointer<T *>>());
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
    replacedInstance = instance<T, Sub>().rawPtr();
    instance<T, Sub>() = t; // possibly deregisters again
  }

  virtual void deregisterInstance() {
    if (replacedInstance.isValueSet() == false)
      return; // noting to do
    T *tmp = static_cast<T *>(replacedInstance);
    replacedInstance.unsetValue();
    instance<T, Sub>() = tmp; // possibly registers again
  }

private:
  ReplacingInstanceRegistration(ReplacingInstanceRegistration const &) =
      delete; // no copy

  detail::OptionalValue<T *> replacedInstance;
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
