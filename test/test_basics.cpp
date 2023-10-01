#include <array>
#include <concepts>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

#if defined(TANUKI_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/string.hpp>

#endif

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wself-move"

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

    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & buffer;
        ar & str;
    }
};

struct small {
    std::string s = "42";

    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & s;
    }
};

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(large, any_iface)
TANUKI_S11N_WRAP_EXPORT(small, any_iface)

#endif

struct nonwrappable {
};

struct copythrow {
    copythrow() = default;
    // NOLINTNEXTLINE
    copythrow(const copythrow &){};
    copythrow(copythrow &&) noexcept = delete;
    copythrow &operator=(const copythrow &) = delete;
    copythrow &operator=(copythrow &&) noexcept = delete;
    ~copythrow() = default;
};

namespace tanuki
{

template <template <typename, typename...> typename IFaceT, typename... Args>
inline constexpr bool is_wrappable<nonwrappable, IFaceT, Args...> = false;

} // namespace tanuki

TEST_CASE("basics")
{
    using tanuki::wrap;
    using wrap_t = wrap<any_iface>;

    // A few simple initialisations from values.
    wrap_t w1(3.), w2(large{}), w3(std::function<void()>{});
    REQUIRE(value_type_index(w1) == typeid(double));
    REQUIRE(*value_ptr<double>(w1) == 3.);
    REQUIRE(value_isa<large>(w2));
    value_ref<large>(w2).buffer[0] = 2;
    REQUIRE(value_ptr<large>(std::as_const(w2))->buffer[0] == 2);
    REQUIRE(value_type_index(w3) == typeid(std::function<void()>));
    REQUIRE(!(*value_ptr<std::function<void()>>(w3)));

    // Default ctor disabled by default.
    REQUIRE(!std::default_initializable<wrap_t>);

    // noexcept handling for the value ctor.
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, int>);
    REQUIRE(std::is_constructible_v<wrap_t, const copythrow &>);
    REQUIRE(!std::is_nothrow_constructible_v<wrap_t, const copythrow &>);

    // noexcept handling for the emplace ctor.
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, tanuki::emplace_type<int>, int>);
    REQUIRE(std::is_constructible_v<wrap_t, tanuki::emplace_type<copythrow>, const copythrow &>);
    REQUIRE(!std::is_nothrow_constructible_v<wrap_t, tanuki::emplace_type<copythrow>, const copythrow &>);

    // Value ctor explicit by default.
    REQUIRE(!std::is_convertible_v<int, wrap_t>);

    // Value ctor disabled for non wrappable type.
    REQUIRE(!std::is_constructible_v<wrap_t, nonwrappable>);

    // Copy constructing.
    // Small one.
    auto w1copy = w1;
    REQUIRE(value_ref<double>(w1copy) == 3.);
    // Large one.
    auto w2copy = w2;
    REQUIRE(value_ref<large>(w2copy).buffer[0] == 2);

    // Move constructing.
    // Small one.
    auto w1move = std::move(w1);
    REQUIRE(value_ref<double>(w1move) == 3.);
    // NOLINTBEGIN
    REQUIRE(!is_invalid(w1));
    // NOLINTEND
    // Large one.
    auto w2move = std::move(w2);
    REQUIRE(value_ref<large>(w2move).buffer[0] == 2);
    // NOLINTBEGIN
    REQUIRE(is_invalid(w2));
    // NOLINTEND

    // Emplace test with class which is not copyable/movable.
    const wrap_t w_mut(tanuki::emplace<std::mutex>);
    REQUIRE(noexcept(wrap_t(tanuki::emplace<int>)));

    auto w4 = w3;

    // NOLINTBEGIN
    // auto w5(std::move(w2));
    // REQUIRE(is_invalid(w2));
    //  NOLINTEND

    w1 = std::move(w3);

    w3 = wrap_t(std::function<void()>{});

    auto w1a = w1;
    w1 = w1a;

    // auto w5a = wrap_t(large{});
    // w5a = std::move(w5);

    w2 = wrap_t(large{});
    auto w2a = wrap_t(large{});
    w2 = w2a;
}

