#include <algorithm>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <vector>

#include <tanuki/tanuki.hpp>

#include "ranges.hpp"

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basics")
{
    std::vector<int> v = {1, 2, 3};

    {
        auto r1 = facade::make_random_access_range(std::ref(v));
        auto r2 = std::ranges::subrange(r1.begin(), r1.begin() + 2);
        auto r3 = facade::make_random_access_range(std::ref(r2));
        REQUIRE(std::ranges::equal(r3, r2));
    }

    {
        auto r1 = facade::make_random_access_range(std::ref(v));
        auto r2 = std::ranges::subrange(r1.begin(), r1.end());
        auto r3 = facade::make_random_access_range(std::ref(r2));
        REQUIRE(std::ranges::equal(r3, v));
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
