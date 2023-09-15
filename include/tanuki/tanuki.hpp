// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef TANUKI_TANUKI_HPP
#define TANUKI_TANUKI_HPP

#include <concepts>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

// Versioning.
#define TANUKI_VERSION_STRING "1.0.0"
#define TANUKI_VERSION_MAJOR 1
#define TANUKI_VERSION_MINOR 0
#define TANUKI_VERSION_PATCH 0
#define TANUKI_ABI_VERSION 1

// Visibility setup.
#if defined(_WIN32) || defined(__CYGWIN__)

#define TANUKI_DLL_PUBLIC_INLINE_CLASS

#elif defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)

#define TANUKI_DLL_PUBLIC_INLINE_CLASS __attribute__((visibility("default")))

#else

#define TANUKI_DLL_PUBLIC_INLINE_CLASS

#endif

// No unique address setup.
#if defined(_MSC_VER)

#define TANUKI_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]

#else

#define TANUKI_NO_UNIQUE_ADDRESS [[no_unique_address]]

#endif

// ABI tag setup.
#if defined(__GNUC__) || defined(__clang__)

#define TANUKI_ABI_TAG_ATTR __attribute__((abi_tag))

#else

#define TANUKI_ABI_TAG_ATTR

#endif

#define TANUKI_BEGIN_NAMESPACE(abi_version)                                                                            \
    namespace tanuki                                                                                                   \
    {                                                                                                                  \
    inline namespace v##abi_version TANUKI_ABI_TAG_ATTR                                                                \
    {

#define TANUKI_END_NAMESPACE                                                                                           \
    }                                                                                                                  \
    }

TANUKI_BEGIN_NAMESPACE(TANUKI_ABI_VERSION)

template <typename, template <typename> typename>
class TANUKI_DLL_PUBLIC_INLINE_CLASS wrap;

namespace detail
{

// Interface containing basic methods
// to manage storage in the wrap class.
template <typename IFace>
struct TANUKI_DLL_PUBLIC_INLINE_CLASS storage_iface {
    storage_iface(const storage_iface &) = delete;
    storage_iface(storage_iface &&) noexcept = delete;
    storage_iface &operator=(const storage_iface &) = delete;
    storage_iface &operator=(storage_iface &&) noexcept = delete;
    virtual ~storage_iface() = default;

    virtual IFace *clone() const = 0;
    virtual IFace *copy_into(void *) const = 0;
    virtual IFace *move_into(void *) && = 0;
};

template <typename T, typename IFace, template <typename> typename IFaceImpl>
struct TANUKI_DLL_PUBLIC_INLINE_CLASS iface_impl final : public storage_iface<IFace>,
                                                         public IFaceImpl<iface_impl<T, IFace, IFaceImpl>> {
    using value_type = T;

    TANUKI_NO_UNIQUE_ADDRESS T m_value;

    iface_impl() = delete;
    iface_impl(const iface_impl &) = delete;
    iface_impl(iface_impl &&) = delete;
    iface_impl &operator=(const iface_impl &) = delete;
    iface_impl &operator=(iface_impl &&) = delete;
    explicit iface_impl(const T &x) : m_value(x) {}
    explicit iface_impl(T &&x) : m_value(std::move(x)) {}
    ~iface_impl() final = default;

    IFace *clone() const final
    {
        return std::make_unique<iface_impl>(m_value).release();
    }

    IFace *copy_into(void *ptr) const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return ::new (ptr) iface_impl(m_value);
    }

    IFace *move_into(void *ptr) && final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return ::new (ptr) iface_impl(std::move(m_value));
    }
};

} // namespace detail

template <typename IFace, template <typename> typename IFaceImpl>
class TANUKI_DLL_PUBLIC_INLINE_CLASS wrap
{
    IFace *m_ptr = nullptr;

public:
    template <typename T>
        requires(!std::same_as<wrap, std::remove_cvref_t<T>>)
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    explicit wrap(T &&x)
    {
        using iface_impl_t = detail::iface_impl<std::remove_cvref_t<T>, IFace, IFaceImpl>;
        // std::cout << sizeof(iface_impl_t) << '\n';
        m_ptr = std::make_unique<iface_impl_t>(std::forward<T>(x)).release();
    }

    IFace *operator->() noexcept
    {
        return m_ptr;
    }

    const IFace *operator->() const noexcept
    {
        return m_ptr;
    }

    IFace &operator*() noexcept
    {
        return *m_ptr;
    }

    const IFace &operator*() const noexcept
    {
        return *m_ptr;
    }
};

TANUKI_END_NAMESPACE

#undef TANUKI_ABI_TAG_ATTR
#undef TANUKI_DLL_PUBLIC_INLINE_CLASS
#undef TANUKI_NO_UNIQUE_ADDRESS

#endif
