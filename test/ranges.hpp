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
#include <iterator>
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

#include "bidirectional_iterator.hpp"
#include "forward_iterator.hpp"
#include "input_iterator.hpp"
#include "random_access_iterator.hpp"
#include "sentinel.hpp"

namespace facade
{

namespace detail
{

// Default implementation of the interface.
template <typename, typename, typename, typename, typename, typename, typename, typename,
          template <typename, typename, typename> typename>
struct generic_range_iface_impl {
};

// Definition of the interface.
template <typename V, typename R, typename RR, typename CR, typename CRR,
          template <typename, typename, typename> typename It>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct generic_range_iface {
    virtual ~generic_range_iface() = default;

    virtual It<V, R, RR> begin() = 0;
    virtual sentinel end() = 0;
    virtual It<V, CR, CRR> begin() const = 0;
    [[nodiscard]] virtual sentinel end() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = generic_range_iface_impl<Base, Holder, T, V, R, RR, CR, CRR, It>;
};

// Helper to invoke make_*_iterator().
template <template <typename, typename, typename> typename>
struct make_generic_iterator;

template <>
struct make_generic_iterator<input_iterator> {
    template <typename T>
        requires ud_input_iterator<T>
    auto operator()(T it) const
    {
        return make_input_iterator(std::move(it));
    }
};

template <>
struct make_generic_iterator<forward_iterator> {
    template <typename T>
        requires ud_forward_iterator<T>
    auto operator()(T it) const
    {
        return make_forward_iterator(std::move(it));
    }
};

template <>
struct make_generic_iterator<bidirectional_iterator> {
    template <typename T>
        requires ud_bidirectional_iterator<T>
    auto operator()(T it) const
    {
        return make_bidirectional_iterator(std::move(it));
    }
};

template <>
struct make_generic_iterator<random_access_iterator> {
    template <typename T>
        requires ud_random_access_iterator<T>
    auto operator()(T it) const
    {
        return make_random_access_iterator(std::move(it));
    }
};

// Machinery to detect the presence of begin()/end(). Contrary
// to std::ranges::range, we only require that begin()/end()
// exist, but not that they return standard-compliant iterators/sentinels.
namespace begin_end_impl
{

template <typename T>
concept has_member_begin_end = requires(T &x) {
    x.begin();
    x.end();
};

// NOTE: we need to special-case if T is an array type, because
// in that case we cannot use ADL-based begin()/end() and we need to resort
// instead to std::ranges::begin()/end().
template <typename T>
concept has_adl_begin_end = std::is_array_v<T> || requires(T &x) {
    begin(x);
    end(x);
};

template <typename T>
    requires has_member_begin_end<T>
auto b(T &x)
{
    return x.begin();
}

template <typename T>
    requires has_member_begin_end<T>
auto e(T &x)
{
    return x.end();
}

// NOTE: like in std::ranges::range, we give the precedence
// to member functions if the ADL versions are also available.
template <typename T>
    requires has_adl_begin_end<T> && (!has_member_begin_end<T>)
auto b(T &x)
{
    if constexpr (std::is_array_v<T>) {
        return std::ranges::begin(x);
    } else {
        return begin(x);
    }
}

template <typename T>
    requires has_adl_begin_end<T> && (!has_member_begin_end<T>)
auto e(T &x)
{
    if constexpr (std::is_array_v<T>) {
        return std::ranges::end(x);
    } else {
        return end(x);
    }
}

} // namespace begin_end_impl

template <typename T, typename V, typename R, typename RR, typename CR, typename CRR,
          template <typename, typename, typename> typename It>
concept is_generic_range = requires(T &x) {
    { make_generic_iterator<It>{}(begin_end_impl::b(x)) } -> std::same_as<It<V, R, RR>>;
    // NOTE: these two are the minimal requirements for the sentinel type,
    // and the conceptual requirements for the S and It types of sentinel_box.
    requires std::copyable<decltype(begin_end_impl::e(x))>;
    requires minimal_eq_comparable<decltype(begin_end_impl::b(x)), decltype(begin_end_impl::e(x))>;
    // If we are building a random access range, then we also need to be able to compute the distance
    // between the iterator and the sentinel.
    requires(!std::random_access_iterator<It<V, R, RR>>)
                || with_ptrdiff_t_difference<decltype(begin_end_impl::b(x)), decltype(begin_end_impl::e(x))>;

    // Const counterparts.
    { make_generic_iterator<It>{}(begin_end_impl::b(std::as_const(x))) } -> std::same_as<It<V, CR, CRR>>;
    requires std::copyable<decltype(begin_end_impl::e(std::as_const(x)))>;
    requires minimal_eq_comparable<decltype(begin_end_impl::b(std::as_const(x))),
                                   decltype(begin_end_impl::e(std::as_const(x)))>;
    requires(!std::random_access_iterator<It<V, CR, CRR>>)
                || with_ptrdiff_t_difference<decltype(begin_end_impl::b(std::as_const(x))),
                                             decltype(begin_end_impl::e(std::as_const(x)))>;
};

// Implementation of the interface.
template <typename Base, typename Holder, typename T, typename V, typename R, typename RR, typename CR, typename CRR,
          template <typename, typename, typename> typename It>
    requires std::derived_from<Base, generic_range_iface<V, R, RR, CR, CRR, It>>
             && is_generic_range<tanuki::unwrap_cvref_t<T>, V, R, RR, CR, CRR, It>
struct generic_range_iface_impl<Base, Holder, T, V, R, RR, CR, CRR, It> : public Base {
    It<V, R, RR> begin() final
    {
        return make_generic_iterator<It>{}(begin_end_impl::b(getval<Holder>(this)));
    }
    sentinel end() final
    {
        using iter_t = decltype(begin_end_impl::b(getval<Holder>(this)));
        using sentinel_t = decltype(begin_end_impl::e(getval<Holder>(this)));

        return sentinel(sentinel_box<sentinel_t, iter_t>{begin_end_impl::e(getval<Holder>(this))});
    }
    It<V, CR, CRR> begin() const final
    {
        return make_generic_iterator<It>{}(begin_end_impl::b(getval<Holder>(this)));
    }
    [[nodiscard]] sentinel end() const final
    {
        using iter_t = decltype(begin_end_impl::b(getval<Holder>(this)));
        using sentinel_t = decltype(begin_end_impl::e(getval<Holder>(this)));

        return sentinel(sentinel_box<sentinel_t, iter_t>{begin_end_impl::e(getval<Holder>(this))});
    }
};

// Detection of the iterator type for
// a range. Unlike std::ranges::iterator_t,
// we do not require the range to provide
// standard-compliant iterators.
template <typename T>
using iter_t = decltype(begin_end_impl::b(std::declval<T &>()));

// Implementation of the reference interface.
template <typename V, typename R, typename RR, typename CR, typename CRR,
          template <typename, typename, typename> typename It>
struct generic_range_ref_iface {
    template <typename Wrap>
    struct impl {
        It<V, R, RR> begin()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->begin();
        }
        sentinel end()
        {
            return iface_ptr(*static_cast<Wrap *>(this))->end();
        }

