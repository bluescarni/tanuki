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
#include <iterator>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
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
// NOTE: the std::input_or_output_iterator concept
// does not specify the dereference operator must
// be const.
template <typename T, typename R>
concept dereferenceable = requires(T &x) {
    requires referenceable<R>;
    {
        *x
    } -> std::same_as<R>;
};

// Concept to check that a type is pre-incrementable.
template <typename T>
concept pre_incrementable = requires(T &x) { static_cast<void>(++x); };

// Minimal equality-comparable concept.
template <typename T>
concept minimal_eq_comparable = requires(const T &a, const T &b) { static_cast<bool>(a == b); };

// Gather the minimal requirements for a type T
// to satisfy the io_iterator concept.
template <typename T, typename R>
concept minimal_io_iterator = std::movable<T> &&
                              // NOTE: the copyable requirement is not part of the
                              // std::input_or_output_iterator - we add it in order
                              // to be able to synthesise a reasonable post-increment
                              // operator.
                              std::copyable<T> && dereferenceable<T, R> && pre_incrementable<T> &&
                              // NOTE: equality comparability is not required in C++20,
                              // but we require it as at this time we don't implement
                              // a type-erased version of sentinels.
                              minimal_eq_comparable<T>;

// Fwd-declaration of the interface.
template <typename>
struct io_iterator_iface;

// Definition of the interface implementation.
template <typename Base, typename Holder, typename T, typename R>
    requires minimal_io_iterator<T, R>
struct io_iterator_iface_impl : public Base, tanuki::iface_impl_helper<Base, Holder> {
    void operator++() final
    {
        static_cast<void>(++this->value());
    }
    R operator*() final
    {
        return *(this->value());
    }
    [[nodiscard]] std::type_index get_type_index() const noexcept final
    {
        return typeid(T);
    }
    [[nodiscard]] const void *get_ptr() const noexcept final
    {
        return std::addressof(this->value());
    }
    bool equal_to(const io_iterator_iface<R> &other) const final
    {
        if (typeid(T) == other.get_type_index()) {
            return static_cast<bool>(this->value() == *static_cast<const T *>(other.get_ptr()));
        } else {
            throw std::runtime_error("Cannot compare iterators of different types");
        }
    }
};

// Definition of the interface.
template <typename R>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct io_iterator_iface {
    virtual ~io_iterator_iface() = default;
    virtual void operator++() = 0;
    virtual R operator*() = 0;
    virtual bool equal_to(const io_iterator_iface &) const = 0;
    [[nodiscard]] virtual std::type_index get_type_index() const noexcept = 0;
    [[nodiscard]] virtual const void *get_ptr() const noexcept = 0;

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
        Wrap operator++(int)
        {
            auto retval(*static_cast<const Wrap *>(this));
            ++*this;
            return retval;
        }
        R operator*()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->operator*();
        }
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

template <typename R>
struct io_iterator_mock {
    void *ptr = nullptr;

    void operator++();
    R operator*() const;
};

template <typename R>
[[nodiscard]] bool operator==(const io_iterator_mock<R> &, const io_iterator_mock<R> &) noexcept;

template <typename R>
inline constexpr auto io_iterator_config = tanuki::config<void, io_iterator_ref_iface<R>>{
    .static_size = tanuki::holder_size<io_iterator_mock<R>, io_iterator_iface<R>>,
    .static_align = tanuki::holder_align<io_iterator_mock<R>, io_iterator_iface<R>>,
    .pointer_interface = false};

} // namespace detail

template <typename R>
using io_iterator = tanuki::wrap<detail::io_iterator_iface<R>, detail::io_iterator_config<R>>;

template <typename T>
auto make_io_iterator(T it) -> decltype(io_iterator<std::iter_reference_t<T>>(std::move(it)))
{
    return io_iterator<std::iter_reference_t<T>>(std::move(it));
}

} // namespace facade

#endif
