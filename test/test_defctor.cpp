#include <concepts>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

struct foo {
};

struct bar {
    bar() = delete;
};

template <typename, typename, typename>
struct any_iface_impl {
};

// NOLINTNEXTLINE
struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename T>
    requires(!std::same_as<T, foo>)
struct any_iface_impl<Base, Holder, T> : Base {
};

TEST_CASE("def invalid")
{
    using wrap1_t = tanuki::wrap<any_iface, tanuki::config<>{.invalid_default_ctor = true}>;

    REQUIRE(std::default_initializable<wrap1_t>);
    REQUIRE(noexcept(wrap1_t{}));

    const wrap1_t w1;
    REQUIRE(is_invalid(w1));

    using wrap2_t = tanuki::wrap<any_iface, tanuki::config<>{.static_size = 0, .invalid_default_ctor = true}>;

    REQUIRE(std::default_initializable<wrap2_t>);
    REQUIRE(noexcept(wrap2_t{}));

    const wrap1_t w2;
    REQUIRE(is_invalid(w2));
}

TEST_CASE("def value type")
{
    using wrap1_t = tanuki::wrap<any_iface, tanuki::config<int>{}>;

    REQUIRE(std::default_initializable<wrap1_t>);
    REQUIRE(noexcept(wrap1_t{}));

    wrap1_t w1;
    REQUIRE(!is_invalid(w1));
    REQUIRE(value_ref<int>(w1) == 0);

    using wrap2_t = tanuki::wrap<any_iface, tanuki::config<foo>{}>;

    REQUIRE(!std::default_initializable<wrap2_t>);

    using wrap3_t = tanuki::wrap<any_iface, tanuki::config<int>{.invalid_default_ctor = true}>;

    REQUIRE(std::default_initializable<wrap3_t>);
    REQUIRE(noexcept(wrap3_t{}));

    const wrap3_t w3;
    REQUIRE(is_invalid(w3));

    using wrap4_t = tanuki::wrap<any_iface, tanuki::config<bar>{}>;

    REQUIRE(!std::default_initializable<wrap4_t>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
