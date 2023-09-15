// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef TANUKI_TANUKI_HPP
#define TANUKI_TANUKI_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include <iostream>

// Versioning.
#define TANUKI_VERSION_STRING "1.0.0"
#define TANUKI_VERSION_MAJOR 1
#define TANUKI_VERSION_MINOR 0
#define TANUKI_VERSION_PATCH 0

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

#define TANUKI_BEGIN_NAMESPACE                                                                                         \
    namespace tanuki                                                                                                   \
    {                                                                                                                  \
    inline namespace v1 TANUKI_ABI_TAG_ATTR                                                                            \
    {

#define TANUKI_END_NAMESPACE                                                                                           \
    }                                                                                                                  \
    }

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

#endif

TANUKI_BEGIN_NAMESPACE

namespace detail
{

template <std::size_t StaticSize>
struct value_semantics : std::integral_constant<std::size_t, StaticSize> {
};

struct reference_semantics_impl {
};

} // namespace detail

template <std::size_t StaticSize>
using sbo_value_semantics = detail::value_semantics<StaticSize>;

using no_sbo_value_semantics = sbo_value_semantics<0>;

using default_sbo_value_semantics = sbo_value_semantics<1>;

using reference_semantics = detail::reference_semantics_impl;

namespace detail
{

// Interface containing methods to interact
// with the value in the holder class.
template <typename IFace>
struct TANUKI_DLL_PUBLIC_INLINE_CLASS value_iface {
    value_iface() = default;
    value_iface(const value_iface &) = delete;
    value_iface(value_iface &&) noexcept = delete;
    value_iface &operator=(const value_iface &) = delete;
    value_iface &operator=(value_iface &&) noexcept = delete;
    virtual ~value_iface() = default;

    [[nodiscard]] virtual std::type_index type_idx() const noexcept = 0;

    virtual IFace *clone() const = 0;
    virtual IFace *copy_init(void *) const = 0;
    virtual IFace *move_init(void *) && noexcept = 0;
    virtual void move_assign(void *) && noexcept = 0;
};

template <typename T, typename IFace, template <typename> typename IFaceImpl>
struct TANUKI_DLL_PUBLIC_INLINE_CLASS holder final : public value_iface<IFace>,
                                                     public IFaceImpl<holder<T, IFace, IFaceImpl>> {
    TANUKI_NO_UNIQUE_ADDRESS T m_value;

    static_assert(std::is_nothrow_destructible_v<T>);
    static_assert(std::is_copy_constructible_v<T>);
    static_assert(std::is_nothrow_move_constructible_v<T>);

    using value_type = T;

    holder() = delete;
    holder(const holder &) = delete;
    holder(holder &&) noexcept = delete;
    holder &operator=(const holder &) = delete;
    holder &operator=(holder &&) noexcept = delete;
    explicit holder(const T &x) : m_value(x) {}
    explicit holder(T &&x) : m_value(std::move(x)) {}
    ~holder() final = default;

    [[nodiscard]] std::type_index type_idx() const noexcept final
    {
        return typeid(T);
    }

    IFace *clone() const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new holder(m_value);
    }

    IFace *copy_init(void *ptr) const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return ::new (ptr) holder(m_value);
    }

    IFace *move_init(void *ptr) && noexcept final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return ::new (ptr) holder(std::move(m_value));
    }

    void move_assign(void *ptr) && noexcept final
    {
        std::launder(reinterpret_cast<holder *>(ptr))->m_value = std::move(m_value);
    }
};

template <typename IFace, template <typename> typename IFaceImpl, std::size_t NSlots>
class TANUKI_DLL_PUBLIC_INLINE_CLASS wrap_sbo
{
    static_assert(NSlots > 0u);

    static constexpr std::size_t ptr_size = sizeof(IFace *);
    static_assert(NSlots < std::numeric_limits<std::size_t>::max());
    static_assert(ptr_size <= std::numeric_limits<std::size_t>::max() / (NSlots + 1u));
    static constexpr std::size_t static_size = ptr_size * NSlots;
    static constexpr std::size_t static_storage = static_size + ptr_size;

    alignas(std::max_align_t) std::byte storage[static_storage];

    std::pair<const IFace *, bool> stype() const noexcept
    {
        auto *p = std::launder(reinterpret_cast<IFace *const *>(storage + static_size));

        if (*p == nullptr) {
            return {*std::launder(reinterpret_cast<IFace *const *>(storage)), false};
        } else {
            return {*p, true};
        }
    }
    std::pair<IFace *, bool> stype() noexcept
    {
        auto *p = std::launder(reinterpret_cast<IFace **>(storage + static_size));

        if (*p == nullptr) {
            return {*std::launder(reinterpret_cast<IFace **>(storage)), false};
        } else {
            return {*p, true};
        }
    }

public:
    wrap_sbo() = delete;

