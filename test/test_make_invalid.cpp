#include <array>
#include <string>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility,misc-use-internal-linkage,bugprone-throwing-static-initialization)

template <typename Base, typename>
struct any_iface_impl : Base {
};

// NOLINTNEXTLINE
struct any_iface {
    template <typename Base, typename T>
    using impl = any_iface_impl<Base, T>;
};

struct large {
    std::array<char, 100> buffer = {1, 2, 3};
    std::string str = "hello world                                                                            ";
};

TEST_CASE("invalid ctor/assignment")
{
    using wrap_t = tanuki::wrap<any_iface>;

    {
        auto w1 = wrap_t(tanuki::invalid_wrap);

        REQUIRE(is_invalid(w1));
        REQUIRE(has_dynamic_storage(w1));

        w1 = tanuki::invalid_wrap_t{};

        REQUIRE(is_invalid(w1));
        REQUIRE(has_dynamic_storage(w1));

        w1 = 123;

        REQUIRE(!is_invalid(w1));
        REQUIRE(has_static_storage(w1));

        w1 = tanuki::invalid_wrap_t{};

        REQUIRE(is_invalid(w1));
        REQUIRE(has_dynamic_storage(w1));
    }

    {
        auto w1 = wrap_t(tanuki::invalid_wrap_t{});

        REQUIRE(is_invalid(w1));
        REQUIRE(has_dynamic_storage(w1));

        w1 = tanuki::invalid_wrap_t{};

        REQUIRE(is_invalid(w1));
        REQUIRE(has_dynamic_storage(w1));

        w1 = large{};

        REQUIRE(!is_invalid(w1));
        REQUIRE(has_dynamic_storage(w1));

        w1 = tanuki::invalid_wrap_t{};

        REQUIRE(is_invalid(w1));
        REQUIRE(has_dynamic_storage(w1));
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility,misc-use-internal-linkage,bugprone-throwing-static-initialization)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
