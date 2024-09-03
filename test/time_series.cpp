// #include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "ranges.hpp"
#include "time_series.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// LCOV_EXCL_START

struct minimal_ra_ts {
    std::vector<std::pair<double, double>> m_container;

    template <bool Const>
    struct iterator_impl {
        std::conditional_t<Const, const std::pair<double, double> *, std::pair<double, double> *> m_ptr;

        auto &operator*() const
        {
            return *m_ptr;
        }
        void operator++()
        {
            ++m_ptr;
        }
        void operator--()
        {
            --m_ptr;
        }
        void operator+=(std::ptrdiff_t n)
        {
            m_ptr += n;
        }
        void operator-=(std::ptrdiff_t n)
        {
            m_ptr -= n;
        }
        [[nodiscard]] friend std::ptrdiff_t operator-(const iterator_impl &a, const iterator_impl &b)
        {
            return a.m_ptr - b.m_ptr;
        }
        friend bool operator==(const iterator_impl &a, const iterator_impl &b)
        {
            return a.m_ptr == b.m_ptr;
        }
        friend bool operator<(const iterator_impl &a, const iterator_impl &b)
        {
            return a.m_ptr < b.m_ptr;
        }
    };

    using iterator = iterator_impl<false>;
    using const_iterator = iterator_impl<true>;

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

// LCOV_EXCL_STOP

TEST_CASE("lagrange interpolation")
{
    using facade::lagrange_interpolation;
    using facade::make_random_access_range;

    using vec_t = std::vector<std::pair<double, double>>;

    {
        auto f = make_random_access_range(minimal_ra_ts{vec_t{}});
        REQUIRE(f.begin() == f.end());

        lagrange_interpolation(f, 5., 2);
    }

    {
        const vec_t v{{1, 2}, {2, 4}, {3, 6}, {4, 8}};

        auto f = make_random_access_range(minimal_ra_ts{v});

        REQUIRE(lagrange_interpolation(f, 2.5, 2) == 5.);
    }

    using vvec_t = std::vector<std::pair<double, std::vector<double>>>;

    {
        const vvec_t v{{1, {2, 0}}, {2, {4, 0}}, {3, {6, 0}}, {4, {8, 0}}};

        auto tmp = make_random_access_range(v);
        REQUIRE(lagrange_interpolation(tmp | std::ranges::views::transform([](const auto &p) {
                                           return std::make_pair(p.first, p.second[0]);
                                       }),
                                       2.5, 2)
                == 5.);
        REQUIRE(lagrange_interpolation(std::as_const(tmp) | std::ranges::views::transform([](const auto &p) {
                                           return std::make_pair(p.first, p.second[0]);
                                       }),
                                       2.5, 2)
                == 5.);

        const auto tmp2 = make_random_access_range(std::ref(v));
        REQUIRE(lagrange_interpolation(tmp2 | std::ranges::views::transform([](const auto &p) {
                                           return std::make_pair(p.first, p.second[0]);
                                       }),
                                       2.5, 2)
                == 5.);

#if __cpp_lib_ranges > 202106L
        // NOTE: post C++20 rvalue ranges can be used to construct views pipelines. See:
        // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2415r2.html

        [[maybe_unused]] auto tmp_rvalue
            = make_random_access_range(v)
              | std::ranges::views::transform([](const auto &p) { return std::make_pair(p.first, p.second[0]); });
#endif
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
