#include <array>
#include <stdexcept>
#include <string>
#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

template <typename Base, typename, typename>
struct any_iface_impl : Base {
};

// NOLINTNEXTLINE
struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

struct large {
    std::array<char, 100> buffer = {1, 2, 3};
    std::string str = "hello world                                                                            ";
};

struct small {
    std::string s = "42";
};

// NOLINTNEXTLINE
struct cm_throw {
    std::array<char, 100> buffer = {1, 2, 3};
    std::string str = "hello world                                                                            ";

    cm_throw() = default;
    cm_throw(const cm_throw &)
    {
        throw std::invalid_argument("copy");
    }
    // NOLINTNEXTLINE
    cm_throw(cm_throw &&)
    {
        throw std::invalid_argument("move");
    }
    // NOLINTNEXTLINE
    cm_throw &operator=(const cm_throw &)
    {
        throw std::invalid_argument("copy assignment");
    }
};

static void my_func1(int) {}
static void my_func2(int) {}

TEST_CASE("gen assignment")
{
    using Catch::Matchers::Message;

    // Test the noncv_rvalue_reference concept.
    // NOLINTNEXTLINE
    REQUIRE(tanuki::detail::noncv_rvalue_reference<int &&>);
    REQUIRE(!tanuki::detail::noncv_rvalue_reference<int &>);
    REQUIRE(!tanuki::detail::noncv_rvalue_reference<int>);
    REQUIRE(!tanuki::detail::noncv_rvalue_reference<const int &&>);
    REQUIRE(!tanuki::detail::noncv_rvalue_reference<volatile int &&>);
    REQUIRE(!tanuki::detail::noncv_rvalue_reference<const volatile int &&>);

    using wrap_t = tanuki::wrap<any_iface>;
    using wrap_ns_t = tanuki::wrap<any_iface, tanuki::config<>{.static_size = 0}>;

    // Revival of invalid object.
    {
        wrap_t w1(large{}), w2(std::move(w1));
        // NOLINTNEXTLINE
        REQUIRE(is_invalid(w1));
        w1 = large{.str = "foo"};
        REQUIRE(!is_invalid(w1));
        REQUIRE(value_ref<large>(w1).str == "foo");
    }

    // Different internal types.
    {
        wrap_t w1(large{});
        w1 = small{.s = "hello"};
        REQUIRE(value_ref<small>(w1).s == "hello");
    }

    // Different internal types with throwing constructor.
    {
        wrap_t w1(large{});
        REQUIRE_THROWS_MATCHES(w1 = cm_throw{}, std::invalid_argument, Message("move"));
        REQUIRE(is_invalid(w1));
    }
    {
        wrap_ns_t w1(large{});
        cm_throw cmt;
        REQUIRE_THROWS_MATCHES(w1 = std::as_const(cmt), std::invalid_argument, Message("copy"));
        REQUIRE(is_invalid(w1));
    }

    // Same internal types.
    {
        wrap_t w1(large{});
        w1 = large{.str = "foo"};
        REQUIRE(value_ref<large>(w1).str == "foo");
        large ol{.str = "bar"};
        w1 = ol;
        REQUIRE(value_ref<large>(w1).str == "bar");
    }
    {
        wrap_t w1(std::in_place_type<cm_throw>);
        cm_throw cmt;
        REQUIRE_THROWS_MATCHES(w1 = cmt, std::invalid_argument, Message("copy assignment"));
    }

    // Special handling for function types.
    {
        wrap_t w1(my_func1);
        w1 = my_func2;
        REQUIRE(value_ref<void (*)(int)>(w1) == &my_func2);
        (*value_ref<void (*)(int)>(w1))(3);
    }
    {
        wrap_t w1(&my_func1);
        w1 = &my_func2;
        REQUIRE(value_ref<void (*)(int)>(w1) == &my_func2);
        (*value_ref<void (*)(int)>(w1))(4);
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
