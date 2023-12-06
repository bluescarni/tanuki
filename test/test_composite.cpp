#include <concepts>
#include <sstream>

#if defined(TANUKI_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#endif

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-virtual-inheritance,fuchsia-multiple-inheritance)

template <typename, typename>
struct foo_iface {
};

template <>
// NOLINTNEXTLINE
struct foo_iface<void, void> {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;
};

template <typename T>
concept fooable = requires(const T &x) {
    {
        x.foo()
    } -> std::same_as<void>;
};

template <typename Holder, typename T>
    requires fooable<T>
struct foo_iface<Holder, T> : virtual foo_iface<void, void>, tanuki::iface_impl_helper<Holder, T, foo_iface> {
    void foo() const final
    {
        this->value().foo();
    }
};

using foo_wrap = tanuki::wrap<foo_iface>;

template <typename, typename>
struct bar_iface {
};

template <>
// NOLINTNEXTLINE
struct bar_iface<void, void> {
    virtual ~bar_iface() = default;
    virtual void bar() const = 0;
};

template <typename T>
concept barable = requires(const T &x) {
    {
        x.bar()
    } -> std::same_as<void>;
};

template <typename Holder, typename T>
    requires barable<T>
struct bar_iface<Holder, T> : virtual bar_iface<void, void>, tanuki::iface_impl_helper<Holder, T, bar_iface> {
    void bar() const final
    {
        this->value().bar();
    }
};

using bar_wrap = tanuki::wrap<bar_iface>;

template <typename Wrap>
struct foobar_ref_iface {
    TANUKI_REF_IFACE_MEMFUN(foo)
    TANUKI_REF_IFACE_MEMFUN(bar)
};

using foobar_wrap = tanuki::wrap<tanuki::composite_wrap_interfaceT<foo_wrap, bar_wrap>::type,
                                 tanuki::config<void, foobar_ref_iface>{.pointer_interface = false}>;

struct foobar_model {
    mutable int n_foo = 0;
    mutable int n_bar = 0;

    void foo() const
    {
        ++n_foo;
    };
    void bar() const
    {
        ++n_bar;
    };

    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & n_foo;
        ar & n_bar;
    }
};

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(foobar_model, tanuki::composite_wrap_interfaceT<foo_wrap, bar_wrap>::type)

#endif

TEST_CASE("basic")
{
    foobar_wrap fb{foobar_model{}};
    fb.foo();
    fb.foo();
    fb.bar();

    REQUIRE(value_ref<foobar_model>(fb).n_foo == 2);
    REQUIRE(value_ref<foobar_model>(fb).n_bar == 1);

#if defined(TANUKI_WITH_BOOST_S11N)

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << fb;
    }

    fb = foobar_wrap{foobar_model{}};

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> fb;
    }

    REQUIRE(value_ref<foobar_model>(fb).n_foo == 2);
    REQUIRE(value_ref<foobar_model>(fb).n_bar == 1);

#endif
}

template <typename, typename, typename>
struct fooT_iface {
};

template <typename U>
// NOLINTNEXTLINE
struct fooT_iface<void, void, U> {
    virtual ~fooT_iface() = default;
    virtual void foo() const = 0;
};

template <typename Holder, typename T, typename U>
    requires fooable<T>
struct fooT_iface<Holder, T, U> : virtual fooT_iface<void, void, U>,
                                  tanuki::iface_impl_helper<Holder, T, fooT_iface, U> {
    void foo() const final
    {
        this->value().foo();
    }
};

template <typename U>
using fooT_wrap = tanuki::wrap<fooT_iface, tanuki::default_config, U>;

template <typename, typename, typename>
struct barT_iface {
};

template <typename U>
// NOLINTNEXTLINE
struct barT_iface<void, void, U> {
    virtual ~barT_iface() = default;
    virtual void bar() const = 0;
};

template <typename Holder, typename T, typename U>
    requires barable<T>
struct barT_iface<Holder, T, U> : virtual barT_iface<void, void, U>,
                                  tanuki::iface_impl_helper<Holder, T, barT_iface, U> {
    void bar() const final
    {
        this->value().bar();
    }
};

template <typename U>
using barT_wrap = tanuki::wrap<barT_iface, tanuki::default_config, U>;

template <typename Wrap, typename U>
struct foobarT_ref_iface_impl {
    TANUKI_REF_IFACE_MEMFUN(foo)
    TANUKI_REF_IFACE_MEMFUN(bar)
};

template <typename U>
struct foobarT_ref_iface {
    template <typename Wrap>
    using type = foobarT_ref_iface_impl<Wrap, U>;
};

template <typename U>
using foobarT_wrap
    = tanuki::wrap<tanuki::composite_wrap_interfaceT<fooT_wrap<U>, barT_wrap<U>>::template type,
                   tanuki::config<void, foobarT_ref_iface<U>::template type>{
                       // Test passing a custom static size.
                       .static_size = tanuki::holder_size<
                           foobar_model, tanuki::composite_wrap_interfaceT<fooT_wrap<U>, barT_wrap<U>>::template type>,
                       .pointer_interface = false}>;

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(foobar_model, tanuki::composite_wrap_interfaceT<fooT_wrap<int>, barT_wrap<int>>::type)

#endif

TEST_CASE("template")
{
    foobarT_wrap<int> fb{foobar_model{}};
    fb.foo();
    fb.foo();
    fb.bar();

    REQUIRE(value_ref<foobar_model>(fb).n_foo == 2);
    REQUIRE(value_ref<foobar_model>(fb).n_bar == 1);

#if defined(TANUKI_WITH_BOOST_S11N)

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << fb;
    }

    fb = foobarT_wrap<int>{foobar_model{}};

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> fb;
    }

    REQUIRE(value_ref<foobar_model>(fb).n_foo == 2);
    REQUIRE(value_ref<foobar_model>(fb).n_bar == 1);

#endif
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-virtual-inheritance,fuchsia-multiple-inheritance)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
