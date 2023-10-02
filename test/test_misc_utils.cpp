#include <array>
#include <string>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename>
struct any_iface;

template <>
// NOLINTNEXTLINE
struct any_iface<void> {
    virtual ~any_iface() = default;
};

template <typename Holder>
struct any_iface : any_iface<void> {
};

struct large {
    std::array<char, 100> buffer = {1, 2, 3};
    std::string str = "hello world                                                                            ";
};

TEST_CASE("misc utils")
{
    using wrap1_t = tanuki::wrap<any_iface, tanuki::config<>{.invalid_default_ctor = true}>;
    using wrap2_t = tanuki::wrap<any_iface>;

    REQUIRE(tanuki::any_wrap<wrap1_t>);
    REQUIRE(tanuki::any_wrap<wrap2_t>);
    REQUIRE(!tanuki::any_wrap<int>);

    using wrap3_t
        = tanuki::wrap<any_iface, tanuki::config<>{.static_size = tanuki::holder_size<large, any_iface>,
                                                   .static_alignment = tanuki::holder_align<large, any_iface>}>;

    const wrap3_t w(large{});

    REQUIRE(has_static_storage(w));
    REQUIRE(!has_dynamic_storage(w));
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
