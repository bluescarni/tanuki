#include <sstream>
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

// LCOV_EXCL_START

template <typename Base, typename, typename>
struct any_iface_impl : public Base {
};

// NOLINTNEXTLINE
struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

struct over {
    alignas(alignof(std::max_align_t)) char value{};

    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & value;
    }
};

// LCOV_EXCL_STOP

using wrap_t = tanuki::wrap<any_iface, tanuki::config<>{.static_align = alignof(std::max_align_t) / 2u}>;

TEST_CASE("basics")
{
    wrap_t w2(over{});
    REQUIRE(has_dynamic_storage(w2));

    const auto *old_ptr2 = iface_ptr(w2);

    auto w3(w2);
    REQUIRE(has_dynamic_storage(w3));

    const auto *old_ptr3 = iface_ptr(w3);

    auto w4(std::move(w3));

    REQUIRE(old_ptr3 == iface_ptr(w4));

    w2 = w4;
    REQUIRE(has_dynamic_storage(w2));

    w2 = std::move(w4);
    REQUIRE(old_ptr3 == iface_ptr(w2));
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved)
    REQUIRE(!is_invalid(w4));
    REQUIRE(iface_ptr(w4) == old_ptr2);
}

#if defined(TANUKI_WITH_BOOST_S11N)

TANUKI_S11N_WRAP_EXPORT(over, any_iface)

TEST_CASE("s11n")
{
    wrap_t w(over{});
    value_ptr<over>(w)->value = char{1};

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

    REQUIRE(value_type_index(w) == typeid(over));
    REQUIRE(value_ptr<over>(w)->value == char{1});
}

#endif

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
