#pragma once

#include <cstdlib> //for exit(1);
#include <exception>
#include <functional>
#include <list>
#include <utility>

namespace global {

namespace detail {

template <typename T> T &staticValue() {
  static T t;
  return t;
}

template <typename T> void throwImpl(T t) {

#ifdef __cpp_exceptions
  throw t;
#else  // __cpp_exceptions
  (void)t; // avoid unused
  exit(1);
#endif // __cpp_exceptions
}

} // namespace detail

class NullptrAccess : public std::exception {};
template <typename T = void> void onNullPtrAccess() {
  detail::throwImpl(NullptrAccess{});
}
// override by spcializing
// template<> void onNullPtrAccess<>(){ exit(1); }

namespace detail {

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
      throwImpl(bad_optional_access{});
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

  explicit operator T *() const { return operator->(); }

  T &operator*() const & { return *instancePtr; }
  T *operator->() const {
    if (instancePtr == nullptr)
      global::onNullPtrAccess<>();
    return instancePtr;
  }

  template <typename Func> void ifAvailable(Func func) {
    if (instancePtr != nullptr) {
      func(*instancePtr);
      return;
    }
    ifAvailableOps.emplace_back(func);
  }

  template <typename Func> void becomesUnavailable(Func func) {
    becomesUnavailableOps.emplace_back(func); // never directly
  }

private:
  InstancePointer &operator=(T *t) {
    if (instancePtr == t)
      return *this; // nothing changed
    auto before = instancePtr;
    instancePtr = t;

    if (instancePtr != nullptr) {
      for (auto const &op : ifAvailableOps)
        op(*instancePtr);
      ifAvailableOps.clear();
    }
    if (before != nullptr && instancePtr == nullptr) {
      for (auto const &op : becomesUnavailableOps)
        op(*before);
      becomesUnavailableOps.clear();
    }
    return *this;
  }

  template <typename> friend class ReplacingInstanceRegistration;

  using ClassType = InstancePointer<T>;

  InstancePointer(ClassType const &) = delete;
  ClassType const &operator=(ClassType const &) = delete;

  using DeferredOperation = std::list<std::function<void(T &)>>;
  DeferredOperation ifAvailableOps;
  DeferredOperation becomesUnavailableOps;

  T *instancePtr = nullptr;
};

} // namespace detail

template <typename T> detail::InstancePointer<T> &instance() {
  return detail::staticValue<detail::InstancePointer<T>>();
}
template <typename T> T &instanceRef() {
  return *detail::staticValue<detail::InstancePointer<T>>();
}

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
      throwImpl(InstanceReplacementNotAllowed{});
    if (t == nullptr)
      throwImpl(RegisteringNullNotAllowed{});

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
