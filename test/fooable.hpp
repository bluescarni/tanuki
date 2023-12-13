// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef TANUKI_TEST_FOOABLE_HPP
#define TANUKI_TEST_FOOABLE_HPP

#include <tanuki/tanuki.hpp>

#if defined(_WIN32) || defined(__CYGWIN__)

#define FOOABLE_VISIBLE

#elif defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)

#define FOOABLE_VISIBLE __attribute__((visibility("default")))

#else

#define FOOABLE_VISIBLE

#endif

namespace fooable
{

template <typename Base, typename Holder, typename T, typename U>
    requires(requires(const T &x) { static_cast<void>(x.foo()); })
struct FOOABLE_VISIBLE foo_iface_impl : public Base, tanuki::iface_impl_helper<Base, Holder> {
    void foo() const final
    {
        this->value().foo();
    }
};

template <typename U>
// NOLINTNEXTLINE
struct FOOABLE_VISIBLE foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T, U>;
};

template <typename U>
struct FOOABLE_VISIBLE foo_ref_iface {
    template <typename Wrap>
    struct impl {
        TANUKI_REF_IFACE_MEMFUN(foo)
    };
};

struct FOOABLE_VISIBLE foo_model {
    void foo() const {}
    int n = 0;

    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & n;
    }
};

template <typename U>
inline constexpr auto foo_wrap_config
    = tanuki::config<void, foo_ref_iface<U>>{.static_size = tanuki::holder_size<foo_model, foo_iface<U>>};

template <typename U>
using foo_wrap = tanuki::wrap<foo_iface<U>, foo_wrap_config<U>>;

} // namespace fooable

#undef FOOABLE_VISIBLE

TANUKI_S11N_WRAP_EXPORT_KEY(fooable::foo_model, fooable::foo_iface<int>)

#endif
