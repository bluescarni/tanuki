#include <concepts>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

#if defined(TANUKI_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#endif

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

struct foo {
    int value = 42;
    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & value;
    }
};

template <typename Base, typename Holder, typename T>
struct any_iface_impl : Base {
};

// NOLINTNEXTLINE
struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(foo, any_iface)

#endif

struct thrower {
    explicit thrower(int)
    {
        throw std::invalid_argument("boo");
    }
};

// NOLINTNEXTLINE
struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable &) = delete;
    noncopyable(noncopyable &&) noexcept = default;
};

TEST_CASE("basics")
{
    using Catch::Matchers::Message;

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
    REQUIRE(contains_reference(w4));

    // Generic in-place constructor.
    const wrap2_t w5(std::in_place_type<int>, 11);
    REQUIRE(!is_invalid(w5));
    REQUIRE(value_isa<int>(w5));
    REQUIRE(value_ref<int>(w5) == 11);

    // Copy construction.
    REQUIRE(std::is_nothrow_copy_constructible_v<wrap2_t>);
    auto w6(w5);
    REQUIRE(!is_invalid(w6));
    REQUIRE(value_isa<int>(w6));
    REQUIRE(value_ref<int>(w6) == 11);
    REQUIRE(tanuki::value_ptr<int>(w5) == tanuki::value_ptr<int>(w6));

    // Move construction.
    REQUIRE(std::is_nothrow_copy_constructible_v<wrap2_t>);
    auto w7(std::move(w6));
    REQUIRE(!is_invalid(w7));
    REQUIRE(value_isa<int>(w7));
    REQUIRE(value_ref<int>(w7) == 11);
    REQUIRE(tanuki::value_ptr<int>(w5) == tanuki::value_ptr<int>(w7));

    // Move assignment.
    REQUIRE(std::is_nothrow_move_assignable_v<wrap2_t>);
    wrap2_t w8;
    w8 = std::move(w7);
    REQUIRE(!is_invalid(w8));
    REQUIRE(value_isa<int>(w8));
    REQUIRE(value_ref<int>(w8) == 11);
    REQUIRE(tanuki::value_ptr<int>(w5) == tanuki::value_ptr<int>(w8));

    // Copy assignment.
    REQUIRE(std::is_nothrow_copy_assignable_v<wrap2_t>);
    wrap2_t w9;
    w9 = w8;
    REQUIRE(!is_invalid(w9));
    REQUIRE(value_isa<int>(w9));
    REQUIRE(value_ref<int>(w9) == 11);
    REQUIRE(tanuki::value_ptr<int>(w5) == tanuki::value_ptr<int>(w9));

    // Generic assignment.
    wrap2_t w10;
    w10 = foo{12};
    REQUIRE(!is_invalid(w10));
    REQUIRE(value_isa<foo>(w10));
    REQUIRE(value_ref<foo>(w10).value == 12);

    // A few tests for the free function interface.
    REQUIRE(value_type_index(w10) == typeid(foo));
    REQUIRE(iface_ptr(w10) != nullptr);
    REQUIRE(iface_ptr(std::as_const(w10)) != nullptr);
    REQUIRE(!has_static_storage(w10));
    REQUIRE(value_ptr<foo>(w10) == raw_value_ptr(w10));
    REQUIRE(value_ptr<foo>(std::as_const(w10)) == raw_value_ptr(std::as_const(w10)));
    REQUIRE(!contains_reference(w10));

    // Swapping.
    REQUIRE(std::is_nothrow_swappable_v<wrap2_t>);
    using std::swap;
    auto *old_ptr9 = value_ptr<int>(w9);
    auto *old_ptr10 = value_ptr<foo>(w10);
    swap(w10, w9);
    REQUIRE(value_ptr<foo>(w9) == old_ptr10);
    REQUIRE(value_ptr<int>(w10) == old_ptr9);
    auto w10a = w10;
    REQUIRE(!same_value(w10, w9));
    REQUIRE(same_value(w10, w10a));
    REQUIRE(same_value(w10a, w10));
    auto w10b = std::move(w10a);
    REQUIRE(same_value(w10, w10b));
    REQUIRE(same_value(w10b, w10));

    // Construction/assignment into the invalid state.
    wrap2_t w11{tanuki::invalid_wrap_t{}};

    REQUIRE(is_invalid(w11));

    w11 = tanuki::invalid_wrap_t{};

    REQUIRE(is_invalid(w11));

    w11 = 123;

    REQUIRE(!is_invalid(w11));

    w11 = tanuki::invalid_wrap_t{};

    REQUIRE(is_invalid(w11));

    // A couple of emplace tests.
    emplace<int>(w11, 43);

    REQUIRE(!is_invalid(w11));
    REQUIRE(value_ref<int>(w11) == 43);

    REQUIRE_THROWS_MATCHES(emplace<thrower>(w11, 33), std::invalid_argument, Message("boo"));
    REQUIRE(!is_invalid(w11));
    REQUIRE(value_ref<int>(w11) == 43);

    // Revive via emplacement.
    w11 = tanuki::invalid_wrap;
    REQUIRE(is_invalid(w11));
    emplace<foo>(w11);
    REQUIRE(!is_invalid(w11));
    REQUIRE(value_isa<foo>(w11));

    // noexcept testing.
    REQUIRE(!noexcept(emplace<int>(w11, 43)));
    REQUIRE(!noexcept(emplace<foo>(w11)));
    REQUIRE(!noexcept(emplace<thrower>(w11, 33)));

    // Test deep copying.
    wrap2_t w12(123);
    auto w12_copy = copy(w12);
    REQUIRE(!same_value(w12, w12_copy));
    REQUIRE(value_ptr<int>(w12) != value_ptr<int>(w12_copy));
    REQUIRE(value_ref<int>(w12_copy) == 123);

    const wrap2_t w13(noncopyable{});
    REQUIRE_THROWS_MATCHES(copy(w13), std::invalid_argument, Message("Attempting to clone a non-copyable value type"));
}

#if defined(TANUKI_WITH_BOOST_S11N)

TEST_CASE("s11n")
{
    using wrap_t = tanuki::wrap<any_iface, tanuki::config<int>{.semantics = tanuki::wrap_semantics::reference}>;

    wrap_t w(foo{});
    value_ptr<foo>(w)->value = -1;

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

    REQUIRE(value_type_index(w) == typeid(foo));
    REQUIRE(value_ptr<foo>(w)->value == -1);
}

TEST_CASE("s11n invalid")
{
    using wrap_t = tanuki::wrap<any_iface, tanuki::config<int>{.invalid_default_ctor = true,
                                                               .semantics = tanuki::wrap_semantics::reference}>;

    wrap_t w;
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

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
