#include <functional>
#include <stdexcept>
#include <type_traits>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// LCOV_EXCL_START

template <typename, typename, typename>
struct foo_iface_impl {
};

template <typename T>
concept fooable = requires(T &x) { static_cast<void>(x.foo()); };

template <typename Base, typename Holder, typename T>
    requires fooable<tanuki::unwrap_cvref_t<T>>
struct foo_iface_impl<Base, Holder, T> : Base {
    void foo() final
    {
        static_cast<void>(getval<Holder>(this).foo());
    }
};

// NOLINTNEXTLINE
struct foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

struct f1 {
    void foo() {}
};

// LCOV_EXCL_STOP

template <typename T>
concept has_const_foo = requires(const T &x) { x.foo(); };

TEST_CASE("const ref access")
{
    using Catch::Matchers::MessageMatches;
    using Catch::Matchers::StartsWith;

    using wrap1_t = tanuki::wrap<foo_iface>;

    const f1 f;

    wrap1_t w(std::ref(f));

    REQUIRE_THROWS_MATCHES(w->foo(), std::runtime_error,
                           MessageMatches(StartsWith("Invalid access to a const reference of type '")));

    REQUIRE(!has_const_foo<wrap1_t>);
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
