#include <algorithm>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <vector>

#include <tanuki/tanuki.hpp>

#include "ranges.hpp"

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// NOTE: the purpose of this test is to check that
// the range type erasure machinery works properly
// when type-erasing non-type-erased ranges that return
// type-erased iterators (say that three times).
//
// Before fixing the implementation of type-erased ranges,
// the following would happen:
// - no type-erasure would take place in the begin()
//   method of generic_range due to the use of make_generic_iterator(),
//   which would detect that the user-defined iterator is already
//   type-erased and which would thus return a copy of the user-defined
//   iterator;
// - type-erasure would however always happen in the end() implementation,
//   and this would break the logic in the sentinel methods that need to
//   access the user-defined iterator stored in the wrap.
TEST_CASE("type erased subranges")
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

    {
        const auto r1 = facade::make_random_access_range(std::cref(v));
        auto r2 = std::ranges::subrange(r1.begin(), r1.begin() + 2);
        const auto r3 = facade::make_random_access_range(std::cref(r2));
        REQUIRE(std::ranges::equal(r3, r2));
    }

    {
        const auto r1 = facade::make_random_access_range(std::cref(v));
        auto r2 = std::ranges::subrange(r1.begin(), r1.end());
        const auto r3 = facade::make_random_access_range(std::cref(r2));
        REQUIRE(std::ranges::equal(r3, v));
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
