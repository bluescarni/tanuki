#include <array>
#include <concepts>
#include <functional>
#include <mutex>
#include <sstream>
#include <stdexcept>
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
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wself-move"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

struct nonwrappable {
};

template <typename, typename, typename>
struct any_iface_impl {
};

struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename T>
    requires(!std::same_as<T, nonwrappable>)
struct any_iface_impl<Base, Holder, T> : Base {
};

struct any_iface2 {
    template <typename Base, typename, typename>
    struct impl : Base {
    };
};

template <typename T, typename IFace>
concept with_holder_size = requires() { requires(tanuki::holder_size<T, IFace> > 0u); };

template <typename T, typename IFace>
concept with_holder_align = requires() { requires(tanuki::holder_align<T, IFace> > 0u); };

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

template <typename W, typename T, typename... Args>
concept emplaceable = requires(W &w, Args &&...args) { emplace<T>(w, std::forward<Args>(args)...); };

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(large, any_iface)
TANUKI_S11N_WRAP_EXPORT(small, any_iface)
TANUKI_S11N_WRAP_EXPORT2(small2, "small2", any_iface)

#endif

void my_func(int) {}

TEST_CASE("basics")
{
    using tanuki::wrap;
    using wrap_t = wrap<any_iface>;

    REQUIRE(tanuki::default_config.static_size == tanuki::wrap_cfg<wrap_t>.static_size);

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
    REQUIRE(value_ptr<const double>(w1) == nullptr);
    REQUIRE(value_ptr<volatile double>(w1) == nullptr);
    REQUIRE(value_ptr<const volatile double>(w1) == nullptr);

    REQUIRE(value_isa<large>(w2));
    value_ref<large>(w2).buffer[0] = 2;
    REQUIRE(value_ptr<large>(std::as_const(w2))->buffer[0] == 2);
    REQUIRE(value_ptr<const large>(std::as_const(w2)) == nullptr);
    REQUIRE(value_ptr<volatile large>(std::as_const(w2)) == nullptr);
    REQUIRE(value_ptr<const volatile large>(std::as_const(w2)) == nullptr);
    REQUIRE(value_type_index(w3) == typeid(std::function<void()>));
    REQUIRE(!(*value_ptr<std::function<void()>>(w3)));

    // Default ctor disabled by default.
    REQUIRE(!std::default_initializable<wrap_t>);

    // noexcept handling for the value ctor.
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, int>);
    REQUIRE(std::is_constructible_v<wrap_t, const std::string &>);
    REQUIRE(!std::is_nothrow_constructible_v<wrap_t, const std::string &>);
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, std::string &&>);
    REQUIRE(std::is_constructible_v<wrap_t, int>);
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, int>);

    // noexcept handling for the emplace ctor.
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, std::in_place_type_t<int>, int>);
    REQUIRE(!std::is_nothrow_constructible_v<wrap_t, std::in_place_type_t<std::string>, const std::string &>);
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, std::in_place_type_t<std::string>, std::string &&>);
    REQUIRE(std::is_nothrow_constructible_v<wrap_t, std::in_place_type_t<int>, int>);

    // noexcept handling for emplacement.
    REQUIRE(noexcept(emplace<int>(w1)));
    REQUIRE(!noexcept(emplace<std::string>(w1, "asdsada")));
    std::string tmp_str = "asda";
    REQUIRE(noexcept(emplace<std::string>(w1, std::move(tmp_str))));

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

    // Emplace test with class which is not copyable/movable/swappable.
    wrap<any_iface, tanuki::config<>{.copyable = false, .movable = false, .swappable = false}> w_mut(
        std::in_place_type<std::mutex>);
    REQUIRE(noexcept(wrap_t(std::in_place_type<int>)));

    // Check throwing in value_ref.
    REQUIRE_THROWS_AS(value_ref<int>(w_mut), std::bad_cast);
    REQUIRE_THROWS_AS(value_ref<int>(std::as_const(w_mut)), std::bad_cast);

    // Test that a function is held via a function pointer.
    const wrap_t wfunc1(my_func);
    const wrap_t wfunc2(&my_func);
    REQUIRE(value_isa<void (*)(int)>(wfunc1));
    REQUIRE(value_isa<void (*)(int)>(wfunc2));

    // Minimal test for iface_with_impl.
    REQUIRE(!tanuki::iface_with_impl<any_iface, nonwrappable>);
    REQUIRE(!tanuki::iface_with_impl<any_iface &, const nonwrappable &>);
    REQUIRE(!tanuki::iface_with_impl<void, void>);

    // Test that concept checking on holder_size fails
    // if an interface does not have an implementation for
    // a type.
    REQUIRE(!with_holder_size<nonwrappable, any_iface>);

    // Same on holder_align.
    REQUIRE(!with_holder_align<nonwrappable, any_iface>);

    // In-place nested construction.
    wrap_t w4(4.), w5(std::in_place_type<wrap_t>, w4);
    REQUIRE(!noexcept(wrap_t(std::in_place_type<wrap_t>, w4)));
    REQUIRE(value_ref<double>(value_ref<wrap_t>(w5)) == 4.);

    wrap_t w8(large{.str = "foobar"}), w9(std::in_place_type<wrap_t>, std::move(w8));
    REQUIRE(value_ref<large>(value_ref<wrap_t>(w9)).str == "foobar");
    REQUIRE(is_invalid(w8));

    // Try with inline implementation as well.
    using wrap2_t = wrap<any_iface2>;
    wrap2_t w2inl{std::string{"foo"}};
    REQUIRE(value_ref<std::string>(w2inl) == "foo");

    // Check that we cannot emplace/in-place init with invalid types.
    REQUIRE(!std::constructible_from<wrap_t, std::in_place_type_t<void>>);
    REQUIRE(!std::constructible_from<wrap_t, std::in_place_type_t<int &>>);
    REQUIRE(!std::constructible_from<wrap_t, std::in_place_type_t<const int>>);

    REQUIRE(!emplaceable<wrap_t, void>);
    REQUIRE(!emplaceable<wrap_t, int &>);
    REQUIRE(!emplaceable<wrap_t, const int>);
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

    REQUIRE(tanuki::wrap_cfg<wrap_t>.static_size == 0u);

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

