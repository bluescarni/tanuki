// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_RANDOM_ACCESS_ITERATOR_HPP
#define FACADE_RANDOM_ACCESS_ITERATOR_HPP

#include <concepts>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <typeinfo>

#include <tanuki/tanuki.hpp>

#include "bidirectional_iterator.hpp"
#include "input_iterator.hpp"
#include "sentinel.hpp"

namespace facade
{

namespace detail
{

template <typename T>
concept minimal_less_than_comparable = requires(const T &a, const T &b) { static_cast<bool>(a < b); };

template <typename T>
concept incrementable_by_ptrdiff_t = requires(T &a, std::ptrdiff_t n) { static_cast<void>(a += n); };

template <typename T>
concept decrementable_by_ptrdiff_t = requires(T &a, std::ptrdiff_t n) { static_cast<void>(a -= n); };

// Fwd declaration of the interface.
template <typename, typename, typename>
struct random_access_iterator_iface;

// Gather the minimal requirements for a type T
// to satisfy the random_access_iterator concept.
template <typename T, typename V, typename R, typename RR>
concept minimal_random_access_iterator
    = minimal_bidirectional_iterator<T, V, R, RR> && minimal_less_than_comparable<T> && incrementable_by_ptrdiff_t<T>
      && decrementable_by_ptrdiff_t<T> && with_ptrdiff_t_difference<T>;

template <typename Base, typename Holder, typename T, typename V, typename R, typename RR>
    requires minimal_random_access_iterator<T, V, R, RR>
struct random_access_iterator_iface_impl : bidirectional_iterator_iface_impl<Base, Holder, T, V, R, RR> {
    bool less_than(const random_access_iterator_iface<V, R, RR> &other) const final
    {
        if (typeid(T) == other.get_type_index()) {
            return static_cast<bool>(getval<Holder>(this) < *static_cast<const T *>(other.get_ptr()));
        } else {
            throw std::runtime_error("Unable to compare an iterator of type '" + tanuki::demangle(typeid(T).name())
                                     + "' to an iterator of type '" + tanuki::demangle(other.get_type_index().name())
                                     + "'");
        }
    }
    void increment_by(std::ptrdiff_t n) final
    {
        static_cast<void>(getval<Holder>(this) += n);
    }
    void decrement_by(std::ptrdiff_t n) final
    {
        static_cast<void>(getval<Holder>(this) -= n);
    }
    std::ptrdiff_t distance_from(const random_access_iterator_iface<V, R, RR> &other) const final
    {
        if (typeid(T) == other.get_type_index()) {
            const auto &other_val = *static_cast<const T *>(other.get_ptr());

            return static_cast<std::ptrdiff_t>(getval<Holder>(this) - other_val);
        } else {
            throw std::runtime_error("Unable to compute the distance of an iterator of type '"
                                     + tanuki::demangle(typeid(T).name()) + "' from an iterator of type '"
                                     + tanuki::demangle(other.get_type_index().name()) + "'");
        }
    }
    [[nodiscard]] std::ptrdiff_t distance_from_sentinel(const sentinel &s) const final
    {
        return s->distance_to_iter(any_ref(std::ref(getval<Holder>(this))));
    }
};

template <typename V, typename R, typename RR>
struct random_access_iterator_iface : bidirectional_iterator_iface<V, R, RR> {
    virtual bool less_than(const random_access_iterator_iface &) const = 0;
    virtual void increment_by(std::ptrdiff_t) = 0;
    virtual void decrement_by(std::ptrdiff_t) = 0;
    virtual std::ptrdiff_t distance_from(const random_access_iterator_iface &) const = 0;
    [[nodiscard]] virtual std::ptrdiff_t distance_from_sentinel(const sentinel &) const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = random_access_iterator_iface_impl<Base, Holder, T, V, R, RR>;
};

// Implementation of the reference interface.
template <typename R, typename RR>
struct random_access_iterator_ref_iface {
    template <typename Wrap>
    struct impl : bidirectional_iterator_ref_iface<R, RR>::template impl<Wrap> {
        friend bool operator<(const impl &a, const impl &b)
        {
            return iface_ptr(static_cast<const Wrap &>(a))->less_than(*iface_ptr(static_cast<const Wrap &>(b)));
        }
        friend bool operator<=(const impl &a, const impl &b)
        {
            return (a < b) || (a == b);
        }
        friend bool operator>(const impl &a, const impl &b)
        {
            return !(a <= b);
        }
        friend bool operator>=(const impl &a, const impl &b)
        {
            return !(a < b);
        }

