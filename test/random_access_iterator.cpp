#include <concepts>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "random_access_iterator.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic")
{
    using Catch::Matchers::Message;

    using int_iter = facade::random_access_iterator<int, int &, int &&>;

    REQUIRE(std::random_access_iterator<int_iter>);
    REQUIRE(std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);

    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);
    REQUIRE(std::same_as<int, std::iter_value_t<int_iter>>);
    REQUIRE(std::same_as<int &, std::iter_reference_t<int_iter>>);
    REQUIRE(std::same_as<int &&, std::iter_rvalue_reference_t<int_iter>>);
    REQUIRE(std::same_as<std::random_access_iterator_tag, int_iter::iterator_concept>);

    // Tests for def constructed instances.
    REQUIRE(int_iter{} == int_iter{});
    REQUIRE(int_iter{} <= int_iter{});
    REQUIRE(int_iter{} >= int_iter{});
    REQUIRE(!(int_iter{} != int_iter{}));
    REQUIRE(!(int_iter{} < int_iter{}));
    REQUIRE(!(int_iter{} > int_iter{}));
    REQUIRE(int_iter{} - int_iter{} == 0);
    int_iter def;
    REQUIRE_THROWS_MATCHES(++def, std::runtime_error, Message("Attempting to increase a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(--def, std::runtime_error, Message("Attempting to decrease a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(def += 1, std::runtime_error,
                           Message("Attempting to increment a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(def -= 1, std::runtime_error,
                           Message("Attempting to decrement a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(*def, std::runtime_error,
                           Message("Attempting to dereference a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(std::ranges::iter_move(def), std::runtime_error,
                           Message("Attempting to invoke iter_move() on a default-constructed iterator"));
    REQUIRE_THROWS_MATCHES(int_iter{std::vector<int>::iterator{}} != int_iter{static_cast<int *>(nullptr)},
                           std::runtime_error, Message("Cannot compare iterators of different types"));

    {
        int arr[] = {1, 2, 3};
        int_iter it(std::begin(arr));
        REQUIRE(has_static_storage(it));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*--it == 1);
        REQUIRE(*it++ == 1);
        REQUIRE(*it-- == 2);
        REQUIRE(*it == 1);
        it += 2;
        REQUIRE(*it == 3);
        it -= 2;
        REQUIRE(*it == 1);
        REQUIRE(*(it + 1) == 2);
        REQUIRE(*(1 + it) == 2);
        it += 3;
        REQUIRE(*(it - 1) == 3);
        REQUIRE(it - int_iter(std::begin(arr)) == 3);
        it -= 3;
        REQUIRE(it[2] == 3);

        REQUIRE(!(it < it));
        REQUIRE(it < it + 1);
        REQUIRE(!(it + 1 < it));

        REQUIRE(it <= it);
        REQUIRE(it <= it + 1);
        REQUIRE(!(it + 1 <= it));

        REQUIRE(!(it > it));
        REQUIRE(!(it > it + 1));
        REQUIRE(it + 1 > it);

        REQUIRE(it >= it);
        REQUIRE(!(it >= it + 1));
        REQUIRE(it + 1 >= it);
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
