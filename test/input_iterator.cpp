#include <concepts>
#include <iterator>
#include <list>
#include <vector>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "input_iterator.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic")
{
    using int_iter = facade::input_iterator<int, int &, int &&>;

    static_assert(std::input_iterator<int_iter>);

    {
        int arr[] = {1, 2, 3};
        int_iter it(std::begin(arr));
        facade::iter_move(it);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
