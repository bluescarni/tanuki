#include <concepts>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

namespace ns
{

// NOLINTNEXTLINE
struct my_iface {
    virtual ~my_iface() = default;
    [[nodiscard]] virtual int foo() const = 0;
};

// Also include an interface with an impl template
// to check the non-intrusive representation takes
// the precedence.
// NOLINTNEXTLINE
struct my_iface2 {
    virtual ~my_iface2() = default;
    [[nodiscard]] virtual int bar() const = 0;

    template <typename Base, typename Holder, typename T>
    struct impl {
    };
};

} // namespace ns

namespace tanuki
{

// Empty default nonintrusive implementation.
template <typename Base, typename Holder, typename T>
struct iface_impl<ns::my_iface, Base, Holder, T> {
};

// Specialisation for int value type.
template <typename Base, typename Holder>
struct iface_impl<ns::my_iface, Base, Holder, int> : public Base {
    [[nodiscard]] int foo() const final
    {
        return 42;
    }
};

template <typename Base, typename Holder, typename T>
struct iface_impl<ns::my_iface2, Base, Holder, T> {
};

// Specialisation for int value type.
template <typename Base, typename Holder>
struct iface_impl<ns::my_iface2, Base, Holder, int> : public Base {
    [[nodiscard]] int bar() const final
    {
        return 43;
    }
};

} // namespace tanuki

TEST_CASE("basics")
{
    using wrap_t = tanuki::wrap<ns::my_iface>;

    wrap_t w(123);
    REQUIRE(w->foo() == 42);

    REQUIRE(!std::constructible_from<wrap_t, long>);

    using wrap2_t = tanuki::wrap<ns::my_iface2>;

    wrap2_t w2(123);
    REQUIRE(w2->bar() == 43);

    REQUIRE(!std::constructible_from<wrap2_t, long>);

    // Composite interface.
    using wrap3_t = tanuki::wrap<tanuki::composite_iface<ns::my_iface, ns::my_iface2>>;

    wrap3_t w3(123);
    REQUIRE(w3->foo() == 42);
    REQUIRE(w3->bar() == 43);

    REQUIRE(!std::constructible_from<wrap3_t, long>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)
