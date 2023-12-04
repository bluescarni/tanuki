#include <concepts>
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
struct foobar_iface;

template <>
// NOLINTNEXTLINE
struct foobar_iface<void, void> {
    virtual ~foobar_iface() = default;
    virtual void foo() const noexcept = 0;
    virtual void bar() = 0;
    virtual void fuzz() && = 0;
};

template <typename Holder, typename T>
struct foobar_iface : foobar_iface<void, void>, tanuki::iface_impl_helper<Holder, T, foobar_iface> {
    void foo() const noexcept final
    {
        this->value().foo();
    }
    void bar() noexcept final {}
    void fuzz() && final {}
};

struct fooer {
    void foo() const {}
};

template <typename Wrap>
struct foobar_ref_iface {
    // NOLINTNEXTLINE
    foobar_ref_iface() {}

    TANUKI_REF_IFACE_MEMFUN(foo)
    TANUKI_REF_IFACE_MEMFUN(bar)
    TANUKI_REF_IFACE_MEMFUN(fuzz)
};

TEST_CASE("ref_iface basics")
{
    using wrap1_t = tanuki::wrap<foobar_iface, tanuki::config<void, foobar_ref_iface>{.invalid_default_ctor = true}>;
    using wrap2_t = tanuki::wrap<foobar_iface, tanuki::config<fooer, foobar_ref_iface>{}>;

    // NOLINTNEXTLINE
    wrap1_t w1{fooer{}};

    REQUIRE(noexcept(std::as_const(w1).foo()));
    REQUIRE(!noexcept(w1.bar()));
    REQUIRE(!noexcept(std::move(w1).fuzz()));

    REQUIRE(!noexcept(wrap1_t{}));
    REQUIRE(!noexcept(wrap2_t{}));
    REQUIRE(!noexcept(wrap1_t{fooer{}}));
    REQUIRE(!noexcept(wrap1_t{tanuki::in_place<fooer>}));
}

template <typename, typename>
struct any_iface;

template <>
// NOLINTNEXTLINE
struct any_iface<void, void> {
    virtual ~any_iface() = default;
};

template <typename Holder, typename>
struct any_iface : any_iface<void, void> {
};

template <typename Wrap>
struct any_ref_iface {
    // NOLINTNEXTLINE
    any_ref_iface() = delete;

    TANUKI_REF_IFACE_MEMFUN(foo)
    TANUKI_REF_IFACE_MEMFUN(bar)
    TANUKI_REF_IFACE_MEMFUN(fuzz)
};

TEST_CASE("ref_iface noinit")
{
    using wrap1_t = tanuki::wrap<any_iface, tanuki::config<void, any_ref_iface>{.invalid_default_ctor = true}>;
    using wrap2_t = tanuki::wrap<any_iface, tanuki::config<fooer, any_ref_iface>{}>;

    REQUIRE(!std::constructible_from<wrap1_t>);
    REQUIRE(!std::constructible_from<wrap2_t>);
    REQUIRE(!std::constructible_from<wrap1_t, int>);
    REQUIRE(!std::constructible_from<wrap1_t, tanuki::in_place_type<int>>);
    REQUIRE(!std::is_copy_constructible_v<wrap1_t>);
    REQUIRE(!std::is_move_constructible_v<wrap1_t>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
