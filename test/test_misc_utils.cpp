#include <array>
#include <functional>
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

template <typename>
struct foo_iface;

template <>
// NOLINTNEXTLINE
struct foo_iface<void> {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;
};

template <typename Holder>
struct foo_iface : foo_iface<void>, tanuki::iface_impl_helper<Holder> {
    void foo() const final
    {
        this->value().foo();
    }
};

template <typename>
struct bar_iface;

template <>
// NOLINTNEXTLINE
struct bar_iface<void> {
    virtual ~bar_iface() = default;
    virtual void bar() = 0;
};

template <typename Holder>
struct bar_iface : bar_iface<void>, tanuki::iface_impl_helper<Holder> {
    void bar() final
    {
        this->value().bar();
    }
};

struct large {
    std::array<char, 100> buffer = {1, 2, 3};
    std::string str = "hello world                                                                            ";
};

struct fooer {
    void foo() const {}
};

struct barer {
    void bar() {}
};

namespace tanuki
{

template <typename Wrap>
struct ref_iface<Wrap, foo_iface> {
    TANUKI_REF_IFACE_MEMFUN(foo)
};

template <typename Wrap>
struct ref_iface<Wrap, bar_iface> {
    TANUKI_REF_IFACE_MEMFUN(bar)
};

} // namespace tanuki

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

    // Test the unwrapping in iface_impl_helper.
    {
        using wrap_foo_t = tanuki::wrap<foo_iface, tanuki::config<>{.pointer_interface = false}>;
        wrap_foo_t wf0{fooer{}}, wf0_ref{std::ref(wf0)}, wf0_cref{std::cref(wf0)};
        REQUIRE(static_cast<void *>(&value_ref<std::reference_wrapper<wrap_foo_t>>(wf0_ref).get())
                == static_cast<void *>(&wf0));
        REQUIRE(static_cast<const void *>(&value_ref<std::reference_wrapper<const wrap_foo_t>>(wf0_cref).get())
                == static_cast<const void *>(&wf0));
        REQUIRE_NOTHROW(wf0_ref.foo());
        REQUIRE_NOTHROW(wf0_cref.foo());
    }

    {
        using wrap_bar_t = tanuki::wrap<bar_iface, tanuki::config<>{.pointer_interface = false}>;
        wrap_bar_t wf0{barer{}}, wf0_ref{std::ref(wf0)};
        REQUIRE(static_cast<void *>(&value_ref<std::reference_wrapper<wrap_bar_t>>(wf0_ref).get())
                == static_cast<void *>(&wf0));
        REQUIRE_NOTHROW(wf0_ref.bar());
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif