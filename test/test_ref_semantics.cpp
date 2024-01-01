#include <concepts>
#include <functional>
#include <sstream>
#include <type_traits>
#include <typeinfo>
#include <utility>

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

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

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
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(foo, any_iface)

#endif

TEST_CASE("basics")
{
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
    const wrap2_t w5(tanuki::in_place<int>, 11);
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

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
