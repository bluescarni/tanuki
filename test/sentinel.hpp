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

template <typename T>
concept has_at_end = requires(const T &x, const any_ref &ar) {
    {
        x.at_end(ar)
    } -> std::same_as<bool>;
};

template <typename T>
concept has_distance_to_iter = requires(const T &x, const any_ref &ar) {
    {
        x.distance_to_iter(ar)
    } -> std::same_as<std::ptrdiff_t>;
};

// Definition of the interface implementation.
template <typename Base, typename Holder, typename T>
    requires std::copyable<T> && has_at_end<T>
struct sentinel_iface_impl : public Base, tanuki::iface_impl_helper<Base, Holder> {
    [[nodiscard]] bool at_end(const any_ref &ar) const final
    {
        return this->value().at_end(ar);
    }
    [[nodiscard]] std::ptrdiff_t distance_to_iter(const any_ref &ar) const final
    {
        if constexpr (has_distance_to_iter<T>) {
            return this->value().distance_to_iter(ar);
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

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] bool at_end(const any_ref &) const
    {
        return false;
    }
};

inline constexpr auto sentinel_config
    = tanuki::config<default_sentinel>{.static_size = tanuki::holder_size<default_sentinel, sentinel_iface>,
                                       .static_align = tanuki::holder_align<default_sentinel, sentinel_iface>};

} // namespace detail

using sentinel = tanuki::wrap<detail::sentinel_iface, detail::sentinel_config>;

} // namespace facade

#endif