TEST_CASE("assignment")
{
    using tanuki::wrap;
    using wrap_t = wrap<any_iface>;

    wrap_t w(42);

    // Self assign, copy and move.
    REQUIRE_NOTHROW(w = *&w);
    // NOLINTNEXTLINE
    REQUIRE_NOTHROW(w = std::move(*&w));

    // Object revival via copy assignment.
    {
        wrap_t wl1(large{});
        auto wl2 = std::move(wl1);
        // NOLINTNEXTLINE
        REQUIRE(is_invalid(wl1));
        const wrap_t wl3(large{{}, "briffo"});
        wl1 = wl3;
        REQUIRE(!is_invalid(wl1));
        REQUIRE(value_ref<large>(wl1).str == "briffo");
    }

    {
        wrap_t wl1(small{});
        auto wl2 = std::move(wl1);
        // NOLINTNEXTLINE
        REQUIRE(!is_invalid(wl1));
        const wrap_t wl3(small{"briffo"});
        wl1 = wl3;
        REQUIRE(!is_invalid(wl1));
        REQUIRE(value_ref<small>(wl1).s == "briffo");
    }

    // Object revival via move assignment.
    {
        wrap_t wl1(large{});
        auto wl2 = std::move(wl1);
        // NOLINTNEXTLINE
        REQUIRE(is_invalid(wl1));
        wrap_t wl3(large{{}, "briffo"});
        wl1 = std::move(wl3);
        REQUIRE(!is_invalid(wl1));
        // NOLINTNEXTLINE
        REQUIRE(is_invalid(wl3));
        REQUIRE(value_ref<large>(wl1).str == "briffo");
    }

    {
        wrap_t wl1(small{});
        auto wl2 = std::move(wl1);
        // NOLINTNEXTLINE
        REQUIRE(!is_invalid(wl1));
        wrap_t wl3(small{"briffo"});
        wl1 = std::move(wl3);
        REQUIRE(!is_invalid(wl1));
        // NOLINTNEXTLINE
        REQUIRE(!is_invalid(wl3));
        REQUIRE(value_ref<small>(wl1).s == "briffo");
    }

    // Copy-assignment with different types.
    {
        wrap_t w1(small{}), w2(large{.str = "blaf"});
        w1 = w2;
        REQUIRE(value_ref<large>(w1).str == "blaf");
        REQUIRE(!is_invalid(w2));
    }

    // Move-assignment with different types.
    {
        wrap_t w1(small{}), w2(large{.str = "blaf"});
        w1 = std::move(w2);
        REQUIRE(value_ref<large>(w1).str == "blaf");
        // NOLINTNEXTLINE
        REQUIRE(is_invalid(w2));
    }

    // Copy-assignment with same types.
    {
        wrap_t w1(large{.str = "blif"}), w2(large{.str = "blaf"});
        w1 = w2;
        REQUIRE(value_ref<large>(w1).str == "blaf");
        REQUIRE(!is_invalid(w2));
    }
    {
        wrap_t w1(small{.s = "blif"}), w2(small{.s = "blaf"});
        w1 = w2;
        REQUIRE(value_ref<small>(w1).s == "blaf");
        REQUIRE(!is_invalid(w2));
    }

    // Move-assignment with same types.
    {
        wrap_t w1(large{.str = "blif"}), w2(large{.str = "blaf"});
        const auto *p1 = value_ptr<large>(w1);
        const auto *p2 = value_ptr<large>(w2);
        w1 = std::move(w2);
        REQUIRE(value_ref<large>(w1).str == "blaf");
        // NOLINTNEXTLINE
        REQUIRE(!is_invalid(w2));
        // Check value ptr swapping.
        REQUIRE(value_ptr<large>(w1) == p2);
        REQUIRE(value_ptr<large>(w2) == p1);
    }
    {
        wrap_t w1(small{.s = "blif"}), w2(small{.s = "blaf"});
        w1 = std::move(w2);
        REQUIRE(value_ref<small>(w1).s == "blaf");
        // NOLINTNEXTLINE
        REQUIRE(!is_invalid(w2));
    }
}

#if defined(TANUKI_WITH_BOOST_S11N)

TEST_CASE("s11n nostatic")
{
    using wrap_t = tanuki::wrap<any_iface, tanuki::config<>{.static_size = 0}>;

    wrap_t w(large{});
    value_ptr<large>(w)->buffer[0] = 42;

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << w;
    }

    w = wrap_t(3);

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> w;
    }

    REQUIRE(value_type_index(w) == typeid(large));
    REQUIRE(value_ptr<large>(w)->buffer[0] == 42);
}

TEST_CASE("s11n large")
{
    using wrap_t = tanuki::wrap<any_iface>;

    wrap_t w(large{});
    value_ptr<large>(w)->buffer[0] = 42;

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << w;
    }

    w = wrap_t(3);

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> w;
    }

    REQUIRE(value_type_index(w) == typeid(large));
    REQUIRE(value_ptr<large>(w)->buffer[0] == 42);
}

TEST_CASE("s11n small")
{
    using wrap_t = tanuki::wrap<any_iface>;

    wrap_t w(small{});
    value_ptr<small>(w)->s = "-42";

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << w;
    }

    w = wrap_t(3);

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> w;
    }

    REQUIRE(value_type_index(w) == typeid(small));
    REQUIRE(value_ptr<small>(w)->s == "-42");
}

#endif

#if defined(AASDSADASDSADSA)

TEST_CASE("basics2")
{
    using tanuki::config;
    using tanuki::wrap;

    const wrap<any_iface, config<int>{}> w1;

    (void)tanuki::iface_ptr(w1);
}

struct foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;
};

struct bar_iface : foo_iface {
    ~bar_iface() override = default;
    virtual void bar() const = 0;
};

