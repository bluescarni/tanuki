#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <tanuki/tanuki.hpp>

#include "ranges.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// LCOV_EXCL_START

// Minimal-interface forward iterators.
struct min_fw_it {
    std::vector<int>::iterator it{};

    int &operator*() const
    {
        return *it;
    }
    void operator++()
    {
        ++it;
    }
    friend bool operator==(const min_fw_it &a, const min_fw_it &b)
    {
        return a.it == b.it;
    }
};

struct min_fw_cit {
    std::vector<int>::const_iterator it{};

    const int &operator*() const
    {
        return *it;
    }
    void operator++()
    {
        ++it;
    }
    friend bool operator==(const min_fw_cit &a, const min_fw_cit &b)
    {
        return a.it == b.it;
    }
};

// Minimal-interface forward range.
struct min_fw_range {
    std::vector<int> vec;

    auto begin()
    {
        return min_fw_it{vec.begin()};
    }
    auto end()
    {
        return min_fw_it{vec.end()};
    }

    [[nodiscard]] auto begin() const
    {
        return min_fw_cit{vec.begin()};
    }
    [[nodiscard]] auto end() const
    {
        return min_fw_cit{vec.end()};
    }
};

// LCOV_EXCL_STOP

TEST_CASE("basic forward")
{
    {
        std::vector vec = {1, 2, 3};

        auto r1 = facade::make_forward_range(vec);
        REQUIRE(std::ranges::forward_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::forward_range<int, int &, int &&, const int &, const int &&>>);

        auto r2 = facade::make_forward_range(std::as_const(vec));
        REQUIRE(std::ranges::forward_range<decltype(r2)>);
        REQUIRE(std::same_as<decltype(r2), facade::forward_range<int, int &, int &&, const int &, const int &&>>);

        auto *orig_data = vec.data();
        auto r3 = facade::make_forward_range(std::move(vec));
        REQUIRE(value_ref<std::vector<int>>(r3).data() == orig_data);

        std::vector vec2 = {1, 2, 3};
        auto r4 = facade::make_forward_range(std::cref(vec2));
        REQUIRE(has_static_storage(r4));
        REQUIRE(std::ranges::forward_range<decltype(r4)>);
        REQUIRE(std::same_as<decltype(r4),
                             facade::forward_range<int, const int &, const int &&, const int &, const int &&>>);
        REQUIRE(&*r4.begin() == vec2.data());

        std::vector vec3 = {1, 2, 3};
        auto r5 = facade::make_forward_range(std::ref(vec3));
        REQUIRE(std::same_as<decltype(r5), facade::forward_range<int, int &, int &&, const int &, const int &&>>);
        REQUIRE(&*r5.begin() == vec3.data());

        REQUIRE(std::ranges::equal(vec3, r5));

        auto r6 = facade::make_forward_range(std::ranges::subrange(vec3.begin(), vec3.end()));
        REQUIRE(std::same_as<decltype(r6), facade::forward_range<int, int &, int &&, int &, int &&>>);
        REQUIRE(std::ranges::equal(r5, r6));
        REQUIRE(has_static_storage(r6));
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        min_fw_range vec{{1, 2, 3}};

        REQUIRE(!std::ranges::range<min_fw_range>);
        auto r1 = facade::make_forward_range(vec);
        REQUIRE(std::ranges::forward_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1), facade::forward_range<int, int &, int &&, const int &, const int &&>>);
    }

    {
        const min_fw_range vec{{1, 2, 3}};

        REQUIRE(!std::ranges::range<min_fw_range>);
        auto r1 = facade::make_forward_range(std::ref(vec));
        REQUIRE(std::ranges::forward_range<decltype(r1)>);
        REQUIRE(std::same_as<decltype(r1),
                             facade::forward_range<int, const int &, const int &&, const int &, const int &&>>);
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
