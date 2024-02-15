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

#include "ranges.hpp"

namespace facade
{

namespace detail
{

template <typename T>
concept is_ts_key = std::totally_ordered<T>;

template <typename>
struct is_ts_pair : std::false_type {
};

template <typename T, typename U>
struct is_ts_pair<std::pair<T, U>> : std::bool_constant<is_ts_key<T>> {
};

template <typename>
struct any_forward_ts_impl : std::false_type {
};

template <typename V, typename R, typename RR, typename CR, typename CRR>
struct any_forward_ts_impl<forward_range<V, R, RR, CR, CRR>> : is_ts_pair<V> {
};

template <typename>
struct any_random_access_ts_impl : std::false_type {
};

template <typename V, typename R, typename RR, typename CR, typename CRR>
struct any_random_access_ts_impl<random_access_range<V, R, RR, CR, CRR>> : is_ts_pair<V> {
};

} // namespace detail

template <typename T>
concept ud_forward_ts = requires(T &&x) {
    make_forward_range(std::forward<T>(x));
    requires detail::is_ts_pair<std::ranges::range_value_t<decltype(make_forward_range(std::forward<T>(x)))>>::value;
};

template <typename T>
    requires ud_forward_ts<T>
auto make_forward_ts(T &&x)
{
    return make_forward_range(std::forward<T>(x));
}

template <typename T>
concept any_forward_ts = detail::any_forward_ts_impl<T>::value;

template <typename T>
concept ud_random_access_ts = requires(T &&x) {
    make_random_access_range(std::forward<T>(x));
    requires detail::is_ts_pair<
        std::ranges::range_value_t<decltype(make_random_access_range(std::forward<T>(x)))>>::value;
};

template <typename T>
    requires ud_random_access_ts<T>
auto make_random_access_ts(T &&x)
{
    return make_random_access_range(std::forward<T>(x));
}

template <typename T>
concept any_random_access_ts = detail::any_random_access_ts_impl<T>::value;

} // namespace facade

#endif
