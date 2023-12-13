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

template <typename, typename, typename>
struct foo_iface_impl {
};

// NOLINTNEXTLINE
struct foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

template <typename T>
concept fooable = requires(const T &x) {
    {
        x.foo()
    } -> std::same_as<void>;
};

template <typename Base, typename Holder, typename T>
    requires fooable<T>
struct foo_iface_impl<Base, Holder, T> : Base, tanuki::iface_impl_helper<Base, Holder> {
    void foo() const final
    {
        this->value().foo();
    }
};

using foo_wrap = tanuki::wrap<foo_iface>;

template <typename, typename, typename>
struct bar_iface_impl {
};

// NOLINTNEXTLINE
struct bar_iface {
    virtual ~bar_iface() = default;
    virtual void bar() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = bar_iface_impl<Base, Holder, T>;
};

template <typename T>
concept barable = requires(const T &x) {
    {
        x.bar()
    } -> std::same_as<void>;
};

template <typename Base, typename Holder, typename T>
    requires barable<T>
struct bar_iface_impl<Base, Holder, T> : Base, tanuki::iface_impl_helper<Base, Holder> {
    void bar() const final
    {
        this->value().bar();
    }
};

using bar_wrap = tanuki::wrap<bar_iface>;

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

using foobar_wrap = tanuki::wrap<tanuki::composite_iface<foo_iface, bar_iface>,
                                 tanuki::config<void, tanuki::composite_ref_iface<foo_ref_iface, bar_ref_iface>>{
                                     .pointer_interface = false}>;

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

TANUKI_S11N_WRAP_EXPORT(foobar_model, tanuki::composite_iface<foo_iface, bar_iface>)

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

template <typename, typename, typename, typename>
struct fooT_iface_impl {
};

template <typename U>
// NOLINTNEXTLINE
struct fooT_iface {
    virtual ~fooT_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = fooT_iface_impl<Base, Holder, T, U>;
};

template <typename Base, typename Holder, typename T, typename U>
    requires fooable<T>
struct fooT_iface_impl<Base, Holder, T, U> : Base, tanuki::iface_impl_helper<Base, Holder> {
    void foo() const final
    {
        this->value().foo();
    }
};

template <typename U>
using fooT_wrap = tanuki::wrap<fooT_iface<U>, tanuki::default_config>;

template <typename, typename, typename, typename>
struct barT_iface_impl {
};

template <typename U>
// NOLINTNEXTLINE
struct barT_iface {
    virtual ~barT_iface() = default;
    virtual void bar() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = barT_iface_impl<Base, Holder, T, U>;
};

template <typename Base, typename Holder, typename T, typename U>
    requires barable<T>
struct barT_iface_impl<Base, Holder, T, U> : Base, tanuki::iface_impl_helper<Base, Holder> {
    void bar() const final
    {
        this->value().bar();
    }
};

template <typename U>
using barT_wrap = tanuki::wrap<barT_iface<U>, tanuki::default_config>;

template <typename U>
struct fooT_ref_iface {
    template <typename Wrap>
    struct impl {
        TANUKI_REF_IFACE_MEMFUN(foo)
    };
};

template <typename U>
struct barT_ref_iface {
    template <typename Wrap>
    struct impl {
        TANUKI_REF_IFACE_MEMFUN(bar)
    };
};

template <typename U>
using foobarT_wrap
    = tanuki::wrap<tanuki::composite_iface<fooT_iface<U>, barT_iface<U>>,
                   tanuki::config<void, tanuki::composite_ref_iface<fooT_ref_iface<U>, barT_ref_iface<U>>>{
                       // Test passing a custom static size.
                       .static_size
                       = tanuki::holder_size<foobar_model, tanuki::composite_iface<fooT_iface<U>, barT_iface<U>>>,
                       .pointer_interface = false}>;

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(foobar_model, tanuki::composite_iface<fooT_iface<int>, barT_iface<int>>)

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
