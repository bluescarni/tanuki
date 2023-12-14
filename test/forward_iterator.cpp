#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "forward_iterator.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic")
{
    using int_iter = facade::forward_iterator<int, int &, int &&>;

    REQUIRE(std::forward_iterator<int_iter>);
    REQUIRE(std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);

    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);
    REQUIRE(std::same_as<int, std::iter_value_t<int_iter>>);
    REQUIRE(std::same_as<int &, std::iter_reference_t<int_iter>>);
    REQUIRE(std::same_as<int &&, std::iter_rvalue_reference_t<int_iter>>);
    REQUIRE(std::same_as<std::forward_iterator_tag, int_iter::iterator_concept>);

#if 0
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
        auto it = facade::make_input_iterator(std::begin(vec));
        REQUIRE(std::same_as<decltype(it), int_iter>);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_input_iterator(vec.cbegin());
        REQUIRE(std::same_as<decltype(it), facade::input_iterator<int, const int &, const int &&>>);
        REQUIRE(std::input_iterator<decltype(it)>);
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
#endif
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
