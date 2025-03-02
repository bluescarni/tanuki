#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "time_series.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)

struct minimal_forward_ts {
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
        friend bool operator==(const iterator_impl &a, const iterator_impl &b)
        {
            return a.m_ptr == b.m_ptr;
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

TEST_CASE("forward_time_series")
{
    using facade::make_forward_ts;

    using vec_t = std::vector<std::pair<double, double>>;

    {
        auto f = make_forward_ts(vec_t{});
        REQUIRE(f.begin() == f.end());
        REQUIRE(facade::any_forward_ts<decltype(f)>);
    }

    REQUIRE(!facade::ud_forward_ts<double>);

    {
        auto f1 = make_forward_ts(minimal_forward_ts{{{1, 1}, {2, 2}, {3, 3}}});
        auto f2 = make_forward_ts(vec_t{{1, 1}, {2, 2}, {3, 3}});

        REQUIRE(std::same_as<decltype(f1), decltype(f2)>);
        REQUIRE(std::ranges::equal(f1, f2));
        REQUIRE(std::ranges::equal(std::as_const(f1), f2));
        REQUIRE(std::ranges::equal(f1, std::as_const(f2)));
        REQUIRE(std::ranges::equal(std::as_const(f1), std::as_const(f2)));
        REQUIRE(facade::any_forward_ts<decltype(f1)>);
    }
}

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

template <typename TS, typename Key, std::indirect_strict_weak_order<const Key *, const Key *> Comp = std::ranges::less>
    requires facade::any_random_access_ts<TS> && std::same_as<Key, facade::ts_key_t<TS>>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
static auto lagrange_interpolation(TS &&ts, const Key &key, std::size_t order, Comp comp = {})
{
    assert(order >= 2u);
    const auto order_half = static_cast<std::ranges::range_difference_t<TS>>(order / 2u);

    // Projection to extract a reference to the key from
    // a time series record.
    const auto proj = [](const auto &p) -> const auto & { return std::get<0>(p); };

    // Locate the first key in ts which is greater than the input key.
    const auto center_it = std::ranges::upper_bound(ts, key, comp, proj);

    // How much can we move left and right of center_it without exiting ts?
    const auto max_distance_right = std::ranges::distance(center_it, std::ranges::end(ts));
    const auto max_distance_left = std::ranges::distance(std::ranges::begin(ts), center_it);

    // Establish the interpolation interval.
    const auto lag_begin
        = std::ranges::prev(center_it, (order_half > max_distance_left) ? max_distance_left : order_half);
    const auto lag_end
        = std::ranges::next(center_it, (order_half > max_distance_right) ? max_distance_right : order_half);

    // The value type used for interpolation.
    using value_t = facade::ts_value_t<TS>;

    // Run the interpolation.
    value_t result{};

    for (auto outer = lag_begin; outer != lag_end; ++outer) {
        auto term = std::get<1>(*outer);

        for (auto inner = lag_begin; inner != lag_end; ++inner) {
            if (outer != inner) {
                term *= (key - std::get<0>(*inner)) / (std::get<0>(*outer) - std::get<0>(*inner));
            }
        }

        if (outer == lag_begin) {
            result = term;
        } else {
            result += term;
        }
    }

    return result;
}

TEST_CASE("lagrange interpolation")
{
    using facade::make_random_access_ts;

    using vec_t = std::vector<std::pair<double, double>>;

    {
        auto f = make_random_access_ts(minimal_ra_ts{vec_t{}});
        REQUIRE(f.begin() == f.end());
        REQUIRE(facade::any_random_access_ts<decltype(f)>);

        lagrange_interpolation(f, 5., 2);
    }

    {
        const vec_t v{{1, 2}, {2, 4}, {3, 6}, {4, 8}};

        auto f = make_random_access_ts(minimal_ra_ts{v});

        REQUIRE(lagrange_interpolation(f, 2.5, 2) == 5.);
    }

    using vvec_t = std::vector<std::pair<double, std::vector<double>>>;

    {
        const vvec_t v{{1, {2, 0}}, {2, {4, 0}}, {3, {6, 0}}, {4, {8, 0}}};

        auto tmp = make_random_access_ts(v);
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

        const auto tmp2 = make_random_access_ts(std::ref(v));
        REQUIRE(lagrange_interpolation(tmp2 | std::ranges::views::transform([](const auto &p) {
                                           return std::make_pair(p.first, p.second[0]);
                                       }),
                                       2.5, 2)
                == 5.);

#if __cpp_lib_ranges > 202106L
        // NOTE: post C++20 rvalue ranges can be used to construct views pipelines. See:
        // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2415r2.html

        [[maybe_unused]] auto tmp_rvalue = make_random_access_ts(v) | std::ranges::views::transform([](const auto &p) {
                                               return std::make_pair(p.first, p.second[0]);
                                           });
#endif
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,bugprone-crtp-constructor-accessibility)
