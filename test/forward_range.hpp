// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_FORWARD_RANGE_HPP
#define FACADE_FORWARD_RANGE_HPP

#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

#include "forward_iterator.hpp"
#include "input_iterator.hpp"

namespace facade
{

namespace detail
{

// Implementation of the interface.
template <typename, typename, typename, typename, typename, typename>
struct forward_range_iface_iface_impl {
};

// Definition of the interface.
template <typename V, typename R, typename RR>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct forward_range_iface {
    virtual ~forward_range_iface() = default;

    virtual forward_iterator<V, R, RR> begin() = 0;
    virtual forward_iterator<V, R, RR> end() = 0;

    template <typename Base, typename Holder, typename T>
    using impl = forward_range_iface_iface_impl<Base, Holder, T, V, R, RR>;
};

template <typename T, typename V, typename R, typename RR>
concept is_forward_range = requires(T &r) {
    requires std::ranges::range<T>;
    {
        make_forward_iterator(std::ranges::begin(r))
    } -> std::same_as<forward_iterator<V, R, RR>>;
    {
        make_forward_iterator(std::ranges::end(r))
    } -> std::same_as<forward_iterator<V, R, RR>>;
};

template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires std::derived_from<Base, forward_range_iface<V, R, RR>>
                 && is_forward_range<std::remove_reference_t<std::unwrap_reference_t<T>>, V, R, RR>
struct forward_range_iface_iface_impl<Base, Holder, T, V, R, RR> : public Base,
                                                                   tanuki::iface_impl_helper<Base, Holder> {
    forward_iterator<V, R, RR> begin() final
    {
        return make_forward_iterator(std::ranges::begin(this->value()));
    }
    forward_iterator<V, R, RR> end() final
    {
        return make_forward_iterator(std::ranges::begin(this->value()));
    }
};

// Implementation of the reference interface.
template <typename V, typename R, typename RR>
struct forward_range_ref_iface {
    template <typename Wrap>
    struct impl {
        forward_iterator<V, R, RR> begin()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->begin();
        }
        forward_iterator<V, R, RR> end()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->end();
        }
    };
};

template <typename V, typename R, typename RR>
struct forward_range_mock {
    void *ptr1 = nullptr;
    void *ptr2 = nullptr;

    forward_iterator<V, R, RR> begin();
    forward_iterator<V, R, RR> end();
};

template <typename V, typename R, typename RR>
inline constexpr auto forward_range_config = tanuki::config<void, forward_range_ref_iface<V, R, RR>>{
    .static_size = tanuki::holder_size<forward_range_mock<V, R, RR>, forward_range_iface<V, R, RR>>,
    .pointer_interface = false};

} // namespace detail

template <typename V, typename R, typename RR>
using forward_range = tanuki::wrap<detail::forward_range_iface<V, R, RR>, detail::forward_range_config<V, R, RR>>;

template <typename T>
auto make_forward_range(T &&x)
    -> decltype(forward_range<detail::deduce_iter_value_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,
                              std::iter_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,
                              std::iter_rvalue_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>>(
        std::forward<T>(x)))
{
    return forward_range<detail::deduce_iter_value_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,
                         std::iter_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,
                         std::iter_rvalue_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>>(
        std::forward<T>(x));
}

template <typename T>
auto make_forward_range(std::reference_wrapper<T> ref)
    -> decltype(forward_range<detail::deduce_iter_value_t<std::ranges::iterator_t<T>>,
                              std::iter_reference_t<std::ranges::iterator_t<T>>,
                              std::iter_rvalue_reference_t<std::ranges::iterator_t<T>>>(std::move(ref)))
{
    return forward_range<detail::deduce_iter_value_t<std::ranges::iterator_t<T>>,
                         std::iter_reference_t<std::ranges::iterator_t<T>>,
                         std::iter_rvalue_reference_t<std::ranges::iterator_t<T>>>(std::move(ref));
}

} // namespace facade

#endif
