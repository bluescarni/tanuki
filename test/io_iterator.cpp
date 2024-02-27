#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "io_iterator.hpp"
#include "sentinel.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename T>
concept can_make_io_iterator = requires(T it) { facade::make_io_iterator(it); };

TEST_CASE("basic")
{
    using int_iter = facade::io_iterator<int &>;

    REQUIRE(std::input_or_output_iterator<int_iter>);
    REQUIRE(!std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);
    REQUIRE(!can_make_io_iterator<int>);

    REQUIRE(std::same_as<std::iter_reference_t<int_iter>, int &>);
    REQUIRE(std::same_as<std::iter_reference_t<facade::io_iterator<int>>, int>);
    REQUIRE(std::same_as<std::iter_reference_t<facade::io_iterator<int &&>>, int &&>);
    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);

    {
        int arr[] = {1, 2, 3};
        int_iter it(std::begin(arr));
        REQUIRE(has_static_storage(it));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);

        // Check that make_io_iterator() on an io_iterator
        // returns a copy.
        auto it2 = facade::make_io_iterator(facade::make_io_iterator(std::begin(arr)));
        REQUIRE(value_isa<int *>(it2));

        REQUIRE(it == facade::sentinel(arr + 2));
        REQUIRE(it != facade::sentinel(arr + 1));
        REQUIRE(std::sentinel_for<facade::sentinel, int_iter>);
    }

    {
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_io_iterator(std::begin(vec));
        REQUIRE(std::same_as<decltype(it), int_iter>);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);

        REQUIRE(it == facade::sentinel(vec.begin() + 2));
        REQUIRE(it != facade::sentinel(vec.begin() + 1));
        REQUIRE(std::sentinel_for<facade::sentinel, int_iter>);
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_io_iterator(vec.cbegin());
        REQUIRE(std::same_as<decltype(it), facade::io_iterator<const int &>>);
        REQUIRE(std::input_or_output_iterator<decltype(it)>);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);
    }

    {
        std::list<int> lst = {1, 2, 3};
        int_iter it(std::begin(lst));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);
    }
}

struct noniter1 {
    struct sentinel_t {
    };

    double operator*() const
    {
        return {};
    }
    void operator++() {}

    friend bool operator==(const noniter1 &, const sentinel_t &)
    {
        return false;
    }
};

struct noniter2 {
    double operator*() const
    {
        return {};
    }
    void operator++() {}

    friend bool operator==(const noniter2 &, const noniter2 &)
    {
        return true;
    }
};

TEST_CASE("noniter")
{
    using Catch::Matchers::MessageMatches;
    using Catch::Matchers::StartsWith;

    using iter_t = facade::io_iterator<double>;

    REQUIRE(std::same_as<iter_t, decltype(facade::make_io_iterator(noniter1{}))>);

    REQUIRE(std::input_or_output_iterator<iter_t>);
    REQUIRE(!std::default_initializable<iter_t>);
    REQUIRE(!std::constructible_from<iter_t, int>);

    REQUIRE(std::same_as<iter_t, decltype(facade::make_io_iterator(noniter2{}))>);

    auto it = facade::make_io_iterator(noniter1{});
    REQUIRE(it != facade::sentinel(noniter1::sentinel_t{}));
    REQUIRE(facade::sentinel(noniter1::sentinel_t{}) != it);
    REQUIRE_THROWS_MATCHES(it != facade::sentinel(int{}), std::runtime_error,
                           MessageMatches(StartsWith("Unable to compare an iterator of type")));
    REQUIRE_THROWS_MATCHES(facade::sentinel(int{}) != it, std::runtime_error,
                           MessageMatches(StartsWith("Unable to compare an iterator of type")));

    it = facade::make_io_iterator(noniter2{});
    REQUIRE(!(it != facade::sentinel(noniter2{})));
    REQUIRE(!(facade::sentinel(noniter2{}) != it));
    REQUIRE_THROWS_MATCHES(it != facade::sentinel(int{}), std::runtime_error,
                           MessageMatches(StartsWith("Unable to compare an iterator of type")));
    REQUIRE_THROWS_MATCHES(facade::sentinel(int{}) != it, std::runtime_error,
                           MessageMatches(StartsWith("Unable to compare an iterator of type")));
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