template <typename Holder, typename IFace = foo_iface>
struct foo_iface_impl : IFace {
    void foo() const override
    {
        static_cast<Holder const *>(this)->m_value.foo();
    }
};

template <typename Holder, typename IFace>
struct bar_iface_impl : IFace {
    void bar() const override
    {
        static_cast<Holder const *>(this)->m_value.bar();
    }
};

template <typename Holder>
using foobar_iface_impl = tanuki::composite_iface_impl<Holder, bar_iface, foo_iface_impl, bar_iface_impl>;

// ---------------------------------------------------------------------

struct frob_iface {
    virtual ~frob_iface() = default;
    virtual void frob() const = 0;
};

struct niz_iface {
    virtual ~niz_iface() = default;
    virtual void niz() const = 0;
};

struct frobniz_iface : frob_iface, niz_iface {
};

template <typename Holder, typename IFace>
struct frob_iface_impl : IFace {
    void frob() const override
    {
        static_cast<Holder const *>(this)->m_value.frob();
    }
};

template <typename Holder, typename IFace>
struct niz_iface_impl : IFace {
    void niz() const override
    {
        static_cast<Holder const *>(this)->m_value.niz();
    }
};

template <typename Holder>
using frobniz_iface_impl = tanuki::composite_iface_impl<Holder, frobniz_iface, frob_iface_impl, niz_iface_impl>;

using tanuki::wrap;

TEST_CASE("basics")
{
    struct blaf {
        char buffer[100];
    };

    wrap<iface0, iface0_impl> w1(3.), w2(blaf{}), w3(std::function<void()>{});

    auto w4 = w3;
    auto w5(std::move(w2));
    // NOLINTNEXTLINE(bugprone-use-after-move)
    // REQUIRE(w2.is_invalid());

    w1 = std::move(w3);

    w3 = wrap<iface0, iface0_impl>(std::function<void()>{});

    auto w1a = w1;
    w1 = w1a;

    auto w5a = wrap<iface0, iface0_impl>(blaf{});
    w5a = std::move(w5);

    w2 = wrap<iface0, iface0_impl>(blaf{});
    auto w2a = wrap<iface0, iface0_impl>(blaf{});
    w2 = w2a;
}

struct my_foobar {
    void foo() const
    {
        std::cout << "foo\n";
    };
    void bar() const
    {
        std::cout << "bar\n";
    };
};

struct my_frobniz {
    void frob() const
    {
        std::cout << "frob\n";
    };
    void niz() const
    {
        std::cout << "niz\n";
    };
};

void dooda(const bar_iface &) {}

void diida(const foo_iface &) {}

void daada(const frobniz_iface &) {}

void daadauda(const frob_iface &) {}

TEST_CASE("foo and bar")
{
    wrap<bar_iface, foobar_iface_impl> g{my_foobar{}};

    g->foo();
    g->bar();

    wrap<frobniz_iface, frobniz_iface_impl> h{my_frobniz{}};

    h->frob();
    h->niz();

    dooda(*g);
    diida(*g);
    daada(*h);
    daadauda(*h);

    (void)static_cast<bar_iface *>(g);
}

TEST_CASE("dasda")
{
    wrap<iface0, iface0_impl, tanuki::config{.static_size = 0}> w1(4);
    auto w2 = std::move(w1);
    wrap<iface0, iface0_impl, tanuki::config{.static_size = 0}> w3(42.);

    using std::swap;

    swap(w2, w3);
}

template <typename Wrap>
struct foo_ref_iface {
    TANUKI_MAKE_REF_IFACE_MEMFUN(foo)
};

TEST_CASE("ref iface")
{
    wrap<foo_iface, foo_iface_impl, tanuki::default_config, tanuki::no_type_checks, foo_ref_iface> w(my_foobar{});

    w.foo();
}

template <typename R, typename... Args>
struct function_iface {
    virtual ~function_iface() = default;
    virtual R operator()(Args... args) const = 0;
};

// template <typename R, typename... Args>
// struct foo_function_iface : function_iface<R, Args...>, foo_iface {
// };

template <typename Holder, typename R, typename... Args>
struct function_iface_impl : function_iface<R, Args...> {
    R operator()(Args... args) const final;
};

template <typename R, typename... Args>
struct blap_iface : function_iface<R, Args...> {
    virtual void blap() const = 0;
};

template <typename Holder, typename R, typename... Args>
struct blap_iface_impl : function_iface_impl<Holder, R, Args...> {
    void blap() const override;
};

// template <typename Holder, typename R, typename... Args>
// using foo_function_iface_impl
//     = foo_iface_impl<Holder, function_iface_impl<Holder, foo_function_iface<R, Args...>, R, Args...>>;

// template <typename R, typename... Args>
// using foo_function = wrap<foo_function_iface<R, Args...>, foo_function_iface_impl>;

template <typename R, typename... Args>
using foo_function = wrap<foo_function_iface<R, Args...>, function_iface_impl>;

// TEST_CASE("foo_function")
// {
//     foo_function<void> f([]() {});
// }

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
