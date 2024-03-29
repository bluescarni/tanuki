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
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

#include "io_iterator.hpp"

namespace facade
{

namespace detail
{

template <typename T, typename RR>
concept with_iter_move = requires(const T &x) {
    requires referenceable<RR>;
    { std::ranges::iter_move(x) } -> std::same_as<RR>;
};

// NOTE: the std::input_iterator concept specifies
// that we must be able to dereference a const-qualified
// object.
template <typename T, typename R>
concept const_dereferenceable = requires(const T &x) {
    requires referenceable<R>;
    { *x } -> std::same_as<R>;
};

// Gather the minimal requirements for a type T
// to satisfy the input_iterator concept.
template <typename T, typename V, typename R, typename RR>
concept minimal_input_iterator = minimal_io_iterator<T, R> && with_iter_move<T, RR> && const_dereferenceable<T, R>
                                 && std::common_reference_with<R &&, V &> && std::common_reference_with<R &&, RR &&>
                                 && std::common_reference_with<RR &&, const V &>;

// Definition of the interface implementation.
template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires minimal_input_iterator<T, V, R, RR>
struct input_iterator_iface_impl : io_iterator_iface_impl<Base, Holder, T, R> {
    R const_deref() const final
    {
        return *getval<Holder>(this);
    }
    RR iter_move() const final
    {
        return std::ranges::iter_move(getval<Holder>(this));
    }
};

template <typename V, typename R, typename RR>
struct input_iterator_iface : io_iterator_iface<R> {
    virtual R const_deref() const = 0;
    virtual RR iter_move() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = input_iterator_iface_impl<Base, Holder, T, V, R, RR>;
};

// Helper struct to assign a value_type
// and iterator_concept to an iterator
// reference interface.
template <typename V, typename Tag>
struct value_tag_ref_iface {
    template <typename Wrap>
    struct impl {
        using value_type = V;
        using iterator_concept = Tag;
    };
};

template <typename R, typename RR>
struct input_iterator_ref_iface {
    template <typename Wrap>
    struct impl : io_iterator_ref_iface<R>::template impl<Wrap> {
        // NOTE: bring in the non-const version of the dereference
        // operator from io_iterator_ref_iface.
        using io_iterator_ref_iface<R>::template impl<Wrap>::operator*;

        R operator*() const
        {
            return iface_ptr(*static_cast<const Wrap *>(this))->const_deref();
        }
        // Implementation of the iter_move customisation point
        // for input iterators.
        friend RR iter_move(const impl &it)
        {
            return iface_ptr(*static_cast<const Wrap *>(&it))->iter_move();
        }
    };
};

template <typename V, typename R, typename RR>
using input_iterator_c_ref_iface
    = tanuki::composite_ref_iface<input_iterator_ref_iface<R, RR>, value_tag_ref_iface<V, std::input_iterator_tag>>;

template <typename V, typename R, typename RR>
inline constexpr auto input_iterator_config = tanuki::config<void, input_iterator_c_ref_iface<V, R, RR>>{
    .static_size = tanuki::holder_size<io_iterator_mock<R>, input_iterator_iface<V, R, RR>>,
    .static_align = tanuki::holder_align<io_iterator_mock<R>, input_iterator_iface<V, R, RR>>,
    .pointer_interface = false,
    .copyable = false};

} // namespace detail

template <typename V, typename R, typename RR>
using input_iterator = tanuki::wrap<detail::input_iterator_iface<V, R, RR>, detail::input_iterator_config<V, R, RR>>;

namespace detail
{

// Machinery to deduce a value type from an iterator-like
// type T.
template <typename T>
concept has_iter_value = requires() { typename std::iter_value_t<T>; };

template <typename T>
concept has_iter_ref = requires() { typename std::iter_reference_t<T>; };

template <typename T>
struct deduce_iter_value {
};

// std::iter_value_t<T> is available, use it.
template <typename T>
    requires has_iter_value<T>
struct deduce_iter_value<T> {
    using type = std::iter_value_t<T>;
};

// std::iter_value_t<T> is not available, but we have
// std::iter_reference_t<T>: the value type will be
// the referenced type.
template <typename T>
    requires(!has_iter_value<T>) && has_iter_ref<T>
struct deduce_iter_value<T> {
    using type = std::remove_cvref_t<std::iter_reference_t<T>>;
};

template <typename T>
using deduce_iter_value_t = typename detail::deduce_iter_value<T>::type;

template <typename T, template <typename, typename, typename> typename It>
concept generic_ud_input_iterator = requires() {
    typename deduce_iter_value_t<T>;
    typename std::iter_reference_t<T>;
    typename std::iter_rvalue_reference_t<T>;
    requires std::constructible_from<
        It<deduce_iter_value_t<T>, std::iter_reference_t<T>, std::iter_rvalue_reference_t<T>>, T>;
};

} // namespace detail

template <typename T>
concept ud_input_iterator = detail::generic_ud_input_iterator<T, input_iterator>;

template <typename T>
    requires ud_input_iterator<T>
auto make_input_iterator(T it)
{
    return input_iterator<detail::deduce_iter_value_t<T>, std::iter_reference_t<T>, std::iter_rvalue_reference_t<T>>(
        std::move(it));
}

} // namespace facade

#endif
