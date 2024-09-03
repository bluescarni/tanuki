// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_TIME_SERIES_HPP
#define FACADE_TIME_SERIES_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>

namespace facade
{

struct key_proj {
    const auto &operator()(const auto &p) const
    {
        return std::get<0>(p);
    }
};

struct value_proj {
    const auto &operator()(const auto &p) const
    {
        return std::get<1>(p);
    }
};

template <typename TS, typename Key, typename KeyProj = key_proj, typename ValueProj = value_proj,
          std::indirect_strict_weak_order<const Key *, std::projected<std::ranges::iterator_t<TS>, KeyProj>> Comp
          = std::ranges::less>
    requires std::ranges::random_access_range<TS>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
auto lagrange_interpolation(TS &&ts, const Key &key, std::size_t order, Comp comp = {}, KeyProj kproj = {},
                            ValueProj vproj = {})
{
    assert(order >= 2u);
    const auto order_half = static_cast<std::ranges::range_difference_t<TS>>(order / 2u);

    // Locate the first key in ts which is greater than the input key.
    const auto center_it = std::ranges::upper_bound(ts, key, comp, kproj);

    // How much can we move left and right of center_it without exiting ts?
    const auto max_distance_right = std::ranges::distance(center_it, std::ranges::end(ts));
    const auto max_distance_left = std::ranges::distance(std::ranges::begin(ts), center_it);

    // Establish the interpolation interval.
    const auto lag_begin
        = std::ranges::prev(center_it, (order_half > max_distance_left) ? max_distance_left : order_half);
    const auto lag_end
        = std::ranges::next(center_it, (order_half > max_distance_right) ? max_distance_right : order_half);

    // The value type used for interpolation.
    using value_t = typename std::projected<std::ranges::iterator_t<TS>, ValueProj>::value_type;

    // Run the interpolation.
    value_t result{};

    for (auto outer = lag_begin; outer != lag_end; ++outer) {
        auto term = std::invoke(vproj, *outer);

        for (auto inner = lag_begin; inner != lag_end; ++inner) {
            if (outer != inner) {
                term *= (key - std::invoke(kproj, *inner)) / (std::invoke(kproj, *outer) - std::invoke(kproj, *inner));
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

} // namespace facade

#endif
