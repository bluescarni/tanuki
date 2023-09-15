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
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

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

    // NOTE: these are meant to implement virtual copy/move primitives for the holder class.
    [[nodiscard]] virtual std::pair<IFace *, value_iface *> clone() const = 0;
    virtual std::pair<IFace *, value_iface *> copy_init(void *) const = 0;
    virtual std::pair<IFace *, value_iface *> move_init(void *) && noexcept = 0;
    virtual void move_assign(void *) && noexcept = 0;
};

// NOTE: how to selectively hide the details of this class from IFaceImpl?
template <typename T, typename IFace, template <typename> typename IFaceImpl>
struct TANUKI_DLL_PUBLIC_INLINE_CLASS holder final : public value_iface<IFace>,
                                                     public IFaceImpl<holder<T, IFace, IFaceImpl>> {
    TANUKI_NO_UNIQUE_ADDRESS T m_value;

    // TODO: make less restrictive - allow throwing dtor and move
    // ctor/assignment for T but leave the noexcepts, so that
    // if they throw the program will terminate.
    static_assert(std::is_nothrow_destructible_v<T>);
    static_assert(std::is_copy_constructible_v<T>);
    static_assert(std::is_nothrow_move_constructible_v<T>);
    static_assert(std::is_nothrow_move_assignable_v<T>);

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

    // Clone this, and cast the result to the two bases.
    [[nodiscard]] std::pair<IFace *, value_iface<IFace> *> clone() const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = new holder(m_value);
        return {ret, ret};
    }

    // Copy-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    std::pair<IFace *, value_iface<IFace> *> copy_init(void *ptr) const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = ::new (ptr) holder(m_value);
        return {ret, ret};
    }

    // Move-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    std::pair<IFace *, value_iface<IFace> *> move_init(void *ptr) && noexcept final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = ::new (ptr) holder(std::move(m_value));
        return {ret, ret};
    }

    // Move-assign m_value into the value of the holder object assumed to be stored in ptr.
    void move_assign(void *ptr) && noexcept final
    {
        std::launder(reinterpret_cast<holder *>(ptr))->m_value = std::move(m_value);
    }
};

template <typename IFace, template <typename> typename IFaceImpl, std::size_t NSlots>
class TANUKI_DLL_PUBLIC_INLINE_CLASS wrap_sbo
{
    static_assert(NSlots > 0u);

    using value_iface_t = value_iface<IFace>;

    static constexpr std::size_t ptr_size = sizeof(void *);
    static constexpr std::size_t ptr_align = alignof(void *);
    static_assert(sizeof(IFace *) <= ptr_size);
    static_assert(sizeof(value_iface_t *) <= ptr_size);
    static_assert(alignof(IFace *) <= ptr_align);
    static_assert(alignof(value_iface_t *) <= ptr_align);

    static_assert(NSlots <= std::numeric_limits<std::size_t>::max() - 2u);
    static_assert(ptr_size <= std::numeric_limits<std::size_t>::max() / (NSlots + 2u));
    static constexpr std::size_t static_size = ptr_size * NSlots;
    static constexpr std::size_t static_storage = static_size + ptr_size * 2u;

    alignas(std::max_align_t) std::byte storage[static_storage];