        It<V, CR, CRR> begin() const
        {
            return iface_ptr(*static_cast<const Wrap *>(this))->begin();
        }
        [[nodiscard]] sentinel end() const
        {
            return iface_ptr(*static_cast<const Wrap *>(this))->end();
        }
    };
};

template <typename V, typename R, typename RR, typename CR, typename CRR,
          template <typename, typename, typename> typename It>
struct generic_range_mock {
    // NOTE: use 2 pointers here, as this is arguably
    // how most trivial (sub)ranges are implemented.
    void *ptr1 = nullptr;
    void *ptr2 = nullptr;

    It<V, R, RR> begin();
    sentinel end();
    It<V, CR, CRR> begin() const;
    [[nodiscard]] sentinel end() const;
};

template <typename V, typename R, typename RR, typename CR, typename CRR,
          template <typename, typename, typename> typename It>
inline constexpr auto generic_range_config = tanuki::config<void, generic_range_ref_iface<V, R, RR, CR, CRR, It>>{
    .static_size
    = tanuki::holder_size<generic_range_mock<V, R, RR, CR, CRR, It>, generic_range_iface<V, R, RR, CR, CRR, It>>,
    .static_align
    = tanuki::holder_align<generic_range_mock<V, R, RR, CR, CRR, It>, generic_range_iface<V, R, RR, CR, CRR, It>>,
    .pointer_interface = false};

template <typename V, typename R, typename RR, typename CR, typename CRR,
          template <typename, typename, typename> typename It>
using generic_range = tanuki::wrap<detail::generic_range_iface<V, R, RR, CR, CRR, It>,
                                   detail::generic_range_config<V, R, RR, CR, CRR, It>>;

template <typename T>
using unwrap_cvref2_t = tanuki::unwrap_cvref_t<std::remove_cvref_t<T>>;

template <typename T, template <typename, typename, typename, typename, typename> typename R>
concept generic_ud_input_range = requires() {
    typename deduce_iter_value_t<iter_t<unwrap_cvref2_t<T>>>;
    typename std::iter_reference_t<iter_t<unwrap_cvref2_t<T>>>;
    typename std::iter_rvalue_reference_t<iter_t<unwrap_cvref2_t<T>>>;
    typename std::iter_reference_t<iter_t<const unwrap_cvref2_t<T>>>;
    typename std::iter_rvalue_reference_t<iter_t<const unwrap_cvref2_t<T>>>;
    requires std::constructible_from<
        R<deduce_iter_value_t<iter_t<unwrap_cvref2_t<T>>>, std::iter_reference_t<iter_t<unwrap_cvref2_t<T>>>,
          std::iter_rvalue_reference_t<iter_t<unwrap_cvref2_t<T>>>,
          typename std::iter_reference_t<iter_t<const unwrap_cvref2_t<T>>>,
          typename std::iter_rvalue_reference_t<iter_t<const unwrap_cvref2_t<T>>>>,
        T>;
};

} // namespace detail

