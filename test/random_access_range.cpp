#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <tanuki/tanuki.hpp>

#include "ranges.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// LCOV_EXCL_START

// Minimal-interface random-access iterator.
template <bool Const>
struct min_ra_it {
    std::conditional_t<Const, std::vector<int>::const_iterator, std::vector<int>::iterator> it{};

    auto &operator*() const
    {
        return *it;
    }
    void operator++()
    {
        ++it;
    }
    void operator--()
    {
        --it;
    }
    void operator+=(std::ptrdiff_t n)
    {
        it += n;
    }
    void operator-=(std::ptrdiff_t n)
    {
        it -= n;
    }
    [[nodiscard]] friend std::ptrdiff_t operator-(const min_ra_it &a, const min_ra_it &b)
    {
        return a.it - b.it;
    }
    friend bool operator==(const min_ra_it &a, const min_ra_it &b)
    {
        return a.it == b.it;
    }
    friend bool operator<(const min_ra_it &a, const min_ra_it &b)
    {
        return a.it < b.it;
    }
};

// Minimal-interface random-access range.
struct min_ra_range {
    std::vector<int> vec;

    auto begin()
    {
        return min_ra_it<false>{vec.begin()};
    }
    auto end()
    {
        return min_ra_it<false>{vec.end()};
    }

    [[nodiscard]] auto begin() const
    {
        return min_ra_it<true>{vec.begin()};
    }
    [[nodiscard]] auto end() const
    {
        return min_ra_it<true>{vec.end()};
    }
};

// LCOV_EXCL_STOP

TEST_CASE("basic random_access")
{
    {
        std::vector vec = {1, 2, 3};

        auto r1 = facade::make_random_access_range(vec);
        REQUIRE(std::ranges::random_access_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::random_access_range<int, int &, int &&, const int &, const int &&>>);

        auto r2 = facade::make_random_access_range(std::as_const(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r2)>);
        REQUIRE(std::same_as<decltype(r2), facade::random_access_range<int, int &, int &&, const int &, const int &&>>);

        auto *orig_data = vec.data();
        auto r3 = facade::make_random_access_range(std::move(vec));
        REQUIRE(value_ref<std::vector<int>>(r3).data() == orig_data);

        std::vector vec2 = {1, 2, 3};
        const auto r4 = facade::make_random_access_range(std::cref(vec2));
        REQUIRE(has_static_storage(r4));
        REQUIRE(std::ranges::random_access_range<decltype(r4)>);
        REQUIRE(
            std::same_as<decltype(r4),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);
        REQUIRE(&*r4.begin() == vec2.data());

        std::vector vec3 = {1, 2, 3};
        auto r5 = facade::make_random_access_range(std::ref(vec3));
        REQUIRE(std::same_as<decltype(r5), facade::random_access_range<int, int &, int &&, const int &, const int &&>>);
        REQUIRE(&*r5.begin() == vec3.data());

        REQUIRE(std::ranges::equal(vec3, r5));

        auto r6 = facade::make_random_access_range(std::ranges::subrange(vec3.begin(), vec3.end()));
        REQUIRE(std::same_as<decltype(r6), facade::random_access_range<int, int &, int &&, int &, int &&>>);
        REQUIRE(std::ranges::equal(r5, r6));
        REQUIRE(std::ranges::equal(std::as_const(r5), r6));
        REQUIRE(std::ranges::equal(std::as_const(r5), std::as_const(r6)));
        REQUIRE(has_static_storage(r6));
    }

    {
        int vec[] = {1, 2, 3};

        auto r1 = facade::make_random_access_range(std::ref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::random_access_range<int, int &, int &&, const int &, const int &&>>);

        const auto r2 = facade::make_random_access_range(std::cref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r2)>);
        REQUIRE(
            std::same_as<decltype(r2),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);

        REQUIRE(std::ranges::equal(r1, r2));
    }

    {
        const int vec[] = {1, 2, 3};

        const auto r1 = facade::make_random_access_range(std::ref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r1)>);
        REQUIRE(
            std::same_as<decltype(r1),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);

        const auto r2 = facade::make_random_access_range(std::cref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r2)>);
        REQUIRE(
            std::same_as<decltype(r2),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);

        REQUIRE(std::ranges::equal(r1, r2));
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        min_ra_range vec{{3, 1, 2}};

        REQUIRE(!std::ranges::range<min_ra_range>);
        auto r1 = facade::make_random_access_range(vec);
        REQUIRE(std::ranges::random_access_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::random_access_range<int, int &, int &&, const int &, const int &&>>);

        std::ranges::sort(r1);
        REQUIRE(std::ranges::equal(r1, std::vector{1, 2, 3}));
    }

    {
        const min_ra_range vec{{1, 2, 3}};

        REQUIRE(!std::ranges::range<min_ra_range>);
        const auto r1 = facade::make_random_access_range(std::ref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r1)>);
        REQUIRE(
            std::same_as<decltype(r1),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);
        REQUIRE(has_static_storage(r1));
        REQUIRE(&*std::ranges::begin(r1) == vec.vec.data());
        REQUIRE(std::ranges::equal(vec.vec, r1));
        REQUIRE(std::ranges::equal(vec.vec, std::as_const(r1)));
        REQUIRE(std::ranges::distance(std::ranges::begin(r1), std::ranges::end(r1)) == 3);
    }

    {
        int vec[] = {1, 2, 3};

        auto r1 = facade::make_random_access_range(std::ref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::random_access_range<int, int &, int &&, const int &, const int &&>>);

        const auto r2 = facade::make_random_access_range(std::cref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r2)>);
        REQUIRE(
            std::same_as<decltype(r2),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);

        REQUIRE(std::ranges::equal(r1, r2));
    }

    {
        const int vec[] = {1, 2, 3};

        const auto r1 = facade::make_random_access_range(std::ref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r1)>);
        REQUIRE(
            std::same_as<decltype(r1),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);

        const auto r2 = facade::make_random_access_range(std::cref(vec));
        REQUIRE(std::ranges::random_access_range<decltype(r2)>);
        REQUIRE(
            std::same_as<decltype(r2),
                         const facade::random_access_range<int, const int &, const int &&, const int &, const int &&>>);

        REQUIRE(std::ranges::equal(r1, r2));
    }
}

