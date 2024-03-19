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
#include <functional>
#include <iterator>
#include <utility>

#include <tanuki/tanuki.hpp>

#include "sentinel.hpp"

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
// NOTE: the std::input_or_output_iterator concept
// does not specify the dereference operator must
// be const.
template <typename T, typename R>
concept dereferenceable = requires(T &x) {
    requires referenceable<R>;
    { *x } -> std::same_as<R>;
};

// Concept to check that a type is pre-incrementable.
template <typename T>
concept pre_incrementable = requires(T &x) { static_cast<void>(++x); };

// Gather the minimal requirements for a type T
// to satisfy the io_iterator concept.
template <typename T, typename R>
concept minimal_io_iterator = std::movable<T> && dereferenceable<T, R> && pre_incrementable<T>;

// Definition of the interface implementation.
template <typename Base, typename Holder, typename T, typename R>
    requires minimal_io_iterator<T, R>
struct io_iterator_iface_impl : public Base {
    void operator++() final
    {
        static_cast<void>(++fetch_value<Holder>(this));
    }
    R deref() final
    {
        return *fetch_value<Holder>(this);
    }
    [[nodiscard]] bool equal_to_sentinel(const sentinel &s) const final
    {
        return s->at_end(any_ref(std::ref(fetch_value<Holder>(this))));
    }
};

// Definition of the interface.
template <typename R>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct io_iterator_iface {
    virtual ~io_iterator_iface() = default;
    virtual void operator++() = 0;
    virtual R deref() = 0;
    [[nodiscard]] virtual bool equal_to_sentinel(const sentinel &) const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = io_iterator_iface_impl<Base, Holder, T, R>;
};

// Implementation of the reference interface.
template <typename R>
struct io_iterator_ref_iface {
    template <typename Wrap>
    struct impl {
        // NOTE: required by the std::input_or_output_iterator concept.
        using difference_type = std::ptrdiff_t;

        Wrap &operator++()
        {
            iface_ptr(*static_cast<Wrap *>(this))->operator++();
            return *static_cast<Wrap *>(this);
        }
        void operator++(int)
        {
            this->operator++();
        }
        R operator*()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->deref();
        }
        friend bool operator==(const impl &a, const sentinel &s)
        {
            return iface_ptr(*static_cast<const Wrap *>(&a))->equal_to_sentinel(s);
        }
        friend bool operator==(const sentinel &s, const impl &a)
        {
            return a == s;
        }
        friend bool operator!=(const impl &a, const sentinel &s)
        {
            return !(a == s);
        }
        friend bool operator!=(const sentinel &s, const impl &a)
        {
            return !(s == a);
        }
    };
};

template <typename R>
struct io_iterator_mock {
    void *ptr = nullptr;

    void operator++();
    R operator*() const;
};

template <typename R>
inline constexpr auto io_iterator_config = tanuki::config<void, io_iterator_ref_iface<R>>{
    .static_size = tanuki::holder_size<io_iterator_mock<R>, io_iterator_iface<R>>,
    .static_align = tanuki::holder_align<io_iterator_mock<R>, io_iterator_iface<R>>,
    .pointer_interface = false,
    .copyable = false};

} // namespace detail

template <typename R>
using io_iterator = tanuki::wrap<detail::io_iterator_iface<R>, detail::io_iterator_config<R>>;

template <typename T>
concept ud_io_iterator = requires() {
    typename std::iter_reference_t<T>;
    requires std::constructible_from<io_iterator<std::iter_reference_t<T>>, T>;
};

template <typename T>
    requires ud_io_iterator<T>
auto make_io_iterator(T it)
{
    return io_iterator<std::iter_reference_t<T>>(std::move(it));
}

} // namespace facade

#endif
