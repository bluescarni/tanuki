#include <concepts>
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
        std::list<int> lst = {1, 2, 3};
        int_iter it(std::begin(lst));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