    // TODO concept checks on T.
    template <typename T>
        requires(!std::same_as<std::remove_cvref_t<T>, wrap_sbo>)
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload,cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    explicit wrap_sbo(T &&x)
    {
        using holder_t = holder<std::remove_cvref_t<T>, IFace, IFaceImpl>;
        static_assert(alignof(holder_t) <= alignof(std::max_align_t),
                      "Over-aligned types do not support the small buffer optimisation.");

        std::cout << "Size of T: " << sizeof(T) << '\n';
        std::cout << "Size of stored: " << sizeof(holder_t) << '\n';
        std::cout << "Static size: " << static_size << "\n\n";

        if constexpr (sizeof(holder_t) <= static_size) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *d_ptr = ::new (storage) holder_t(std::forward<T>(x));
            ::new (storage + static_size) IFace *(d_ptr);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto d_ptr = new holder_t(std::forward<T>(x));
            ::new (storage) IFace *(d_ptr);
            ::new (storage + static_size) IFace *(nullptr);
        }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    wrap_sbo(const wrap_sbo &other)
    {
        const auto [p, st] = other.stype();

        const auto *dptr = dynamic_cast<const value_iface<IFace> *>(p);

        assert(dptr != nullptr);

        if (st) {
            auto *nptr = dptr->copy_init(storage);
            ::new (storage + static_size) IFace *(nptr);
        } else {
            auto *nptr = dptr->clone();
            ::new (storage) IFace *(nptr);
            ::new (storage + static_size) IFace *(nullptr);
        }
    }

private:
    void move_init_from(wrap_sbo &&other) noexcept
    {
        const auto [p, st] = other.stype();

        if (st) {
            auto *dptr = dynamic_cast<value_iface<IFace> *>(p);
            assert(dptr != nullptr);
            auto *nptr = std::move(*dptr).move_init(storage);
            ::new (storage + static_size) IFace *(nptr);
        } else {
            ::new (storage) IFace *(p);
            ::new (storage + static_size) IFace *(nullptr);

            ::new (other.storage) IFace *(nullptr);
        }
    }

public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    wrap_sbo(wrap_sbo &&other) noexcept
    {
        move_init_from(std::move(other));
    }

private:
    void destroy() noexcept
    {
        const auto [p, st] = stype();

        if (st) {
            std::cout << "static size delete\n\n";
            p->~IFace();
        } else {
            std::cout << "dynamics delete\n\n";
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete (p);
        }
    }

public:
    ~wrap_sbo()
    {
        destroy();
    }

    wrap_sbo &operator=(wrap_sbo &&other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        if (type_idx() != other.type_idx()) {
            destroy();
            move_init_from(std::move(other));
            return *this;
        }

        const auto [p0, st0] = stype();
        const auto [p1, st1] = other.stype();

        assert(st0 == st1);

        if (st0) {
            auto *dptr1 = dynamic_cast<value_iface<IFace> *>(p1);
            assert(dptr1 != nullptr);

            std::move(*dptr1).move_assign(storage);
        } else {
        }

        return *this;
    }

    [[nodiscard]] std::type_index type_idx() const noexcept
    {
        return dynamic_cast<const value_iface<IFace> *>(stype().first)->type_idx();
    }
};

template <typename IFace, template <typename> typename IFaceImpl, typename Semantics>
struct wrap_selector {
};

template <typename IFace, template <typename> typename IFaceImpl>
struct wrap_selector<IFace, IFaceImpl, default_sbo_value_semantics> {
    using type = wrap_sbo<IFace, IFaceImpl, 6>;
};

template <typename IFace, template <typename> typename IFaceImpl, std::size_t StaticSize>
    requires(StaticSize > 1u)
struct wrap_selector<IFace, IFaceImpl, sbo_value_semantics<StaticSize>> {
    // TODO bytes -> pointer slots conversion.
    // NOTE: need at least 1 slot in any case, need to round up?
    // using type = wrap_sbo<IFace, IFaceImpl, 3>;
};

} // namespace detail

template <typename IFace, template <typename> typename IFaceImpl, typename Semantics = default_sbo_value_semantics>
using wrap = typename detail::wrap_selector<IFace, IFaceImpl, Semantics>::type;

TANUKI_END_NAMESPACE

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

#undef TANUKI_ABI_TAG_ATTR
#undef TANUKI_DLL_PUBLIC_INLINE_CLASS
#undef TANUKI_NO_UNIQUE_ADDRESS

#endif
