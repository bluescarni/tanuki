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

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

struct nonwrappable {
};

template <typename, typename, typename>
struct any_iface_impl {
};

// NOLINTNEXTLINE
struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename T>
    requires(!std::same_as<T, nonwrappable>)
struct any_iface_impl<Base, Holder, T> : Base {
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

struct small2 {
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
TANUKI_S11N_WRAP_EXPORT2(small2, "small2", any_iface)

#endif

struct copythrow {
    copythrow() = default;
    // NOLINTNEXTLINE
    copythrow(const copythrow &){};
    copythrow(copythrow &&) noexcept = delete;
    copythrow &operator=(const copythrow &) = delete;
    copythrow &operator=(copythrow &&) noexcept = delete;
    ~copythrow() = default;
};

void my_func(int) {}

TEST_CASE("basics")
{
    using tanuki::wrap;
    using wrap_t = wrap<any_iface>;

    // Version macro check.
    REQUIRE(TANUKI_VERSION_STRING
            == std::to_string(TANUKI_VERSION_MAJOR) + "." + std::to_string(TANUKI_VERSION_MINOR) + "."
                   + std::to_string(TANUKI_VERSION_PATCH));

    // A few simple initialisations from values.
    wrap_t w1(3.), w2(large{}), w3(std::function<void()>{});
    REQUIRE(has_static_storage(w1));
    REQUIRE(has_dynamic_storage(w2));
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
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, tanuki::in_place_type<int>, int>);
    REQUIRE(std::is_constructible_v<wrap_t, tanuki::in_place_type<copythrow>, const copythrow &>);
    REQUIRE(!std::is_nothrow_constructible_v<wrap_t, tanuki::in_place_type<copythrow>, const copythrow &>);

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
    wrap_t w_mut(tanuki::in_place<std::mutex>);
    REQUIRE(noexcept(wrap_t(tanuki::in_place<int>)));

    // Check throwing in value_ref.
    REQUIRE_THROWS_AS(value_ref<int>(w_mut), std::bad_cast);
    REQUIRE_THROWS_AS(value_ref<int>(std::as_const(w_mut)), std::bad_cast);

    // Test that a function is held via a function pointer.
    const wrap_t wfunc1(my_func);
    const wrap_t wfunc2(&my_func);
    REQUIRE(value_isa<void (*)(int)>(wfunc1));
    REQUIRE(value_isa<void (*)(int)>(wfunc2));
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

TEST_CASE("s11n small2")
{
    using wrap_t = tanuki::wrap<any_iface>;

    wrap_t w(small2{});
    value_ptr<small2>(w)->s = "-42";

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

    REQUIRE(value_type_index(w) == typeid(small2));
    REQUIRE(value_ptr<small2>(w)->s == "-42");
}

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