TEST_CASE("s11n invalid")
{
    using wrap_t = tanuki::wrap<any_iface>;

    wrap_t w(large{});
    value_ptr<large>(w)->buffer[0] = 42;
    auto w_move = std::move(w);
    // NOLINTNEXTLINE
    REQUIRE(is_invalid(w));

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << w;
    }

    w = wrap_t(3);
    REQUIRE(!is_invalid(w));

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> w;
    }

    REQUIRE(is_invalid(w));
}

struct nodefctor {
    explicit nodefctor(int) {}
    nodefctor() = delete;
    nodefctor(nodefctor &&) noexcept = default;
};

TEST_CASE("s11n nodef")
{
    using Catch::Matchers::ContainsSubstring;
    using Catch::Matchers::Message;
    using Catch::Matchers::MessageMatches;
    using Catch::Matchers::StartsWith;

    using wrap_t = tanuki::wrap<any_iface, tanuki::config<>{.copyable = false, .movable = false, .swappable = false}>;

    wrap_t w(nodefctor{3});

    std::stringstream ss;

    boost::archive::binary_oarchive oa(ss);

    REQUIRE_THROWS_MATCHES(oa << w, std::invalid_argument,
                           MessageMatches(StartsWith("Cannot serialise a wrap containing a value of type '")));
    REQUIRE_THROWS_MATCHES(oa << w, std::invalid_argument,
                           MessageMatches(ContainsSubstring("the type is not default-initializable")));
}

struct nomovector {
    nomovector() = default;
    nomovector(nomovector &&) noexcept = delete;
};

TEST_CASE("s11n nomove")
{
    using Catch::Matchers::ContainsSubstring;
    using Catch::Matchers::Message;
    using Catch::Matchers::MessageMatches;
    using Catch::Matchers::StartsWith;

    using wrap_t = tanuki::wrap<any_iface, tanuki::config<>{.copyable = false, .movable = false, .swappable = false}>;

    wrap_t w(std::in_place_type<nomovector>);

    std::stringstream ss;

    boost::archive::binary_oarchive oa(ss);

    REQUIRE_THROWS_MATCHES(oa << w, std::invalid_argument,
                           MessageMatches(StartsWith("Cannot serialise a wrap containing a value of type '")));
    REQUIRE_THROWS_MATCHES(oa << w, std::invalid_argument,
                           MessageMatches(ContainsSubstring("the type is not move-constructible")));
}

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
