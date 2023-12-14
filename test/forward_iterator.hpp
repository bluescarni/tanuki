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

#include <tanuki/tanuki.hpp>

#include "input_iterator.hpp"
#include "io_iterator.hpp"

namespace facade
{

namespace detail
{

// Definition of the interface implementation for forward iterators.
template <typename, typename, typename, typename, typename, typename>
struct forward_iterator_iface_impl {
};

template <typename V, typename R, typename RR>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct forward_iterator_iface {
    virtual ~forward_iterator_iface() = default;
    virtual bool equal_to(const forward_iterator_iface &) const = 0;
    [[nodiscard]] virtual std::type_index get_type_index_for_comparison() const noexcept = 0;
    [[nodiscard]] virtual const void *get_ptr_for_comparison() const noexcept = 0;

    template <typename Base, typename Holder, typename T>
    using impl = forward_iterator_iface_impl<Base, Holder, T, V, R, RR>;
};

template <typename T>
concept minimal_eq_comparable = requires(const T &a, const T &b) { static_cast<bool>(a == b); };

template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires std::derived_from<Base, forward_iterator_iface<V, R, RR>> && std::default_initializable<T>
                 && minimal_eq_comparable<T>
struct forward_iterator_iface_impl<Base, Holder, T, V, R, RR> : public Base, tanuki::iface_impl_helper<Base, Holder> {
    [[nodiscard]] std::type_index get_type_index_for_comparison() const noexcept final
    {
        return typeid(T);
    }
    [[nodiscard]] const void *get_ptr_for_comparison() const noexcept final
    {
        return std::addressof(this->value());
    }
    bool equal_to(const forward_iterator_iface<V, R, RR> &other) const final
    {
        if (typeid(T) == other.get_type_index_for_comparison()) {
            return static_cast<bool>(this->value() == *static_cast<const T *>(other.get_ptr_for_comparison()));
        } else {
            return false;
        }
    }
};

struct forward_iterator_ref_iface {
    template <typename Wrap>
    struct impl {
        friend bool operator==(const Wrap &a, const Wrap &b)
        {
            return iface_ptr(a)->equal_to(*iface_ptr(b));
        }
        friend bool operator!=(const Wrap &a, const Wrap &b)
        {
            return !(a == b);
        }
    };
};

template <typename V, typename R, typename RR>
using forward_iterator_c_iface
    = tanuki::composite_iface<io_iterator_iface<R>, input_iterator_iface<V, R, RR>, forward_iterator_iface<V, R, RR>>;

template <typename V, typename R, typename RR>
using forward_iterator_c_ref_iface
    = tanuki::composite_ref_iface<io_iterator_ref_iface<R>, value_tag_ref_iface<V, std::forward_iterator_tag>,
                                  forward_iterator_ref_iface>;

template <typename V, typename R, typename RR>
struct forward_iterator_mock {
    void *ptr = nullptr;

    [[noreturn]] void operator++()
    {
        throw std::runtime_error("Attempting to increase a default-constructed forward_iterator");
    }
    [[noreturn]] R operator*() const
    {
        throw std::runtime_error("Attempting to dereference a default-constructed forward_iterator");
    }
    [[nodiscard]] bool operator==(const forward_iterator_mock &) const noexcept
    {
        return true;
    }

    [[noreturn]] friend RR iter_move(const forward_iterator_mock &)
    {
        throw std::runtime_error("Attempting to invoke iter_move() on a default-constructed forward_iterator");
    }
};

template <typename V, typename R, typename RR>
inline constexpr auto forward_iterator_config
    = tanuki::config<forward_iterator_mock<V, R, RR>, forward_iterator_c_ref_iface<V, R, RR>>{
        .static_size = tanuki::holder_size<forward_iterator_mock<V, R, RR>, forward_iterator_c_iface<V, R, RR>>,
        .pointer_interface = false};

} // namespace detail

template <typename V, typename R, typename RR>
using forward_iterator
    = tanuki::wrap<detail::forward_iterator_c_iface<V, R, RR>, detail::forward_iterator_config<V, R, RR>>;

} // namespace facade

#endif
