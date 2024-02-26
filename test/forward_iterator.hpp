// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_FORWARD_ITERATOR_HPP
#define FACADE_FORWARD_ITERATOR_HPP

#include <concepts>
#include <iterator>
#include <stdexcept>

#include <tanuki/tanuki.hpp>

#include "input_iterator.hpp"

namespace facade
{

namespace detail
{

// Gather the minimal requirements for a type T
// to satisfy the forward_iterator concept.
template <typename T, typename V, typename R, typename RR>
concept minimal_forward_iterator = minimal_input_iterator<T, V, R, RR>;

// Fwd declaration of the interface.
template <typename, typename, typename>
struct forward_iterator_iface;

// Implementation of the interface.
template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires minimal_forward_iterator<T, V, R, RR>
struct forward_iterator_iface_impl
    : input_iterator_iface_impl<Base, Holder, T, V, R, RR>,
      tanuki::iface_impl_helper<input_iterator_iface_impl<Base, Holder, T, V, R, RR>, Holder> {
};

template <typename V, typename R, typename RR>
struct forward_iterator_iface : input_iterator_iface<V, R, RR> {
    template <typename Base, typename Holder, typename T>
    using impl = forward_iterator_iface_impl<Base, Holder, T, V, R, RR>;
};

template <typename R, typename RR>
struct forward_iterator_ref_iface {
    template <typename Wrap>
    struct impl : input_iterator_ref_iface<R, RR>::template impl<Wrap> {
    };
};

template <typename V, typename R, typename RR>
using forward_iterator_c_ref_iface
    = tanuki::composite_ref_iface<forward_iterator_ref_iface<R, RR>, value_tag_ref_iface<V, std::forward_iterator_tag>>;

template <typename V, typename R, typename RR>
inline constexpr auto forward_iterator_config
    = tanuki::config<input_iterator_mock<V, R, RR>, forward_iterator_c_ref_iface<V, R, RR>>{
        .static_size = tanuki::holder_size<input_iterator_mock<V, R, RR>, forward_iterator_iface<V, R, RR>>,
        .static_align = tanuki::holder_align<input_iterator_mock<V, R, RR>, forward_iterator_iface<V, R, RR>>,
        .pointer_interface = false};

} // namespace detail

template <typename V, typename R, typename RR>
using forward_iterator
    = tanuki::wrap<detail::forward_iterator_iface<V, R, RR>, detail::forward_iterator_config<V, R, RR>>;

template <typename T>
auto make_forward_iterator(T it) -> decltype(forward_iterator<detail::deduce_iter_value_t<T>, std::iter_reference_t<T>,
                                                              std::iter_rvalue_reference_t<T>>(std::move(it)))
{
    return forward_iterator<detail::deduce_iter_value_t<T>, std::iter_reference_t<T>, std::iter_rvalue_reference_t<T>>(
        std::move(it));
}

} // namespace facade

#endif
