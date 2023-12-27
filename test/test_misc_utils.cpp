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

template <typename Base, typename, typename>
struct any_iface_impl : Base {
};

// NOLINTNEXTLINE
struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename>
struct foo_iface_impl : Base, tanuki::iface_impl_helper<Base, Holder> {
    void foo() const final
    {
        this->value().foo();
    }
};

// NOLINTNEXTLINE
struct foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename>
struct bar_iface_impl : Base, tanuki::iface_impl_helper<Base, Holder> {
    void bar() final
    {
        this->value().bar();
    }
};

// NOLINTNEXTLINE
struct bar_iface {
    virtual ~bar_iface() = default;
    virtual void bar() = 0;

    template <typename Base, typename Holder, typename T>
    using impl = bar_iface_impl<Base, Holder, T>;
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

struct foo_ref_iface {
    template <typename Wrap>
    struct impl {
        TANUKI_REF_IFACE_MEMFUN(foo)
    };
};

struct bar_ref_iface {
    template <typename Wrap>
    struct impl {
        TANUKI_REF_IFACE_MEMFUN(bar)
    };
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

    // Test the unwrapping in iface_impl_helper.
    {
        using wrap_foo_t = tanuki::wrap<foo_iface, tanuki::config<void, foo_ref_iface>{.pointer_interface = false}>;
        wrap_foo_t wf0{fooer{}}, wf0_ref{std::ref(wf0)}, wf0_cref{std::cref(wf0)};
        REQUIRE(!contains_reference(wf0));
        REQUIRE(contains_reference(wf0_ref));
        REQUIRE(contains_reference(wf0_cref));
        REQUIRE(static_cast<void *>(&value_ref<std::reference_wrapper<wrap_foo_t>>(wf0_ref).get())
                == static_cast<void *>(&wf0));
        REQUIRE(static_cast<const void *>(&value_ref<std::reference_wrapper<const wrap_foo_t>>(wf0_cref).get())
                == static_cast<const void *>(&wf0));
        REQUIRE_NOTHROW(wf0_ref.foo());
        REQUIRE_NOTHROW(wf0_cref.foo());
    }

    {
        using wrap_bar_t = tanuki::wrap<bar_iface, tanuki::config<void, bar_ref_iface>{.pointer_interface = false}>;
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
