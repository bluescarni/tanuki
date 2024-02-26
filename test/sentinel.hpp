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
#include <iterator>

#include <tanuki/tanuki.hpp>

namespace facade
{

namespace detail
{

// Definition of the interface implementation.
template <typename Base, typename Holder, typename T>
    requires std::copyable<T>
struct sentinel_iface_impl : public Base {
};

// Definition of the interface.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct sentinel_iface {
    virtual ~sentinel_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = sentinel_iface_impl<Base, Holder, T>;
};

inline constexpr auto sentinel_config
    = tanuki::config<std::default_sentinel_t>{.static_size = tanuki::holder_size<void *, sentinel_iface>,
                                              .static_align = tanuki::holder_align<void *, sentinel_iface>};

} // namespace detail

using sentinel = tanuki::wrap<detail::sentinel_iface, detail::sentinel_config>;

} // namespace facade

#endif
