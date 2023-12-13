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

// Definition of the interface template for input iterators.
template <typename, typename, typename, typename, typename>
struct input_iterator_iface {
};

template <typename V, typename R, typename RR>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct input_iterator_iface<void, void, V, R, RR> {
    virtual ~input_iterator_iface() = default;
    virtual RR iter_move() const = 0;
};

template <typename T, typename RR>
concept with_iter_move = requires(const T &x) {
    {
        std::ranges::iter_move(x)
    } -> std::same_as<RR>;
};

template <typename Holder, typename T, typename V, typename R, typename RR>
    requires with_iter_move<T, RR> && std::common_reference_with<R &&, V &> && std::common_reference_with<R &&, RR &&>
                 && std::common_reference_with<RR &&, const V &>
struct input_iterator_iface<Holder, T, V, R, RR>
    : virtual input_iterator_iface<void, void, V, R, RR>,
      tanuki::iface_impl_helper<Holder, T, input_iterator_iface, V, R, RR> {
    RR iter_move() const final
    {
        return std::ranges::iter_move(this->value());
    }
};

template <typename V, typename R, typename RR>
using input_iterator_wrap = tanuki::wrap<input_iterator_iface, tanuki::default_config, V, R, RR>;

template <typename Wrap, typename V, typename R, typename RR>
struct input_iterator_ref_iface_impl : io_iterator_ref_iface_impl<Wrap, R> {
    using value_type = V;
};

template <typename V, typename R, typename RR>
struct input_iterator_ref_iface {
    template <typename Wrap>
    using type = input_iterator_ref_iface_impl<Wrap, V, R, RR>;
};

template <typename V, typename R, typename RR>
inline constexpr auto input_iterator_config
    = tanuki::config<void, input_iterator_ref_iface<V, R, RR>::template type>{.pointer_interface = false};

template <typename V, typename R, typename RR>
struct compoiface {
    template <typename Holder, typename T>
    using type
        = tanuki::composite_wrap_interfaceT<io_iterator<R>, input_iterator_wrap<V, R, RR>>::template type<Holder, T>;
};

template <typename V, typename R, typename RR>
using input_iterator = tanuki::wrap<compoiface<V, R, RR>::template type, input_iterator_config<V, R, RR>>;

template <typename T>
struct is_blaffo : std::false_type {
};

template <typename V, typename R, typename RR>
struct is_blaffo<input_iterator<V, R, RR>> : std::true_type {
};

template <typename V, typename R, typename RR>
RR iter_move(input_iterator<V, R, RR> it)
{
    // std::cout << "Calling iter move!\n";
    return iface_ptr(it)->iter_move();
}

} // namespace facade

#endif
