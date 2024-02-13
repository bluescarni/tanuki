#include <algorithm>
#include <concepts>
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
    using fts = facade::forward_time_series<double, double>;

    using vec_t = std::vector<std::pair<double, double>>;

    {
        fts f{vec_t{}};
        REQUIRE(f.begin() == f.end());
    }

    {
        // fts f{123.};
        REQUIRE(!std::constructible_from<fts, double>);
    }

    {
        minimal_forward_ts mfts{{{1, 1}, {2, 2}, {3, 3}}};

        fts f1{mfts};
        fts f2{vec_t{{1, 1}, {2, 2}, {3, 3}}};

        REQUIRE(std::ranges::equal(f1, f2));
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
