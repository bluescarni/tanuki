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
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>

#include <tanuki/tanuki.hpp>

#include "input_iterator.hpp"

namespace facade
{

namespace detail
{

// Definition of the interface implementation for forward iterators.
template <typename, typename, typename, typename, typename, typename>
struct forward_iterator_iface_impl {
};

template <typename V, typename R, typename RR>
struct forward_iterator_iface : input_iterator_iface<V, R, RR> {
    virtual bool equal_to(const forward_iterator_iface &) const = 0;
    [[nodiscard]] virtual std::type_index get_type_index() const noexcept = 0;
    [[nodiscard]] virtual const void *get_ptr() const noexcept = 0;

    template <typename Base, typename Holder, typename T>
    using impl = forward_iterator_iface_impl<Base, Holder, T, V, R, RR>;
};

template <typename T>
concept minimal_eq_comparable = requires(const T &a, const T &b) { static_cast<bool>(a == b); };

// Gather the minimal requirements for a type T
// to satisfy the forward_iterator concept.
template <typename T, typename V, typename R, typename RR>
concept minimal_forward_iterator
    = minimal_input_iterator<T, V, R, RR> && std::default_initializable<T> && minimal_eq_comparable<T>;

template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires std::derived_from<Base, forward_iterator_iface<V, R, RR>> && minimal_forward_iterator<T, V, R, RR>
struct forward_iterator_iface_impl<Base, Holder, T, V, R, RR>
    : input_iterator_iface_impl<Base, Holder, T, V, R, RR>,
      tanuki::iface_impl_helper<input_iterator_iface_impl<Base, Holder, T, V, R, RR>, Holder> {
    [[nodiscard]] std::type_index get_type_index() const noexcept final
    {
        return typeid(T);
    }
    [[nodiscard]] const void *get_ptr() const noexcept final
    {
        return std::addressof(this->value());
    }
    bool equal_to(const forward_iterator_iface<V, R, RR> &other) const final
    {
        if (typeid(T) == other.get_type_index()) {
            return static_cast<bool>(this->value() == *static_cast<const T *>(other.get_ptr()));
        } else {
            throw std::runtime_error("Cannot compare iterators of different type");
        }
    }
};

template <typename R, typename RR>
struct forward_iterator_ref_iface {
    template <typename Wrap>
    struct impl : input_iterator_ref_iface<R, RR>::template impl<Wrap> {
        friend bool operator==(const impl &a, const impl &b)
        {
            return iface_ptr(*static_cast<const Wrap *>(&a))->equal_to(*iface_ptr(*static_cast<const Wrap *>(&b)));
        }
        friend bool operator!=(const impl &a, const impl &b)
        {
            return !(a == b);
        }
    };
};

template <typename V, typename R, typename RR>
using forward_iterator_c_ref_iface
    = tanuki::composite_ref_iface<forward_iterator_ref_iface<R, RR>, value_tag_ref_iface<V, std::forward_iterator_tag>>;

template <typename V, typename R, typename RR>
struct forward_iterator_mock {
    void *ptr = nullptr;

    [[noreturn]] void operator++()
    {
        throw std::runtime_error("Attempting to increase a default-constructed iterator");
    }
    [[noreturn]] R operator*() const
    {
        throw std::runtime_error("Attempting to dereference a default-constructed iterator");
    }
    [[nodiscard]] friend bool operator==(const forward_iterator_mock &, const forward_iterator_mock &) noexcept
    {
        return true;
    }
    [[noreturn]] friend RR iter_move(const forward_iterator_mock &)
    {
        throw std::runtime_error("Attempting to invoke iter_move() on a default-constructed iterator");
    }
};

template <typename V, typename R, typename RR>
inline constexpr auto forward_iterator_config
    = tanuki::config<forward_iterator_mock<V, R, RR>, forward_iterator_c_ref_iface<V, R, RR>>{
        .static_size = tanuki::holder_size<forward_iterator_mock<V, R, RR>, forward_iterator_iface<V, R, RR>>,
        .pointer_interface = false};

} // namespace detail

template <typename V, typename R, typename RR>
using forward_iterator
    = tanuki::wrap<detail::forward_iterator_iface<V, R, RR>, detail::forward_iterator_config<V, R, RR>>;

template <typename T>
    requires std::forward_iterator<T>
auto make_forward_iterator(T it)
{
    return forward_iterator<std::iter_value_t<T>, std::iter_reference_t<T>, std::iter_rvalue_reference_t<T>>(
        std::move(it));
}

template <typename T>
auto make_forward_iterator(T it)
    -> decltype(forward_iterator<std::remove_cvref_t<decltype(*it)>, decltype(*it),
                                 decltype(std::ranges::iter_move(std::as_const(it)))>(std::move(it)))
{
    return forward_iterator<std::remove_cvref_t<decltype(*it)>, decltype(*it),
                            decltype(std::ranges::iter_move(std::as_const(it)))>(std::move(it));
}

} // namespace facade

#endif
