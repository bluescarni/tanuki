#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "io_iterator.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic")
{
    using int_iter = facade::io_iterator<int &>;

    REQUIRE(std::input_or_output_iterator<int_iter>);
    REQUIRE(!std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);

    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);

    {
        int arr[] = {1, 2, 3};
        int_iter it(std::begin(arr));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);
    }

    {
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_io_iterator(std::begin(vec));
        REQUIRE(std::same_as<decltype(it), int_iter>);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_io_iterator(vec.cbegin());
        REQUIRE(std::same_as<decltype(it), facade::io_iterator<const int &>>);
        REQUIRE(std::input_or_output_iterator<decltype(it)>);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);
    }

    {
        std::list<int> lst = {1, 2, 3};
        int_iter it(std::begin(lst));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);
    }
}

struct noniter {
    double operator*() const
    {
        return {};
    }
    void operator++() {}
};

TEST_CASE("noniter")
{
    using iter_t = facade::io_iterator<double>;

    REQUIRE(std::same_as<iter_t, decltype(facade::make_io_iterator(noniter{}))>);

    REQUIRE(std::input_or_output_iterator<iter_t>);
    REQUIRE(!std::default_initializable<iter_t>);
    REQUIRE(!std::constructible_from<iter_t, int>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)