    std::tuple<const IFace *, const value_iface_t *, bool> stype() const noexcept
    {
        auto *p_iface = *std::launder(reinterpret_cast<IFace *const *>(storage + static_size));
        auto *pv_iface = *std::launder(reinterpret_cast<value_iface_t *const *>(storage + static_size + ptr_size));

        if (p_iface == nullptr) {
            const auto *ret = *std::launder(reinterpret_cast<IFace *const *>(storage));
            assert((ret == nullptr) == (pv_iface == nullptr));
            return {ret, pv_iface, false};
        } else {
            assert((p_iface == nullptr) == (pv_iface == nullptr));
            return {p_iface, pv_iface, true};
        }
    }
    std::tuple<IFace *, value_iface_t *, bool> stype() noexcept
    {
        auto *p_iface = *std::launder(reinterpret_cast<IFace **>(storage + static_size));
        auto *pv_iface = *std::launder(reinterpret_cast<value_iface_t **>(storage + static_size + ptr_size));

        if (p_iface == nullptr) {
            auto *ret = *std::launder(reinterpret_cast<IFace **>(storage));
            assert((ret == nullptr) == (pv_iface == nullptr));
            return {ret, pv_iface, false};
        } else {
            assert((p_iface == nullptr) == (pv_iface == nullptr));
            return {p_iface, pv_iface, true};
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
        static_assert(alignof(holder_t) <= alignof(std::max_align_t), "Over-aligned types are not supported.");

        if constexpr (sizeof(holder_t) <= static_size) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *d_ptr = ::new (storage) holder_t(std::forward<T>(x));
            ::new (storage + static_size) IFace *(d_ptr);
            ::new (storage + static_size + ptr_size) value_iface_t *(d_ptr);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto d_ptr = new holder_t(std::forward<T>(x));
            ::new (storage) IFace *(d_ptr);
            ::new (storage + static_size) IFace *(nullptr);
            ::new (storage + static_size + ptr_size) value_iface_t *(d_ptr);
        }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    wrap_sbo(const wrap_sbo &other)
    {
        const auto [_, pv_iface, st] = other.stype();

        if (st) {
            auto [new_p_iface, new_pv_iface] = pv_iface->copy_init(storage);
            ::new (storage + static_size) IFace *(new_p_iface);
            ::new (storage + static_size + ptr_size) value_iface_t *(new_pv_iface);
        } else {
            auto [new_p_iface, new_pv_iface] = pv_iface->clone();
            ::new (storage) IFace *(new_p_iface);
            ::new (storage + static_size) IFace *(nullptr);
            ::new (storage + static_size + ptr_size) value_iface_t *(new_pv_iface);
        }
    }

private:
    void move_init_from(wrap_sbo &&other) noexcept
    {
        const auto [p_iface, pv_iface, st] = other.stype();

        if (st) {
            auto [new_p_iface, new_pv_iface] = std::move(*pv_iface).move_init(storage);
            ::new (storage + static_size) IFace *(new_p_iface);
            ::new (storage + static_size + ptr_size) value_iface_t *(new_pv_iface);
        } else {
            ::new (storage) IFace *(p_iface);
            ::new (storage + static_size) IFace *(nullptr);
            ::new (storage + static_size + ptr_size) value_iface_t *(pv_iface);

            // Nullify the interface pointers in other, so that, on destruction,
            // we will be calling delete on a nullptr.
            // NOTE: directly overwriting the existing pointer
            // with new() is ok - no need to call the destructor
            // on pointer objects.
            ::new (other.storage) IFace *(nullptr);
            ::new (other.storage + static_size + ptr_size) value_iface_t *(nullptr);
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
        const auto [p_iface, _, st] = stype();

        if (st) {
            p_iface->~IFace();
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete (p_iface);
        }
    }

public:
    ~wrap_sbo()
    {
        destroy();
    }

    wrap_sbo &operator=(wrap_sbo &&other) noexcept
    {
        // Handle self-assign.
        if (this == &other) {
            return *this;
        }

        // Handle different internal types.
        if (type_idx() != other.type_idx()) {
            destroy();
            // NOTE: move_init_from() will re-init the interface pointers
            // with placement new. This is ok, no need to call
            // the destructor on pointer objects.
            move_init_from(std::move(other));
            return *this;
        }

        // The internal types are the same.
        const auto [p_iface1, pv_iface1, st1] = other.stype();

        // The storage flags must match, as they depend only
        // on the internal types.
        assert(st1 == std::get<2>(stype()));

        if (st1) {
            // For static storage, directly move assign the internal value.
            std::move(*pv_iface1).move_assign(storage);
        } else {
            // NOTE: similar to the move ctor.
            // NOTE: no need to set null on (storage + static_size), as
            // it should be already on null.
            ::new (storage) IFace *(p_iface1);
            ::new (storage + static_size + ptr_size) value_iface_t *(pv_iface1);

            ::new (other.storage) IFace *(nullptr);
            ::new (other.storage + static_size + ptr_size) value_iface_t *(nullptr);
        }

        return *this;
    }

    [[nodiscard]] std::type_index type_idx() const noexcept
    {
        return std::get<1>(stype())->type_idx();
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
