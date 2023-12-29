#include <cstddef>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// LCOV_EXCL_START

template <typename Base, typename, typename>
struct any_iface_impl : public Base {
};

// NOLINTNEXTLINE
struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

struct over {
    alignas(alignof(std::max_align_t) * 2u) std::byte value{};
};

// LCOV_EXCL_STOP

TEST_CASE("overaligned")
{
    using wrap_t = tanuki::wrap<any_iface>;

    wrap_t w1(123);
    REQUIRE(has_static_storage(w1));

    wrap_t w2(over{});
    REQUIRE(has_dynamic_storage(w2));

    // NOLINTNEXTLINE
    const auto w3 = w2;
    REQUIRE(has_dynamic_storage(w3));
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
