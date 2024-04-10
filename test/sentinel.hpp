// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_SENTINEL_HPP
#define FACADE_SENTINEL_HPP

#include <concepts>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>

#include <tanuki/tanuki.hpp>

namespace facade
{

namespace detail
{

// Detect instances of std::reference_wrapper.
template <typename>
struct is_reference_wrapper : std::false_type {
};

template <typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {
};

// A type-erased interface for storing references to objects.
template <typename Base, typename Holder, typename T>
    requires is_reference_wrapper<T>::value
struct any_ref_iface_impl : public Base {
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct any_ref_iface {
    virtual ~any_ref_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_ref_iface_impl<Base, Holder, T>;
};

inline constexpr auto any_ref_config
    = tanuki::config<>{.static_size = tanuki::holder_size<std::reference_wrapper<int>, any_ref_iface>,
                       .static_align = tanuki::holder_align<std::reference_wrapper<int>, any_ref_iface>};

using any_ref = tanuki::wrap<any_ref_iface, any_ref_config>;

// Detect if T - U is well defined, returning something which is castable to std::ptrdiff_t.
template <typename T, typename U = T>
concept with_ptrdiff_t_difference = requires(const T &a, const U &b) { static_cast<std::ptrdiff_t>(a - b); };

// This struct stores internally a sentinel of type S
// and enables comparison and distance computation with
// respect to an iterator of type It (which is always passed
// wrapped into an any_ref).
template <typename S, typename It>
struct sentinel_box {
    TANUKI_NO_UNIQUE_ADDRESS S m_sentinel;

    [[nodiscard]] bool at_end(const any_ref &r_it) const
    {
        if (const auto *ptr = value_ptr<std::reference_wrapper<const It>>(r_it)) {
            return static_cast<bool>(ptr->get() == m_sentinel);
        }

        throw std::runtime_error("Unable to compare an iterator of type '" + tanuki::demangle(typeid(It).name())
                                 + "' to a sentinel of type '" + tanuki::demangle(typeid(S).name()) + "'");
    }
    // NOTE: this is enabled only if subtraction between It and S is well defined.
    [[nodiscard]] std::ptrdiff_t distance_to_iter(const any_ref &r_it) const
        requires with_ptrdiff_t_difference<It, S>
    {
        if (const auto *ptr = value_ptr<std::reference_wrapper<const It>>(r_it)) {
            return static_cast<std::ptrdiff_t>(ptr->get() - m_sentinel);
        }

        throw std::runtime_error("Unable to compute the distance between an iterator of type '"
                                 + tanuki::demangle(typeid(It).name()) + "' and a sentinel of type '"
                                 + tanuki::demangle(typeid(S).name()) + "'");
    }
};

// NOTE: this is needed only for sized sentinels.
template <typename T>
concept has_distance_to_iter = requires(const T &x, const any_ref &ar) {
    {
        x.distance_to_iter(ar)
    } -> std::same_as<std::ptrdiff_t>;
};

// Detect instances of sentinel_box.
template <typename>
struct is_sentinel_box : std::false_type {
};

template <typename S, typename It>
struct is_sentinel_box<sentinel_box<S, It>> : std::true_type {
};

// Definition of the interface implementation for sentinel.
template <typename Base, typename Holder, typename T>
    requires is_sentinel_box<T>::value
struct sentinel_iface_impl : public Base {
    [[nodiscard]] bool at_end(const any_ref &ar) const final
    {
        return getval<Holder>(this).at_end(ar);
    }
    [[nodiscard]] std::ptrdiff_t distance_to_iter(const any_ref &ar) const final
    {
        if constexpr (has_distance_to_iter<T>) {
            return getval<Holder>(this).distance_to_iter(ar);
        } else {
            throw std::runtime_error("The sentinel type '" + tanuki::demangle(typeid(T).name())
                                     + "' is not a sized sentinel");
        }
    }
};

// Definition of the interface.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct sentinel_iface {
    virtual ~sentinel_iface() = default;
    [[nodiscard]] virtual bool at_end(const any_ref &) const = 0;
    [[nodiscard]] virtual std::ptrdiff_t distance_to_iter(const any_ref &) const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = sentinel_iface_impl<Base, Holder, T>;
};

struct default_sentinel {
    void *ptr = nullptr;

    [[nodiscard]] friend bool operator==(int *, const default_sentinel &)
    {
        return false;
    }
};

inline constexpr auto sentinel_config = tanuki::config<sentinel_box<default_sentinel, int *>>{
    .static_size = tanuki::holder_size<sentinel_box<default_sentinel, int *>, sentinel_iface>,
    .static_align = tanuki::holder_align<sentinel_box<default_sentinel, int *>, sentinel_iface>};

} // namespace detail

using sentinel = tanuki::wrap<detail::sentinel_iface, detail::sentinel_config>;

} // namespace facade

#endif
