#include <array>
#include <string>
#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

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

template <typename Wrap, typename T, typename... Args>
concept emplaceable = requires(Wrap &w, Args &&...args) { emplace<T>(w, std::forward<Args>(args)...); };

TEST_CASE("emplace")
{
    using wrap_t = tanuki::wrap<any_iface>;

    auto w1 = wrap_t(tanuki::invalid_wrap);

    REQUIRE(emplaceable<wrap_t, int, int>);
    REQUIRE(!emplaceable<wrap_t, std::string>);
    REQUIRE(!emplaceable<wrap_t, wrap_t, int>);

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
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
