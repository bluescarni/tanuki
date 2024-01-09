#include <concepts>
#include <type_traits>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename Base, typename Holder, typename T>
struct foo_iface_impl {
};

template <typename T>
concept fooable = requires(const T &x) { x.foo(); };

struct foo_iface;

template <typename Base, typename Holder, typename T>
    requires fooable<T> && std::derived_from<Base, foo_iface>
struct foo_iface_impl<Base, Holder, T> : public Base, tanuki::iface_impl_helper<Base, Holder> {
    void foo() const final
    {
        this->value().foo();
    }
};

// NOLINTNEXTLINE
struct foo_iface {
    virtual ~foo_iface() = default;
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
struct bar_iface_impl<Base, Holder, T> : public Base, tanuki::iface_impl_helper<Base, Holder> {
    void bar() const final
    {
        this->value().bar();
    }
};

// NOLINTNEXTLINE
struct bar_iface {
    virtual ~bar_iface() = default;
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

struct bar2_iface;

template <typename Base, typename Holder, typename T>
    requires barable<T> && std::derived_from<foo_iface_impl<Base, Holder, T>, bar2_iface>
struct bar2_iface_impl<Base, Holder, T> : foo_iface_impl<Base, Holder, T>,
                                          tanuki::iface_impl_helper<foo_iface_impl<Base, Holder, T>, Holder> {
    void bar() const final
    {
        this->value().bar();
    }
};

// NOLINTNEXTLINE
struct bar2_iface : foo_iface {
    virtual void bar() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = bar2_iface_impl<Base, Holder, T>;
};

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

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
