#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "ranges.hpp"
#include "tanuki/tanuki.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic forward")
{
    {
        std::vector vec = {1, 2, 3};

        auto r1 = facade::make_forward_range(vec);
        REQUIRE(std::ranges::forward_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::forward_range<int, int &, int &&>>);

        auto r2 = facade::make_forward_range(std::as_const(vec));
        REQUIRE(std::ranges::forward_range<decltype(r2)>);
        REQUIRE(std::same_as<decltype(r2), facade::forward_range<int, int &, int &&>>);

        auto *orig_data = vec.data();
        auto r3 = facade::make_forward_range(std::move(vec));
        REQUIRE(value_ref<std::vector<int>>(r3).data() == orig_data);

        std::vector vec2 = {1, 2, 3};
        auto r4 = facade::make_forward_range(std::cref(vec2));
        REQUIRE(has_static_storage(r4));
        REQUIRE(std::ranges::forward_range<decltype(r4)>);
        REQUIRE(std::same_as<decltype(r4), facade::forward_range<int, const int &, const int &&>>);
        REQUIRE(&*r4.begin() == vec2.data());

        std::vector vec3 = {1, 2, 3};
        auto r5 = facade::make_forward_range(std::ref(vec3));
        REQUIRE(std::same_as<decltype(r5), facade::forward_range<int, int &, int &&>>);
        REQUIRE(&*r5.begin() == vec3.data());

        REQUIRE(std::ranges::equal(vec3, r5));

        auto r6 = facade::make_forward_range(std::ranges::subrange(vec3.begin(), vec3.end()));
        REQUIRE(std::ranges::equal(r5, r6));
        REQUIRE(has_static_storage(r6));
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
