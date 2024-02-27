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
#include <tuple>
#include <type_traits>
#include <utility>

#include "ranges.hpp"

namespace facade
{

namespace detail
{

template <typename T>
struct is_ts_record : std::false_type {
};

template <typename T, typename U>
struct is_ts_record<std::pair<T, U>> : std::true_type {
};

template <typename T, typename U>
struct is_ts_record<std::tuple<T, U>> : std::true_type {
};

} // namespace detail

// Detect if T is a time series record, that is, a std::pair or a std::tuple
// of two elements.
template <typename T>
concept ts_record = detail::is_ts_record<T>::value;

namespace detail
{

template <typename TS>
using ts_key_from_ref_t
    = std::remove_cvref_t<std::tuple_element_t<0, std::remove_cvref_t<std::ranges::range_reference_t<TS>>>>;

template <typename TS>
using ts_key_from_rref_t
    = std::remove_cvref_t<std::tuple_element_t<0, std::remove_cvref_t<std::ranges::range_rvalue_reference_t<TS>>>>;

template <typename TS>
using ts_value_from_ref_t
    = std::remove_cvref_t<std::tuple_element_t<1, std::remove_cvref_t<std::ranges::range_reference_t<TS>>>>;

template <typename TS>
using ts_value_from_rref_t
    = std::remove_cvref_t<std::tuple_element_t<1, std::remove_cvref_t<std::ranges::range_rvalue_reference_t<TS>>>>;

} // namespace detail

// Alias to fetch the key type of a time series.
template <typename TS>
using ts_key_t = detail::ts_key_from_ref_t<TS>;

// Alias to fetch the value type of a time series.
template <typename TS>
using ts_value_t = detail::ts_value_from_ref_t<TS>;

// Common requirements for all time series types:
// - std::ranges::range_value_t must satisfy ts_record,
// - after the removal of cv and ref qualifiers,
//   the reference and rvalue reference types must
//   satisfy ts_record;
// - the key and value types deduced from the reference and
//   rvalue reference types must be the same.
template <typename TS>
concept common_ts_reqs
    = ts_record<std::ranges::range_value_t<TS>> && ts_record<std::remove_cvref_t<std::ranges::range_reference_t<TS>>>
      && ts_record<std::remove_cvref_t<std::ranges::range_rvalue_reference_t<TS>>>
      && std::same_as<detail::ts_key_from_ref_t<TS>, detail::ts_key_from_rref_t<TS>>
      && std::same_as<detail::ts_value_from_ref_t<TS>, detail::ts_value_from_rref_t<TS>>;

template <typename T>
concept any_input_ts = std::ranges::input_range<T> && common_ts_reqs<T>;

template <typename T>
concept ud_input_ts = requires(T &&x) {
    requires ud_input_range<T>;
    requires any_input_ts<decltype(make_input_range(std::forward<T>(x)))>;
};

template <typename T>
    requires ud_input_ts<T>
auto make_input_ts(T &&x)
{
    return make_input_range(std::forward<T>(x));
}

template <typename T>
concept any_forward_ts = std::ranges::forward_range<T> && common_ts_reqs<T>;

template <typename T>
concept ud_forward_ts = requires(T &&x) {
    requires ud_forward_range<T>;
    requires any_forward_ts<decltype(make_forward_range(std::forward<T>(x)))>;
};

template <typename T>
    requires ud_forward_ts<T>
auto make_forward_ts(T &&x)
{
    return make_forward_range(std::forward<T>(x));
}

template <typename T>
concept any_bidirectional_ts = std::ranges::bidirectional_range<T> && common_ts_reqs<T>;

template <typename T>
concept ud_bidirectional_ts = requires(T &&x) {
    requires ud_bidirectional_range<T>;
    requires any_bidirectional_ts<decltype(make_bidirectional_range(std::forward<T>(x)))>;
};

template <typename T>
    requires ud_bidirectional_ts<T>
auto make_bidirectional_ts(T &&x)
{
    return make_bidirectional_range(std::forward<T>(x));
}

template <typename T>
concept any_random_access_ts = std::ranges::random_access_range<T> && common_ts_reqs<T>;

template <typename T>
concept ud_random_access_ts = requires(T &&x) {
    requires ud_random_access_range<T>;
    requires any_random_access_ts<decltype(make_random_access_range(std::forward<T>(x)))>;
};

template <typename T>
    requires ud_random_access_ts<T>
auto make_random_access_ts(T &&x)
{
    return make_random_access_range(std::forward<T>(x));
}

} // namespace facade

#endif
