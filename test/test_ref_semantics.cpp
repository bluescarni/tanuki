#include <concepts>
#include <functional>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

struct foo {
    int value = 42;
};

template <typename Base, typename Holder, typename T>
struct any_iface_impl : Base {
};

// NOLINTNEXTLINE
struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

TEST_CASE("basics")
{
    // Default init in the invalid state.
    using wrap1_t = tanuki::wrap<any_iface, tanuki::config<>{.invalid_default_ctor = true,
                                                             .semantics = tanuki::wrap_semantics::reference}>;

    REQUIRE(std::default_initializable<wrap1_t>);
    REQUIRE(noexcept(wrap1_t{}));

    const wrap1_t w1;
    REQUIRE(is_invalid(w1));

    // Default init into the default value type.
    using wrap2_t = tanuki::wrap<any_iface, tanuki::config<int>{.semantics = tanuki::wrap_semantics::reference}>;

    const wrap2_t w2;
    REQUIRE(!is_invalid(w2));
    REQUIRE(value_isa<int>(w2));
    REQUIRE(value_ref<int>(w2) == 0);

    // Generic ctor from a wrappable type.
    const foo f1{43};
    const wrap2_t w3(f1);
    REQUIRE(!is_invalid(w3));
    REQUIRE(value_isa<foo>(w3));
    REQUIRE(value_ref<foo>(w3).value == 43);

    // Generic ctor from reference wrapper.
    const wrap2_t w4(std::ref(f1));
    REQUIRE(!is_invalid(w4));
    REQUIRE(value_isa<std::reference_wrapper<const foo>>(w4));
    REQUIRE(value_ref<std::reference_wrapper<const foo>>(w4).get().value == 43);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
