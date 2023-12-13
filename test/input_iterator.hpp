// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_INPUT_ITERATOR_HPP
#define FACADE_INPUT_ITERATOR_HPP

#include <concepts>
#include <iterator>

#include <tanuki/tanuki.hpp>

#include "io_iterator.hpp"

namespace facade
{

namespace detail
{

// Definition of the interface implementation for input iterators.
template <typename, typename, typename, typename, typename, typename>
struct input_iterator_iface_impl {
};

template <typename T, typename RR>
concept with_iter_move = requires(const T &x) {
    {
        std::ranges::iter_move(x)
    } -> std::same_as<RR>;
};

template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires with_iter_move<T, RR> && std::common_reference_with<R &&, V &> && std::common_reference_with<R &&, RR &&>
                 && std::common_reference_with<RR &&, const V &>
struct input_iterator_iface_impl<Base, Holder, T, V, R, RR> : public Base, tanuki::iface_impl_helper<Base, Holder> {
    RR iter_move() const final
    {
        return std::ranges::iter_move(this->value());
    }
};

template <typename V, typename R, typename RR>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct input_iterator_iface {
    virtual ~input_iterator_iface() = default;
    virtual RR iter_move() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = input_iterator_iface_impl<Base, Holder, T, V, R, RR>;
};

template <typename V, typename R, typename RR>
struct input_iterator_ref_iface {
    template <typename Wrap>
    struct impl {
        using value_type = V;
        using iterator_concept = std::input_iterator_tag;
    };
};

template <typename V, typename R, typename RR>
inline constexpr auto input_iterator_config
    = tanuki::config<void, tanuki::composite_ref_iface<io_iterator_ref_iface<R>, input_iterator_ref_iface<V, R, RR>>>{
        .pointer_interface = false};

} // namespace detail

template <typename V, typename R, typename RR>
using input_iterator
    = tanuki::wrap<tanuki::composite_iface<detail::io_iterator_iface<R>, detail::input_iterator_iface<V, R, RR>>,
                   detail::input_iterator_config<V, R, RR>>;

template <typename V, typename R, typename RR>
RR iter_move(input_iterator<V, R, RR> it)
{
    return iface_ptr(it)->iter_move();
}

} // namespace facade

#endif
