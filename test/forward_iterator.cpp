#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "forward_iterator.hpp"
#include "sentinel.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

template <typename T>
concept can_make_forward_iterator = requires(T it) { facade::make_forward_iterator(it); };

TEST_CASE("basic")
{
    using Catch::Matchers::Message;
    using Catch::Matchers::MessageMatches;
    using Catch::Matchers::StartsWith;

    using int_iter = facade::forward_iterator<int, int &, int &&>;

    REQUIRE(std::input_or_output_iterator<int_iter>);
    REQUIRE(std::input_iterator<int_iter>);
    REQUIRE(std::forward_iterator<int_iter>);
    REQUIRE(std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);
    REQUIRE(!can_make_forward_iterator<int>);

    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);
    REQUIRE(std::same_as<int, std::iter_value_t<int_iter>>);
    REQUIRE(std::same_as<int &, std::iter_reference_t<int_iter>>);
    REQUIRE(std::same_as<int &&, std::iter_rvalue_reference_t<int_iter>>);
    REQUIRE(std::same_as<std::forward_iterator_tag, int_iter::iterator_concept>);

    // Tests for def constructed instances.
    REQUIRE(int_iter{} == int_iter{});
    REQUIRE(!(int_iter{} != int_iter{}));
    int_iter def;
    REQUIRE_THROWS_MATCHES(++def, std::runtime_error, Message("Attempting to increase a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(*def, std::runtime_error,
                           Message("Attempting to dereference a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(std::ranges::iter_move(def), std::runtime_error,
                           Message("Attempting to invoke iter_move() on a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(int_iter{std::vector<int>::iterator{}} != int_iter{std::list<int>::iterator{}},
                           std::runtime_error, MessageMatches(StartsWith("Unable to compare an iterator of type")));

    {
        int arr[] = {1, 2, 3};
        int_iter it(std::begin(arr));
        REQUIRE(has_static_storage(it));
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*std::as_const(it) == 3);

        // Check that make_forward_iterator() on an io_iterator
        // returns a copy.
        auto it2 = facade::make_forward_iterator(facade::make_forward_iterator(std::begin(arr)));
        REQUIRE(value_isa<int *>(it2));

        REQUIRE(std::sentinel_for<facade::sentinel, int_iter>);
        REQUIRE(std::sentinel_for<int_iter, int_iter>);
    }

    {
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_forward_iterator(std::begin(vec));
        REQUIRE(std::same_as<decltype(it), int_iter>);
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*std::as_const(it) == 3);

        REQUIRE(std::sentinel_for<facade::sentinel, int_iter>);
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_forward_iterator(vec.cbegin());
        REQUIRE(std::same_as<decltype(it), facade::forward_iterator<int, const int &, const int &&>>);
        REQUIRE(std::forward_iterator<decltype(it)>);
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*std::as_const(it) == 3);
    }

    {
        std::list<int> lst = {1, 2, 3};
        int_iter it(std::begin(lst));
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*std::as_const(it) == 3);
    }
}

// LCOV_EXCL_START

struct noniter1 {
    double operator*() const
    {
        return {};
    }
    void operator++() {}
    friend bool operator==(const noniter1 &, const noniter1 &)
    {
        return true;
    }
};

struct noniter2 {
    double &operator*() const
    {
        static double x = 56;
        return x;
    }
    void operator++() {}
    friend bool operator==(const noniter2 &, const noniter2 &)
    {
        return true;
    }
};

struct noniter3 {
    const double &operator*() const
    {
        static const double x = 56;
        return x;
    }
    void operator++() {}
    friend bool operator==(const noniter3 &, const noniter3 &)
    {
        return true;
    }
};

struct noniter4 {
    double &&operator*() const
    {
        static double x = 56;
        return static_cast<double &&>(x);
    }
    void operator++() {}
    friend bool operator==(const noniter4 &, const noniter4 &)
    {
        return true;
    }
};

struct noniter5 {
    const double &&operator*() const
    {
        static const double x = 56;
        return static_cast<const double &&>(x);
    }
    void operator++() {}
    friend bool operator==(const noniter5 &, const noniter5 &)
    {
        return true;
    }
};

// LCOV_EXCL_STOP

TEST_CASE("noniter")
{
    {
        using iter_t = facade::forward_iterator<double, double, double>;

        auto nit = facade::make_forward_iterator(noniter1{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::forward_iterator<iter_t>);
        REQUIRE(std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::forward_iterator<double, double &, double &&>;

        auto nit = facade::make_forward_iterator(noniter2{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::forward_iterator<iter_t>);
        REQUIRE(std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::forward_iterator<double, const double &, const double &&>;

        auto nit = facade::make_forward_iterator(noniter3{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::forward_iterator<iter_t>);
        REQUIRE(std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::forward_iterator<double, double &&, double &&>;

        auto nit = facade::make_forward_iterator(noniter4{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::forward_iterator<iter_t>);
        REQUIRE(std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::forward_iterator<double, const double &&, const double &&>;

        auto nit = facade::make_forward_iterator(noniter5{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::forward_iterator<iter_t>);
        REQUIRE(std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }
}

namespace ns
{

// LCOV_EXCL_START

struct iter_move1 {
    double operator*() const
    {
        return {};
    }
    void operator++() {}
};

static bool operator==(const iter_move1 &, const iter_move1 &)
{
    return true;
}

// LCOV_EXCL_STOP

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static int iter_move1_counter = 0;

static double iter_move(const iter_move1 &)
{
    ++iter_move1_counter;

    return 0;
}

} // namespace ns

// A test to check that the iter_move() customisation
// in the reference interface is picked up correctly.
TEST_CASE("iter_move")
{
    auto nit = facade::make_forward_iterator(ns::iter_move1{});
    (void)std::ranges::iter_move(nit);
    (void)std::ranges::iter_move(nit);
    (void)std::ranges::iter_move(nit);
    REQUIRE(ns::iter_move1_counter == 3);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)