        friend Wrap &operator+=(impl &a, std::ptrdiff_t n)
        {
            iface_ptr(static_cast<Wrap &>(a))->increment_by(n);
            return static_cast<Wrap &>(a);
        }
        friend Wrap operator+(const impl &a, std::ptrdiff_t n)
        {
            auto retval(static_cast<const Wrap &>(a));
            retval += n;
            return retval;
        }
        friend Wrap operator+(std::ptrdiff_t n, const impl &a)
        {
            return a + n;
        }
        friend Wrap &operator-=(impl &a, std::ptrdiff_t n)
        {
            iface_ptr(static_cast<Wrap &>(a))->decrement_by(n);
            return static_cast<Wrap &>(a);
        }
        friend Wrap operator-(const impl &a, std::ptrdiff_t n)
        {
            auto retval(static_cast<const Wrap &>(a));
            retval -= n;
            return retval;
        }
        friend std::ptrdiff_t operator-(const impl &a, const impl &b)
        {
            return iface_ptr(static_cast<const Wrap &>(a))->distance_from(*iface_ptr(static_cast<const Wrap &>(b)));
        }
        friend std::ptrdiff_t operator-(const impl &a, const sentinel &s)
        {
            return iface_ptr(static_cast<const Wrap &>(a))->distance_from_sentinel(s);
        }
        friend std::ptrdiff_t operator-(const sentinel &s, const impl &a)
        {
            return -(a - s);
        }

        R operator[](std::ptrdiff_t n) const
        {
            return *(static_cast<const Wrap &>(*this) + n);
        }
    };
};

template <typename V, typename R, typename RR>
using random_access_iterator_c_ref_iface
    = tanuki::composite_ref_iface<random_access_iterator_ref_iface<R, RR>,
                                  value_tag_ref_iface<V, std::random_access_iterator_tag>>;

template <typename V, typename R, typename RR>
struct random_access_iterator_mock : bidirectional_iterator_mock<V, R, RR> {
    [[nodiscard]] friend bool operator<(const random_access_iterator_mock &,
                                        const random_access_iterator_mock &) noexcept
    {
        return false;
    }
    [[noreturn]] friend void operator+=(random_access_iterator_mock &, std::ptrdiff_t)
    {
        throw std::runtime_error("Attempting to increment a default-constructed iterator");
    }
    [[noreturn]] friend void operator-=(random_access_iterator_mock &, std::ptrdiff_t)
    {
        throw std::runtime_error("Attempting to decrement a default-constructed iterator");
    }
    [[nodiscard]] friend std::ptrdiff_t operator-(const random_access_iterator_mock &,
                                                  const random_access_iterator_mock &) noexcept
    {
        return 0;
    }
};

template <typename V, typename R, typename RR>
inline constexpr auto random_access_iterator_config
    = tanuki::config<random_access_iterator_mock<V, R, RR>, random_access_iterator_c_ref_iface<V, R, RR>>{
        .static_size
        = tanuki::holder_size<random_access_iterator_mock<V, R, RR>, random_access_iterator_iface<V, R, RR>>,
        .static_align
        = tanuki::holder_align<random_access_iterator_mock<V, R, RR>, random_access_iterator_iface<V, R, RR>>,
        .pointer_interface = false};

} // namespace detail

template <typename V, typename R, typename RR>
using random_access_iterator
    = tanuki::wrap<detail::random_access_iterator_iface<V, R, RR>, detail::random_access_iterator_config<V, R, RR>>;

template <typename T>
concept ud_random_access_iterator = detail::generic_ud_input_iterator<T, random_access_iterator>;

template <typename T>
    requires ud_random_access_iterator<T>
auto make_random_access_iterator(T it)
{
    return random_access_iterator<detail::deduce_iter_value_t<T>, std::iter_reference_t<T>,
                                  std::iter_rvalue_reference_t<T>>(std::move(it));
}

} // namespace facade

#endif
