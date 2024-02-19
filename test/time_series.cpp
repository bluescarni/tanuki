#include <algorithm>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "time_series.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

struct minimal_forward_ts {
    std::vector<std::pair<double, double>> m_container;

    struct iterator {
        std::pair<double, double> *m_ptr;

        auto &operator*() const
        {
            return *m_ptr;
        }
        void operator++()
        {
            ++m_ptr;
        }
        friend bool operator==(const iterator &a, const iterator &b)
        {
            return a.m_ptr == b.m_ptr;
        }
    };

    struct const_iterator {
        const std::pair<double, double> *m_ptr;

        auto &operator*() const
        {
            return *m_ptr;
        }
        void operator++()
        {
            ++m_ptr;
        }
        friend bool operator==(const const_iterator &a, const const_iterator &b)
        {
            return a.m_ptr == b.m_ptr;
        }
    };

    auto begin()
    {
        return iterator{m_container.data()};
    }
    auto end()
    {
        return iterator{m_container.data() + m_container.size()};
    }

    [[nodiscard]] auto begin() const
    {
        return const_iterator{m_container.data()};
    }
    [[nodiscard]] auto end() const
    {
        return const_iterator{m_container.data() + m_container.size()};
    }
};

TEST_CASE("forward_time_series")
{
    using facade::make_forward_ts;

    using vec_t = std::vector<std::pair<double, double>>;

    {
        auto f = make_forward_ts(vec_t{});
        REQUIRE(f.begin() == f.end());
        REQUIRE(facade::any_forward_ts<decltype(f)>);
    }

    // forward_ts(123.);
    REQUIRE(!facade::ud_forward_ts<double>);

    {
        auto f1 = make_forward_ts(minimal_forward_ts{{{1, 1}, {2, 2}, {3, 3}}});
        auto f2 = make_forward_ts(vec_t{{1, 1}, {2, 2}, {3, 3}});

        REQUIRE(std::same_as<decltype(f1), decltype(f2)>);
        REQUIRE(std::ranges::equal(f1, f2));
        REQUIRE(facade::any_forward_ts<decltype(f1)>);
    }
}

struct minimal_ra_ts {
    std::vector<std::pair<double, double>> m_container;

    struct iterator {
        std::pair<double, double> *m_ptr;

        auto &operator*() const
        {
            return *m_ptr;
        }
        void operator++()
        {
            ++m_ptr;
        }
        friend bool operator==(const iterator &a, const iterator &b)
        {
            return a.m_ptr == b.m_ptr;
        }
    };

    struct const_iterator {
        const std::pair<double, double> *m_ptr;

        auto &operator*() const
        {
            return *m_ptr;
        }
        void operator++()
        {
            ++m_ptr;
        }
        friend bool operator==(const const_iterator &a, const const_iterator &b)
        {
            return a.m_ptr == b.m_ptr;
        }
    };

    auto begin()
    {
        return iterator{m_container.data()};
    }
    auto end()
    {
        return iterator{m_container.data() + m_container.size()};
    }

    [[nodiscard]] auto begin() const
    {
        return const_iterator{m_container.data()};
    }
    [[nodiscard]] auto end() const
    {
        return const_iterator{m_container.data() + m_container.size()};
    }
};

template <typename TS, typename Key>
    requires std::ranges::random_access_range<TS> && (facade::detail::is_ts_pair<std::ranges::range_value_t<TS>>::value)
auto lagrange_interpolation(TS &&ts, const Key &key, std::size_t order)
{
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