template <typename V, typename R, typename RR, typename CR, typename CRR>
using input_range = detail::generic_range<V, R, RR, CR, CRR, input_iterator>;

template <typename V, typename R, typename RR, typename CR, typename CRR>
using forward_range = detail::generic_range<V, R, RR, CR, CRR, forward_iterator>;

template <typename V, typename R, typename RR, typename CR, typename CRR>
using bidirectional_range = detail::generic_range<V, R, RR, CR, CRR, bidirectional_iterator>;

template <typename V, typename R, typename RR, typename CR, typename CRR>
using random_access_range = detail::generic_range<V, R, RR, CR, CRR, random_access_iterator>;

template <typename T>
concept ud_input_range = detail::generic_ud_input_range<T, input_range>;

template <typename T>
concept ud_forward_range = detail::generic_ud_input_range<T, forward_range>;

template <typename T>
concept ud_bidirectional_range = detail::generic_ud_input_range<T, bidirectional_range>;

template <typename T>
concept ud_random_access_range = detail::generic_ud_input_range<T, random_access_range>;

// Factory functions.
#define FACADE_DEFINE_RANGE_FACTORY(tp)                                                                                \
    template <typename T>                                                                                              \
        requires(ud_##tp##_range<T>)                                                                                   \
    auto make_##tp##_range(T &&x)                                                                                      \
    {                                                                                                                  \
        return tp##_range<detail::deduce_iter_value_t<detail::iter_t<detail::unwrap_cvref2_t<T>>>,                     \
                          std::iter_reference_t<detail::iter_t<detail::unwrap_cvref2_t<T>>>,                           \
                          std::iter_rvalue_reference_t<detail::iter_t<detail::unwrap_cvref2_t<T>>>,                    \
                          std::iter_reference_t<detail::iter_t<const detail::unwrap_cvref2_t<T>>>,                     \
                          std::iter_rvalue_reference_t<detail::iter_t<const detail::unwrap_cvref2_t<T>>>>(             \
            std::forward<T>(x));                                                                                       \
    }

FACADE_DEFINE_RANGE_FACTORY(input)
FACADE_DEFINE_RANGE_FACTORY(forward)
FACADE_DEFINE_RANGE_FACTORY(bidirectional)
FACADE_DEFINE_RANGE_FACTORY(random_access)

#undef FACADE_DEFINE_RANGE_FACTORY

} // namespace facade

#endif
