// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef FACADE_RANGES_HPP
#define FACADE_RANGES_HPP

#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

#include "bidirectional_iterator.hpp"
#include "forward_iterator.hpp"
#include "input_iterator.hpp"
#include "random_access_iterator.hpp"

namespace facade
{

namespace detail
{

// Implementation of the interface.
template <typename, typename, typename, typename, typename, typename, template <typename, typename, typename> typename>
struct generic_range_iface_iface_impl {
};

// Definition of the interface.
template <typename V, typename R, typename RR, template <typename, typename, typename> typename It>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct generic_range_iface {
    virtual ~generic_range_iface() = default;

    virtual It<V, R, RR> begin() = 0;
    virtual It<V, R, RR> end() = 0;

    template <typename Base, typename Holder, typename T>
    using impl = generic_range_iface_iface_impl<Base, Holder, T, V, R, RR, It>;
};

template <template <typename, typename, typename> typename>
struct make_generic_iterator;

template <>
struct make_generic_iterator<forward_iterator> {
    template <typename T>
    auto operator()(T it) const -> decltype(make_forward_iterator(std::move(it)))
    {
        return make_forward_iterator(std::move(it));
    }
};

template <>
struct make_generic_iterator<bidirectional_iterator> {
    template <typename T>
    auto operator()(T it) const -> decltype(make_bidirectional_iterator(std::move(it)))
    {
        return make_bidirectional_iterator(std::move(it));
    }
};

template <>
struct make_generic_iterator<random_access_iterator> {
    template <typename T>
    auto operator()(T it) const -> decltype(make_random_access_iterator(std::move(it)))
    {
        return make_random_access_iterator(std::move(it));
    }
};

template <typename T, typename V, typename R, typename RR, template <typename, typename, typename> typename It>
concept is_generic_range = requires(T &r) {
    requires std::ranges::range<T>;
    {
        make_generic_iterator<It>{}(std::ranges::begin(r))
    } -> std::same_as<It<V, R, RR>>;
    {
        make_generic_iterator<It>{}(std::ranges::end(r))
    } -> std::same_as<It<V, R, RR>>;
};

template <typename Base, typename Holder, typename T, typename V, typename R, typename RR,
          template <typename, typename, typename> typename It>
    requires std::derived_from<Base, generic_range_iface<V, R, RR, It>>
                 && is_generic_range<std::remove_reference_t<std::unwrap_reference_t<T>>, V, R, RR, It>
struct generic_range_iface_iface_impl<Base, Holder, T, V, R, RR, It> : public Base,
                                                                       tanuki::iface_impl_helper<Base, Holder> {
    It<V, R, RR> begin() final
    {
        return make_generic_iterator<It>{}(std::ranges::begin(this->value()));
    }
    It<V, R, RR> end() final
    {
        return make_generic_iterator<It>{}(std::ranges::end(this->value()));
    }
};

// Implementation of the reference interface.
template <typename V, typename R, typename RR, template <typename, typename, typename> typename It>
struct generic_range_ref_iface {
    template <typename Wrap>
    struct impl {
        It<V, R, RR> begin()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->begin();
        }
        It<V, R, RR> end()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->end();
        }
    };
};

template <typename V, typename R, typename RR, template <typename, typename, typename> typename It>
struct generic_range_mock {
    void *ptr1 = nullptr;
    void *ptr2 = nullptr;

    It<V, R, RR> begin();
    It<V, R, RR> end();
};

template <typename V, typename R, typename RR, template <typename, typename, typename> typename It>
inline constexpr auto generic_range_config = tanuki::config<void, generic_range_ref_iface<V, R, RR, It>>{
    .static_size = tanuki::holder_size<generic_range_mock<V, R, RR, It>, generic_range_iface<V, R, RR, It>>,
    .pointer_interface = false};

template <typename V, typename R, typename RR, template <typename, typename, typename> typename It>
using generic_range
    = tanuki::wrap<detail::generic_range_iface<V, R, RR, It>, detail::generic_range_config<V, R, RR, It>>;

} // namespace detail

template <typename V, typename R, typename RR>
using forward_range = detail::generic_range<V, R, RR, forward_iterator>;

template <typename V, typename R, typename RR>
using bidirectional_range = detail::generic_range<V, R, RR, bidirectional_iterator>;

template <typename V, typename R, typename RR>
using random_access_range = detail::generic_range<V, R, RR, random_access_iterator>;

#define FACADE_DEFINE_RANGE_FACTORY(tp)                                                                                \
    template <typename T>                                                                                              \
    auto make_##tp##_range(T &&x)                                                                                      \
        -> decltype(tp##_range<detail::deduce_iter_value_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,           \
                               std::iter_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,                 \
                               std::iter_rvalue_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>>(         \
            std::forward<T>(x)))                                                                                       \
    {                                                                                                                  \
        return tp##_range<detail::deduce_iter_value_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,                \
                          std::iter_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>,                      \
                          std::iter_rvalue_reference_t<std::ranges::iterator_t<std::remove_cvref_t<T>>>>(              \
            std::forward<T>(x));                                                                                       \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    auto make_##tp##_range(std::reference_wrapper<T> ref)                                                              \
        -> decltype(tp##_range<detail::deduce_iter_value_t<std::ranges::iterator_t<T>>,                                \
                               std::iter_reference_t<std::ranges::iterator_t<T>>,                                      \
                               std::iter_rvalue_reference_t<std::ranges::iterator_t<T>>>(std::move(ref)))              \
    {                                                                                                                  \
        return tp##_range<detail::deduce_iter_value_t<std::ranges::iterator_t<T>>,                                     \
                          std::iter_reference_t<std::ranges::iterator_t<T>>,                                           \
                          std::iter_rvalue_reference_t<std::ranges::iterator_t<T>>>(std::move(ref));                   \
    }

FACADE_DEFINE_RANGE_FACTORY(forward)
FACADE_DEFINE_RANGE_FACTORY(bidirectional)
FACADE_DEFINE_RANGE_FACTORY(random_access)

#undef FACADE_DEFINE_RANGE_FACTORY

} // namespace facade

#endif
