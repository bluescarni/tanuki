#include <concepts>
#include <functional>
#include <type_traits>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename T, typename U>
struct is_reference_wrapper_for : std::false_type {
};

template <typename T, typename U>
struct is_reference_wrapper_for<std::reference_wrapper<T>, U>
    : std::bool_constant<std::same_as<std::remove_cv_t<T>, U>> {
};

template <typename T, typename U>
concept ref_wrapper_for = is_reference_wrapper_for<T, U>::value;

template <typename, typename>
struct foo_iface;

template <>
// NOLINTNEXTLINE
struct foo_iface<void, void> {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;
};

template <typename, typename>
struct bar_iface;

template <>
// NOLINTNEXTLINE
struct bar_iface<void, void> {
    virtual ~bar_iface() = default;
    virtual void bar() const = 0;
};

using foo_wrap = tanuki::wrap<foo_iface, tanuki::config<>{.explicit_value_ctor = false}>;
using bar_wrap = tanuki::wrap<foo_iface, tanuki::config<>{.explicit_value_ctor = false}>;

struct my_class {
    void foo() const {}
    void bar() const {}
};

template <typename Holder, typename T>
    requires std::same_as<T, my_class> || ref_wrapper_for<T, my_class>
struct foo_iface<Holder, T> : foo_iface<void, void>, tanuki::iface_impl_helper<Holder, T> {
    void foo() const final
    {
        this->value().foo();
    }
};

template <typename Holder, typename T>
    requires std::same_as<T, my_class> || ref_wrapper_for<T, my_class>
struct bar_iface<Holder, T> : bar_iface<void, void>, tanuki::iface_impl_helper<Holder, my_class> {
    void bar() const final
    {
        this->value().bar();
    }
};

auto foo_func(const foo_wrap &w)
{
    return w;
}

auto bar_func(const bar_wrap &w)
{
    return w;
}

TEST_CASE("trait like")
{
    my_class c;

    REQUIRE(&value_ref<my_class>(foo_func(c)) != &c);
    REQUIRE(&value_ref<my_class>(bar_func(c)) != &c);

    REQUIRE(&value_ref<std::reference_wrapper<const my_class>>(foo_func(std::cref(c))).get() == &c);
    REQUIRE(&value_ref<std::reference_wrapper<const my_class>>(bar_func(std::cref(c))).get() == &c);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
