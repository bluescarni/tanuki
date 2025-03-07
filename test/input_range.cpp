#include <algorithm>
#include <concepts>
#include <functional>
#include <list>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <tanuki/tanuki.hpp>

#include "ranges.hpp"
#include "sentinel.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// LCOV_EXCL_START

// Minimal-interface forward iterator.
template <bool Const>
struct min_input_it {
    using iter_t = std::conditional_t<Const, std::vector<int>::const_iterator, std::vector<int>::iterator>;

    iter_t it{};

    auto &operator*() const
    {
        return *it;
    }
    void operator++()
    {
        ++it;
    }

    bool operator==(const min_input_it &other) const
    {
        return it == other.it;
    }
};

// Minimal-interface forward range.
struct min_input_range {
    std::vector<int> vec;

    auto begin()
    {
        return min_input_it<false>{vec.begin()};
    }
    // NOLINTNEXTLINE
    auto end()
    {
        return min_input_it<false>{vec.end()};
    }

    [[nodiscard]] auto begin() const
    {
        return min_input_it<true>{vec.begin()};
    }
    // NOLINTNEXTLINE
    [[nodiscard]] auto end() const
    {
        return min_input_it<true>{vec.end()};
    }
};

// LCOV_EXCL_STOP

TEST_CASE("basic input")
{
    {
        std::vector vec = {1, 2, 3};

        auto r1 = facade::make_input_range(vec);
        REQUIRE(std::ranges::input_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::input_range<int, int &, int &&, const int &, const int &&>>);

        auto r2 = facade::make_input_range(std::as_const(vec));
        REQUIRE(std::ranges::input_range<decltype(r2)>);
        REQUIRE(std::same_as<decltype(r2), facade::input_range<int, int &, int &&, const int &, const int &&>>);

        auto *orig_data = vec.data();
        auto r3 = facade::make_input_range(std::move(vec));
        REQUIRE(value_ref<std::vector<int>>(r3).data() == orig_data);

        std::vector vec2 = {1, 2, 3};
        const auto r4 = facade::make_input_range(std::cref(vec2));
        REQUIRE(has_static_storage(r4));
        REQUIRE(std::ranges::input_range<decltype(r4)>);
        REQUIRE(std::same_as<decltype(r4),
                             const facade::input_range<int, const int &, const int &&, const int &, const int &&>>);
        REQUIRE(&*r4.begin() == vec2.data());

        std::vector vec3 = {1, 2, 3};
        auto r5 = facade::make_input_range(std::ref(vec3));
        REQUIRE(std::same_as<decltype(r5), facade::input_range<int, int &, int &&, const int &, const int &&>>);
        REQUIRE(&*r5.begin() == vec3.data());

        REQUIRE(std::ranges::equal(vec3, r5));

        auto r6 = facade::make_input_range(std::ranges::subrange(vec3.begin(), vec3.end()));
        REQUIRE(std::same_as<decltype(r6), facade::input_range<int, int &, int &&, int &, int &&>>);
        REQUIRE(std::ranges::equal(r5, r6));
        REQUIRE(std::ranges::equal(std::as_const(r5), r6));
        REQUIRE(std::ranges::equal(std::as_const(r5), std::as_const(r6)));
        REQUIRE(has_static_storage(r6));
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        min_input_range vec{{1, 2, 3}};

        REQUIRE(!std::ranges::range<min_input_range>);
        auto r1 = facade::make_input_range(vec);
        REQUIRE(std::ranges::input_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::input_range<int, int &, int &&, const int &, const int &&>>);
    }

    {
        const min_input_range vec{{1, 2, 3}};

        REQUIRE(!std::ranges::range<min_input_range>);
        const auto r1 = facade::make_input_range(std::ref(vec));
        REQUIRE(std::ranges::input_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1),
                             const facade::input_range<int, const int &, const int &&, const int &, const int &&>>);
        REQUIRE(has_static_storage(r1));
        REQUIRE(&*std::ranges::begin(r1) == vec.vec.data());
        REQUIRE(std::ranges::equal(vec.vec, r1));
        REQUIRE(std::ranges::equal(vec.vec, std::as_const(r1)));
    }
}

TEST_CASE("sentinel")
{
    using Catch::Matchers::ContainsSubstring;
    using Catch::Matchers::MessageMatches;
    using Catch::Matchers::StartsWith;

    auto r1 = facade::make_input_range(std::vector{1, 2, 3});
    auto r2 = facade::make_input_range(std::list{1, 2, 3});

    REQUIRE_THROWS_MATCHES(r1.begin() == r2.end(), std::runtime_error,
                           MessageMatches(StartsWith("Unable to compare an iterator of type '")));
    REQUIRE_THROWS_MATCHES(r1.begin() == r2.end(), std::runtime_error,
                           MessageMatches(ContainsSubstring("' to a sentinel of type '")));

    std::list l{1, 2, 3};
    auto s
        = facade::sentinel(facade::detail::sentinel_box<std::list<int>::iterator, std::list<int>::iterator>{l.begin()});

    auto it = l.begin();
    REQUIRE_THROWS_MATCHES(s->distance_to_iter(facade::detail::any_ref{std::cref(it)}), std::runtime_error,
                           MessageMatches(StartsWith("The sentinel type '")));
    REQUIRE_THROWS_MATCHES(s->distance_to_iter(facade::detail::any_ref{std::cref(it)}), std::runtime_error,
                           MessageMatches(ContainsSubstring("' is not a sized sentinel")));
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
