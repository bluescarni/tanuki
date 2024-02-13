// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_TIME_SERIES_HPP
#define FACADE_TIME_SERIES_HPP

#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

#include "forward_iterator.hpp"
#include "ranges.hpp"

namespace facade
{

template <typename K, typename V>
class forward_time_series
{
public:
    using value_type = std::pair<K, V>;
    using iterator = forward_iterator<value_type, value_type &, value_type &&>;
    using const_iterator = forward_iterator<value_type, const value_type &, const value_type &&>;

private:
    using range_t = forward_range<value_type, value_type &, value_type &&, const value_type &, const value_type &&>;

    range_t m_range;

public:
    template <typename R>
        requires(!std::same_as<std::remove_cvref_t<R>, forward_time_series>) && std::constructible_from<range_t, R &&>
    explicit forward_time_series(R &&r) : m_range(std::forward<R>(r))
    {
    }

    forward_time_series(const forward_time_series &) = default;
    forward_time_series(forward_time_series &&) noexcept = default;
    forward_time_series &operator=(const forward_time_series &) = default;
    forward_time_series &operator=(forward_time_series &&) noexcept = default;
    ~forward_time_series() = default;

    iterator begin()
    {
        return std::ranges::begin(m_range);
    }
    iterator end()
    {
        return std::ranges::end(m_range);
    }

    const_iterator begin() const
    {
        return std::ranges::begin(m_range);
    }
    const_iterator end() const
    {
        return std::ranges::end(m_range);
    }
};

} // namespace facade

#endif
