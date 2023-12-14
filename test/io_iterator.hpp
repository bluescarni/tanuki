// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_IO_ITERATOR_HPP
#define FACADE_IO_ITERATOR_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

namespace facade
{

namespace detail
{

// Helper to detect if T is referenceable.
template <typename T>
using template_arg_with_ref = T &;

// Concept to check that a type is referenceable.
template <typename T>
concept referenceable = requires() { typename template_arg_with_ref<T>; };

// Concept to check that a type is dereferenceable,
// returning the referenceable type R.
template <typename T, typename R>
concept dereferenceable = requires(const T &x) {
    requires referenceable<R>;
    {
        *x
    } -> std::same_as<R>;
};

// Concept to check that a type is pre-incrementable.
template <typename T>
concept pre_incrementable = requires(T &x) { static_cast<void>(++x); };

// Implementation of the interface.
template <typename Base, typename Holder, typename T, typename R>
struct io_iterator_iface_impl {
};

template <typename Base, typename Holder, typename T, typename R>
    requires std::movable<T> && std::copyable<T> && dereferenceable<T, R> && pre_incrementable<T>
struct io_iterator_iface_impl<Base, Holder, T, R> : public Base, tanuki::iface_impl_helper<Base, Holder> {
    void operator++() final
    {
        static_cast<void>(++this->value());
    }
    R operator*() const final
    {
        return *(this->value());
    }
};

// Definition of the interface.
template <typename R>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct io_iterator_iface {
    virtual ~io_iterator_iface() = default;
    virtual void operator++() = 0;
    virtual R operator*() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = io_iterator_iface_impl<Base, Holder, T, R>;
};

// Implementation of the reference interface.
template <typename R>
struct io_iterator_ref_iface {
    template <typename Wrap>
    struct impl {
        using difference_type = std::ptrdiff_t;

        Wrap &operator++()
        {
            iface_ptr(*static_cast<Wrap *>(this))->operator++();
            return *static_cast<Wrap *>(this);
        }
        Wrap operator++(int)
        {
            auto retval(*static_cast<const Wrap *>(this));
            ++*this;
            return retval;
        }
        R operator*() const
        {
            return iface_ptr(*static_cast<const Wrap *>(this))->operator*();
        }
    };
};

template <typename R>
inline constexpr auto io_iterator_config = tanuki::config<void, io_iterator_ref_iface<R>>{
    .static_size = tanuki::holder_size<std::remove_reference_t<R> *, detail::io_iterator_iface<R>>,
    .pointer_interface = false};

} // namespace detail

template <typename R>
using io_iterator = tanuki::wrap<detail::io_iterator_iface<R>, detail::io_iterator_config<R>>;

template <typename T>
auto make_io_iterator(T it) -> decltype(io_iterator<decltype(*it)>(std::move(it)))
{
    return io_iterator<decltype(*it)>(std::move(it));
}

} // namespace facade

#endif
