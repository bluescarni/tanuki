#include <array>
#include <string>
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename, typename>
struct any_iface;

template <>
// NOLINTNEXTLINE
struct any_iface<void, void> {
    virtual ~any_iface() = default;
};

template <typename Holder, typename T>
struct any_iface : any_iface<void, void> {
};

struct large {
    std::array<char, 100> buffer = {1, 2, 3};
    std::string str = "hello world                                                                            ";
};

struct small {
    std::string s = "42";
};

TEST_CASE("swap")
{
    using std::swap;

    using wrap_t = tanuki::wrap<any_iface>;

    // NOLINTNEXTLINE
    REQUIRE(std::is_nothrow_swappable_v<wrap_t>);

    // Self swap.
    {
        wrap_t w{3};

        REQUIRE_NOTHROW(swap(w, w));
    }

    // Swap invalid values.
    {
        wrap_t w1{large{}}, w1a(std::move(w1));
        wrap_t w2{large{}}, w2a(std::move(w2));

        swap(w1, w2);
    }

    // Revive via swapping.
    {
        wrap_t w1{large{}}, w1a(std::move(w1));
        wrap_t w2{large{.str = "blif"}};

        REQUIRE(is_invalid(w1));

        swap(w1, w2);

        REQUIRE(!is_invalid(w1));
        REQUIRE(value_ref<large>(w1).str == "blif");
    }

    {
        wrap_t w2{large{}}, w2a(std::move(w2));
        wrap_t w1{large{.str = "blif"}};

        REQUIRE(is_invalid(w2));

        swap(w1, w2);

        REQUIRE(!is_invalid(w2));
        REQUIRE(value_ref<large>(w2).str == "blif");
    }

    // Swap different types.
    {
        wrap_t w1{small{.s = "blif"}}, w2{large{.str = "blaf"}};
        swap(w1, w2);
        REQUIRE(value_ref<large>(w1).str == "blaf");
        REQUIRE(value_ref<small>(w2).s == "blif");
    }

    // Swap same types.
    {
        wrap_t w1{small{.s = "blif"}}, w2{small{.s = "blaf"}};
        swap(w1, w2);
        REQUIRE(value_ref<small>(w1).s == "blaf");
        REQUIRE(value_ref<small>(w2).s == "blif");
    }
    {
        wrap_t w1{large{.str = "blif"}}, w2{large{.str = "blaf"}};
        const auto *p1 = value_ptr<large>(w1);
        const auto *p2 = value_ptr<large>(w2);
        swap(w1, w2);
        REQUIRE(value_ref<large>(w1).str == "blaf");
        REQUIRE(value_ref<large>(w2).str == "blif");
        REQUIRE(value_ptr<large>(w1) == p2);
        REQUIRE(value_ptr<large>(w2) == p1);
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