TEST_CASE("nested type erasure")
{
    const std::vector vec = {1, 2, 3};

    // This test checks that type-erasing a range which already returns
    // type-erased iterators works as expected (i.e., the type-erased iterators
    // are re-type-erased intead of being copied).
    {
        auto tmp = facade::make_random_access_range(vec);
        auto r1 = facade::make_random_access_range(tmp | std::views::transform(std::identity{}));
        REQUIRE(*std::ranges::lower_bound(r1, 1) == 1);
    }

    // This test checks that type-erasing a type-erased range containing
    // a const reference works as expected (specifically, type-erasing
    // a const reference results in a range which returns const iterators
    // for both overloads of begin()).
    {
        auto tmp = facade::make_random_access_range(std::ref(vec));
        auto r1 = facade::make_random_access_range(tmp | std::views::transform(std::identity{}));
        REQUIRE(*std::ranges::lower_bound(r1, 1) == 1);
    }
}

TEST_CASE("counted iterator")
{
    const std::vector vec = {1, 2, 3};

    auto sr = std::ranges::subrange(std::counted_iterator{std::cbegin(vec), 3}, std::default_sentinel);
    auto tmp = facade::make_random_access_range(sr);

    REQUIRE(*std::ranges::lower_bound(tmp, 1) == 1);
}

TEST_CASE("sentinel")
{
    using Catch::Matchers::ContainsSubstring;
    using Catch::Matchers::MessageMatches;
    using Catch::Matchers::StartsWith;

    auto r1 = facade::make_random_access_range(std::vector{1, 2, 3});
    auto r2 = facade::make_random_access_range(std::array{1, 2, 3});

    REQUIRE_THROWS_MATCHES(r1.begin() - r2.end(), std::runtime_error,
                           MessageMatches(StartsWith("Unable to compute the distance between an iterator of type '")));
    REQUIRE_THROWS_MATCHES(r1.begin() - r2.end(), std::runtime_error,
                           MessageMatches(ContainsSubstring("' and a sentinel of type '")));
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
