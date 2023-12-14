#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "catch2/matchers/catch_matchers_exception.hpp"
#include "forward_iterator.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic")
{
    using Catch::Matchers::Message;

    using int_iter = facade::forward_iterator<int, int &, int &&>;

    REQUIRE(std::forward_iterator<int_iter>);
    REQUIRE(std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);

    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);
    REQUIRE(std::same_as<int, std::iter_value_t<int_iter>>);
    REQUIRE(std::same_as<int &, std::iter_reference_t<int_iter>>);
    REQUIRE(std::same_as<int &&, std::iter_rvalue_reference_t<int_iter>>);
    REQUIRE(std::same_as<std::forward_iterator_tag, int_iter::iterator_concept>);

    // Tests for def constructed instances.
    REQUIRE(int_iter{} == int_iter{});
    REQUIRE(!(int_iter{} != int_iter{}));
    int_iter def;
    REQUIRE_THROWS_MATCHES(++def, std::runtime_error,
                           Message("Attempting to increase a default-constructed forward_iterator"));
    REQUIRE_THROWS_MATCHES(*def, std::runtime_error,
                           Message("Attempting to dereference a default-constructed forward_iterator"));
    REQUIRE_THROWS_MATCHES(std::ranges::iter_move(def), std::runtime_error,
                           Message("Attempting to invoke iter_move() on a default-constructed forward_iterator"));

    // Comparison between forward iterators containing different types.
    REQUIRE(int_iter{std::vector<int>::iterator{}} != int_iter{std::list<int>::iterator{}});
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

bool operator==(const iter_move1 &, const iter_move1 &)
{
    return true;
}

// LCOV_EXCL_STOP

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int iter_move1_counter = 0;

double iter_move(const iter_move1 &)
{
    ++iter_move1_counter;

    return 0;
}

} // namespace ns

// A test to check that the iter_move() customisation
// in the reference interface is picked up correctly.
TEST_CASE("iter_move")
{
    auto nit = facade::forward_iterator<double, double, double>(ns::iter_move1{});
    (void)std::ranges::iter_move(nit);
    (void)std::ranges::iter_move(nit);
    (void)std::ranges::iter_move(nit);
    REQUIRE(ns::iter_move1_counter == 3);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
