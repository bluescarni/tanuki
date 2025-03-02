#include <concepts>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

template <typename Base, typename Holder, typename T>
struct foo_iface_impl {
};

template <typename T>
concept fooable = requires(const T &x) { x.foo(); };

struct foo_iface;

template <typename Base, typename Holder, typename T>
    requires fooable<T> && std::derived_from<Base, foo_iface>
struct foo_iface_impl<Base, Holder, T> : public Base {
    void foo() const final
    {
        getval<Holder>(this).foo();
    }
};

// NOLINTNEXTLINE
struct foo_iface {
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename T>
struct bar_iface_impl {
};

template <typename T>
concept barable = requires(const T &x) { x.bar(); };

struct bar_iface;

template <typename Base, typename Holder, typename T>
    requires barable<T> && std::derived_from<Base, bar_iface>
struct bar_iface_impl<Base, Holder, T> : public Base {
    void bar() const final
    {
        getval<Holder>(this).bar();
    }
};

// NOLINTNEXTLINE
struct bar_iface {
    virtual void bar() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = bar_iface_impl<Base, Holder, T>;
};

struct foobar_model {
    void foo() const {}
    void bar() const {}
};

struct foo_model {
    void foo() const {}
};

struct bar_model {
    void bar() const {}
};

template <typename Base, typename Holder, typename T>
struct bar2_iface_impl {
};

// NOLINTNEXTLINE(cppcoreguidelines-virtual-class-destructor)
struct bar2_iface;

template <typename Base, typename Holder, typename T>
    requires barable<T> && std::derived_from<foo_iface_impl<Base, Holder, T>, bar2_iface>
struct bar2_iface_impl<Base, Holder, T> : foo_iface_impl<Base, Holder, T> {
    void bar() const final
    {
        getval<Holder>(this).bar();
    }
};

// NOLINTNEXTLINE
struct bar2_iface : foo_iface {
    virtual void bar() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = bar2_iface_impl<Base, Holder, T>;
};

// NOTE: this test is meant to check that
// for graceful failures (as opposed to hard
// compile errors) when only part of a composite
// interface is implemented by a type.
TEST_CASE("invalid composite")
{
    using wrap_t = tanuki::wrap<tanuki::composite_iface<foo_iface, bar_iface>>;

    REQUIRE(!std::constructible_from<wrap_t, foo_model>);
    REQUIRE(!std::constructible_from<wrap_t, bar_model>);
    REQUIRE(std::constructible_from<wrap_t, foobar_model>);

    using wrap2_t = tanuki::wrap<bar2_iface>;

    REQUIRE(!std::constructible_from<wrap2_t, foo_model>);
    REQUIRE(!std::constructible_from<wrap2_t, bar_model>);
    REQUIRE(std::constructible_from<wrap2_t, foobar_model>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)
