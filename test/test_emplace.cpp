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

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename Base, typename, typename>
struct any_iface_impl : Base {
};

template <typename Base, typename Holder>
struct any_iface_impl<Base, Holder, std::string> {
};

// NOLINTNEXTLINE
struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

struct large {
    std::array<char, 100> buffer = {1, 2, 3};
    std::string str = "hello world                                                                            ";
};

struct thrower {
    explicit thrower(int)
    {
        throw std::invalid_argument("boo");
    }
};

template <typename Wrap, typename T, typename... Args>
concept emplaceable = requires(Wrap &w, Args &&...args) { emplace<T>(w, std::forward<Args>(args)...); };

TEST_CASE("emplace")
{
    using Catch::Matchers::Message;

    using wrap_t = tanuki::wrap<any_iface>;

    auto w1 = wrap_t(tanuki::invalid_wrap);

    REQUIRE(emplaceable<wrap_t, int, int>);
    REQUIRE(!emplaceable<wrap_t, std::string>);
    REQUIRE(emplaceable<wrap_t, wrap_t, int>);
    REQUIRE(emplaceable<wrap_t, wrap_t, wrap_t>);

    emplace<int>(w1, 43);

    REQUIRE(!is_invalid(w1));
    REQUIRE(value_ref<int>(w1) == 43);

    emplace<int>(w1, 43);

    REQUIRE(!is_invalid(w1));
    REQUIRE(value_ref<int>(w1) == 43);

    emplace<large>(w1);
    REQUIRE(!is_invalid(w1));
    REQUIRE(value_isa<large>(w1));

    emplace<int>(w1, 43);
    REQUIRE(!is_invalid(w1));
    REQUIRE(value_ref<int>(w1) == 43);

    // Revive via emplacement.
    w1 = tanuki::invalid_wrap;
    REQUIRE(is_invalid(w1));
    emplace<large>(w1);
    REQUIRE(!is_invalid(w1));
    REQUIRE(value_isa<large>(w1));

    // Try an emplacement that throws.
    emplace<large>(w1);

    REQUIRE_THROWS_MATCHES(emplace<thrower>(w1, 33), std::invalid_argument, Message("boo"));
    REQUIRE(is_invalid(w1));
    emplace<large>(w1);
    REQUIRE(!is_invalid(w1));
    REQUIRE(value_isa<large>(w1));

    // noexcept testing.
    REQUIRE(noexcept(emplace<int>(w1, 43)));
    REQUIRE(!noexcept(emplace<large>(w1)));
    REQUIRE(!noexcept(emplace<thrower>(w1, 33)));

    // Nested emplacement.
    emplace<wrap_t>(w1, 12);
    REQUIRE(value_isa<wrap_t>(w1));
    REQUIRE(value_ref<int>(value_ref<wrap_t>(w1)) == 12);

    auto w2 = wrap_t(tanuki::invalid_wrap);
    emplace<wrap_t>(w2, w1);
    REQUIRE(value_isa<wrap_t>(w2));
    REQUIRE(value_ref<int>(value_ref<wrap_t>(value_ref<wrap_t>(w2))) == 12);

    auto w3 = wrap_t(tanuki::invalid_wrap);
    emplace<wrap_t>(w3, 123.);
    REQUIRE(value_isa<wrap_t>(w3));
    REQUIRE(value_ref<double>(value_ref<wrap_t>(w3)) == 123.);

    auto w4 = wrap_t(12);
    emplace<wrap_t>(w4, tanuki::invalid_wrap);
    REQUIRE(is_invalid(value_ref<wrap_t>(w4)));
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
