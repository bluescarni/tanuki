// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_BIDIRECTIONAL_ITERATOR_HPP
#define FACADE_BIDIRECTIONAL_ITERATOR_HPP

#include <concepts>
#include <iterator>
#include <stdexcept>

#include <tanuki/tanuki.hpp>

#include "forward_iterator.hpp"
#include "input_iterator.hpp"

namespace facade
{

namespace detail
{

// Concept to check that a type is pre-decrementable.
template <typename T>
concept pre_decrementable = requires(T &x) { static_cast<void>(--x); };

// Gather the minimal requirements for a type T
// to satisfy the bidirectional_iterator concept.
template <typename T, typename V, typename R, typename RR>
concept minimal_bidirectional_iterator = minimal_forward_iterator<T, V, R, RR> && pre_decrementable<T>;

template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires minimal_bidirectional_iterator<T, V, R, RR>
struct bidirectional_iterator_iface_impl
    : forward_iterator_iface_impl<Base, Holder, T, V, R, RR>,
      tanuki::iface_impl_helper<forward_iterator_iface_impl<Base, Holder, T, V, R, RR>, Holder> {
    void operator--() final
    {
        static_cast<void>(--this->value());
    }
};

template <typename V, typename R, typename RR>
struct bidirectional_iterator_iface : forward_iterator_iface<V, R, RR> {
    virtual void operator--() = 0;

    template <typename Base, typename Holder, typename T>
    using impl = bidirectional_iterator_iface_impl<Base, Holder, T, V, R, RR>;
};

// Implementation of the reference interface.
template <typename R, typename RR>
struct bidirectional_iterator_ref_iface {
    template <typename Wrap>
    struct impl : forward_iterator_ref_iface<R, RR>::template impl<Wrap> {
        // NOTE: in these operators we need the value type
        // to be copyable/movable. These requirements are part
        // of the forward iterator concept.
        Wrap &operator--()
        {
            iface_ptr(*static_cast<Wrap *>(this))->operator--();
            return *static_cast<Wrap *>(this);
        }
        Wrap operator--(int)
        {
            auto retval(*static_cast<const Wrap *>(this));
            --*this;
            return retval;
        }
    };
};

template <typename V, typename R, typename RR>
using bidirectional_iterator_c_ref_iface
    = tanuki::composite_ref_iface<bidirectional_iterator_ref_iface<R, RR>,
                                  value_tag_ref_iface<V, std::bidirectional_iterator_tag>>;

template <typename V, typename R, typename RR>
struct bidirectional_iterator_mock : forward_iterator_mock<V, R, RR> {
    [[noreturn]] void operator--()
    {
        throw std::runtime_error("Attempting to decrease a default-constructed iterator");
    }
};

template <typename V, typename R, typename RR>
inline constexpr auto bidirectional_iterator_config
    = tanuki::config<bidirectional_iterator_mock<V, R, RR>, bidirectional_iterator_c_ref_iface<V, R, RR>>{
        .static_size
        = tanuki::holder_size<bidirectional_iterator_mock<V, R, RR>, bidirectional_iterator_iface<V, R, RR>>,
        .static_align
        = tanuki::holder_align<bidirectional_iterator_mock<V, R, RR>, bidirectional_iterator_iface<V, R, RR>>,
        .pointer_interface = false};

} // namespace detail

template <typename V, typename R, typename RR>
using bidirectional_iterator
    = tanuki::wrap<detail::bidirectional_iterator_iface<V, R, RR>, detail::bidirectional_iterator_config<V, R, RR>>;

template <typename T>
auto make_bidirectional_iterator(T it)
    -> decltype(bidirectional_iterator<detail::deduce_iter_value_t<T>, std::iter_reference_t<T>,
                                       std::iter_rvalue_reference_t<T>>(std::move(it)))
{
    return bidirectional_iterator<detail::deduce_iter_value_t<T>, std::iter_reference_t<T>,
                                  std::iter_rvalue_reference_t<T>>(std::move(it));
}

} // namespace facade

#endif
