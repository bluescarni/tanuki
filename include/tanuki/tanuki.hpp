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

// NOTE: an argument of this tag type is appended
// to the signature of all member functions in value_iface. The purpose
// is to prevent the user from accidentally implementing
// functions from value_iface in the interface implementations.
struct vtag {
};

// Interface containing methods to interact
// with the value in the holder class.
// NOTE: templating this on IFace is not strictly necessary,
// as we could just use void * instead of IFace * and then
// cast back to Iface * as needed. However, templating
// gives a higher degree of type safety as there's no risk
// of casting to the wrong type in the wrap class (which already
// does enough memory shenanigans). Perhaps in the future
// we can reconsider if we want to reduce binary bloat.
template <typename IFace>
struct value_iface {
    value_iface() = default;
    value_iface(const value_iface &) = delete;
    value_iface(value_iface &&) noexcept = delete;
    value_iface &operator=(const value_iface &) = delete;
    value_iface &operator=(value_iface &&) noexcept = delete;
    virtual ~value_iface() = default;

    // Access to the value and its type.
    [[nodiscard]] virtual const void *value_ptr(vtag) const noexcept = 0;
    [[nodiscard]] virtual void *value_ptr(vtag) noexcept = 0;
    [[nodiscard]] virtual std::type_index value_type_index(vtag) const noexcept = 0;

    // Methods to implement virtual copy/move primitives for the holder class.
    [[nodiscard]] virtual std::pair<IFace *, value_iface *> clone(vtag) const = 0;
    virtual std::pair<IFace *, value_iface *> copy_init_holder(void *, vtag) const = 0;
    virtual std::pair<IFace *, value_iface *> move_init_holder(void *, vtag) && noexcept = 0;
    virtual void copy_assign_value(void *, vtag) const = 0;
    virtual void move_assign_value(void *, vtag) && noexcept = 0;
};

template <typename T, typename IFace, template <typename> typename IFaceImpl>
struct holder final : public value_iface<IFace>, public IFaceImpl<holder<T, IFace, IFaceImpl>> {
    TANUKI_NO_UNIQUE_ADDRESS T m_value;

    using value_type = T;

    static_assert(std::destructible<T>);

    holder() = delete;
    holder(const holder &) = delete;
    holder(holder &&) noexcept = delete;
    holder &operator=(const holder &) = delete;
    holder &operator=(holder &&) noexcept = delete;
    ~holder() final = default;

    // NOTE: no concept requirements on these as they are performed
    // in the wrap constructor.
    explicit holder(const T &x) : m_value(x) {}
    explicit holder(T &&x) noexcept : m_value(std::move(x)) {}

    // NOTE: mark everything else as private so that it is going to be
    // unreachable from IFaceImpl.
private:
    [[nodiscard]] std::type_index value_type_index(vtag) const noexcept final
    {
        return typeid(T);
    }
    [[nodiscard]] const void *value_ptr(vtag) const noexcept final
    {
        return std::addressof(m_value);
    }
    [[nodiscard]] void *value_ptr(vtag) noexcept final
    {
        return std::addressof(m_value);
    }

    // Clone this, and cast the result to the two bases.
    [[nodiscard]] std::pair<IFace *, value_iface<IFace> *> clone(vtag) const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = new holder(m_value);
        return {ret, ret};
    }
    // Copy-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    std::pair<IFace *, value_iface<IFace> *> copy_init_holder(void *ptr, vtag) const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = ::new (ptr) holder(m_value);
        return {ret, ret};
    }
    // Move-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    std::pair<IFace *, value_iface<IFace> *> move_init_holder(void *ptr, vtag) && noexcept final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = ::new (ptr) holder(std::move(m_value));
        return {ret, ret};
    }
    // Copy-assign m_value into the object of type T assumed to be stored in ptr.
    void copy_assign_value(void *ptr, vtag) const final
    {
        // NOTE: I don't think it is necessary to invoke launder here,
        // as ptr is always supposed to come from an invocation of value_ptr(),
        // which just does a static cast to void *. Since we are assuming that
        // copy_assign_value() is called only when assigning holders containing
        // the same T, the conversion chain should boil down to T * -> void * -> T *, which
        // does not require laundering.
        *static_cast<T *>(ptr) = m_value;
    }
    // Move-assign m_value into the object of type T assumed to be stored in ptr.
    void move_assign_value(void *ptr, vtag) && noexcept final
    {
        // NOTE: see comments above.
        *static_cast<T *>(ptr) = std::move(m_value);
    }
};

// Implementation of basic storage for the wrap class.
template <typename IFace, std::size_t StaticStorageSize, std::size_t StaticStorageAlignment>
struct wrap_storage {
    static_assert(StaticStorageSize > 0u);

    alignas(StaticStorageAlignment) std::byte static_storage[StaticStorageSize];
    IFace *m_p_iface;
    value_iface<IFace> *m_pv_iface;
};

template <typename IFace, std::size_t StaticStorageAlignment>
struct wrap_storage<IFace, 0, StaticStorageAlignment> {
    IFace *m_p_iface;
    value_iface<IFace> *m_pv_iface;
};

// Implementation details for composite interfaces.
template <typename Holder, typename IFace, template <typename, typename> typename Impl0,
          template <typename, typename> typename... Impls>
struct iface_composer {
    using type = Impl0<Holder, typename iface_composer<Holder, IFace, Impls...>::type>;
};

template <typename Holder, typename IFace, template <typename, typename> typename Impl0>
struct iface_composer<Holder, IFace, Impl0> {
    using type = Impl0<Holder, IFace>;
};

} // namespace detail

// Composite interface implementation.
template <typename Holder, typename IFace, template <typename, typename> typename... Impls>
    requires(sizeof...(Impls) > 0u)
using composite_iface_impl = typename detail::iface_composer<Holder, IFace, Impls...>::type;

// Configuration structure for the wrap class.
struct config {
    std::size_t static_size = 48;
    std::size_t static_alignment = alignof(std::max_align_t);
    bool explicit_iface_conversion = true;
};

inline constexpr auto default_config = config{};

template <typename IFace, template <typename> typename IFaceImpl, auto Cfg = default_config>
    requires std::is_polymorphic_v<IFace> && std::has_virtual_destructor_v<IFace> && (Cfg.static_alignment > 0u)
             && ((Cfg.static_alignment & (Cfg.static_alignment - 1u)) == 0u)
class wrap : private detail::wrap_storage<IFace, Cfg.static_size, Cfg.static_alignment>
{
    using value_iface_t = detail::value_iface<IFace>;

    // Helpers to fetch the interface pointers and the storage type.
    std::tuple<const IFace *, const value_iface_t *, bool> stype() const noexcept
        requires(Cfg.static_size > 0u)
    {
        if (this->m_p_iface == nullptr) {
            // Dynamic storage.
            const auto *ret = *std::launder(reinterpret_cast<IFace *const *>(this->static_storage));
            // NOTE: if one interface pointer is null, the other must be as well, and vice-versa.
            // Null interface pointers with dynamic storage indicate that this object is in the
            // invalid state.
            assert((ret == nullptr) == (this->m_pv_iface == nullptr));
            return {ret, this->m_pv_iface, false};
        } else {
            // Static storage.
            // NOTE: with static storage, the interface pointers cannot be null.
            assert(this->m_p_iface != nullptr && this->m_pv_iface != nullptr);
            return {this->m_p_iface, this->m_pv_iface, true};
        }
    }
    std::tuple<IFace *, value_iface_t *, bool> stype() noexcept
        requires(Cfg.static_size > 0u)
    {
        if (this->m_p_iface == nullptr) {
            auto *ret = *std::launder(reinterpret_cast<IFace **>(this->static_storage));
            assert((ret == nullptr) == (this->m_pv_iface == nullptr));
            return {ret, this->m_pv_iface, false};
        } else {
            assert(this->m_p_iface != nullptr && this->m_pv_iface != nullptr);
            return {this->m_p_iface, this->m_pv_iface, true};
        }
    }

    // The holder type corresponding to the type T
    // passed to the generic ctor.
    template <typename T>
    using make_holder_t = detail::holder<std::remove_cvref_t<T>, IFace, IFaceImpl>;

public:
    wrap() = delete;

    // Generic ctor.
    template <typename T>
        requires
        // Must not compete with copy/move.
        (!std::same_as<std::remove_cvref_t<T>, wrap>) &&
        // T must be destructible.
        std::destructible<std::remove_cvref_t<T>> &&
        // These checks are for verifying that IFace is a base
        // of the interface implementation and that all
        // interface requirements have been implemented.
        std::derived_from<make_holder_t<T &&>, IFace> && std::constructible_from<make_holder_t<T &&>, T &&> &&
        // Alignment checks.
        (sizeof(make_holder_t<T &&>) > Cfg.static_size || alignof(make_holder_t<T &&>) <= Cfg.static_alignment)
        // NOLINTNEXTLINE(bugprone-forwarding-reference-overload,cppcoreguidelines-pro-type-member-init,hicpp-member-init)
        explicit wrap(T &&x)
    {
        using holder_t = make_holder_t<T &&>;

        if constexpr (sizeof(holder_t) <= Cfg.static_size) {
            // Static storage.
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *d_ptr = ::new (this->static_storage) holder_t(std::forward<T>(x));
            this->m_p_iface = d_ptr;
            this->m_pv_iface = d_ptr;
        } else {
            // Dynamic storage.
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto d_ptr = new holder_t(std::forward<T>(x));
            ::new (this->static_storage) IFace *(d_ptr);
            this->m_p_iface = nullptr;
            this->m_pv_iface = d_ptr;
        }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    wrap(const wrap &other)
    {
        const auto [_, pv_iface, st] = other.stype();

        if (st) {
            // Other has static storage.
            auto [new_p_iface, new_pv_iface] = pv_iface->copy_init_holder(this->static_storage, detail::vtag{});
            this->m_p_iface = new_p_iface;
            this->m_pv_iface = new_pv_iface;
        } else {
            // Other has dynamic storage.
            auto [new_p_iface, new_pv_iface] = pv_iface->clone(detail::vtag{});
            ::new (this->static_storage) IFace *(new_p_iface);
            this->m_p_iface = nullptr;
            this->m_pv_iface = new_pv_iface;
        }
    }

private:
    void move_init_from(wrap &&other) noexcept
    {
        const auto [p_iface, pv_iface, st] = other.stype();

        if (st) {
            // Other has static storage.
            auto [new_p_iface, new_pv_iface]
                = std::move(*pv_iface).move_init_holder(this->static_storage, detail::vtag{});
            this->m_p_iface = new_p_iface;
            this->m_pv_iface = new_pv_iface;
        } else {
            // Other has dynamic storage.
            ::new (this->static_storage) IFace *(p_iface);
            this->m_p_iface = nullptr;
            this->m_pv_iface = pv_iface;

            // Nullify the interface pointers in other, so that, on destruction,
            // we will be calling delete on a nullptr.
            // NOTE: re-initing with new() is ok here: we know that
            // other.static_storage contains a pointer and we can overwrite
            // it with another pointer without calling the destructor first.
            ::new (other.static_storage) IFace *(nullptr);
            assert(other.m_p_iface == nullptr);
            other.m_pv_iface = nullptr;
        }
    }

public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    wrap(wrap &&other) noexcept
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
            delete p_iface;
        }
    }

public:
    ~wrap()
    {
        destroy();
    }

    wrap &operator=(wrap &&other) noexcept
    {
        // Handle self-assign.
        if (this == &other) {
            return *this;
        }

        // Handle invalid object.
        if (is_invalid()) {
            // No need to destroy, just move init
            // from other is sufficient.
            move_init_from(std::move(other));
            return *this;
        }

        // Handle different internal types.
        if (value_type_index() != other.value_type_index()) {
            destroy();
            move_init_from(std::move(other));
            return *this;
        }

        // The internal types are the same.
        const auto [p_iface0, pv_iface0, st0] = stype();
        const auto [p_iface1, pv_iface1, st1] = other.stype();

        // The storage flags must match, as they depend only
        // on the internal types.
        assert(st0 == st1);

        if (st0) {
            // For static storage, directly move assign the internal value.
            std::move(*pv_iface1).move_assign_value(pv_iface0->value_ptr(detail::vtag{}), detail::vtag{});
        } else {
            // For dynamic storage, delete the current value and
            // then shallow copy the interface pointers
            // from other.
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete p_iface0;

            // NOTE: no need to set m_p_iface to null, as
            // it should already be null.
            ::new (this->static_storage) IFace *(p_iface1);
            assert(this->m_p_iface == nullptr);
            this->m_pv_iface = pv_iface1;

            // Nullify the interface pointers in other, so that, on destruction,
            // we will be calling delete on a nullptr.
            ::new (other.static_storage) IFace *(nullptr);
            assert(other.m_p_iface == nullptr);
            other.m_pv_iface = nullptr;
        }

        return *this;
    }

    wrap &operator=(const wrap &other)
    {
        // Handle self-assign.
        if (this == &other) {
            return *this;
        }

        // Handle invalid object or different internal types.
        if (is_invalid() || value_type_index() != other.value_type_index()) {
            *this = wrap(other);
            return *this;
        }

        // The internal types are the same.
        const auto [p_iface0, pv_iface0, st0] = stype();
        const auto [p_iface1, pv_iface1, st1] = other.stype();

        // The storage flags must match, as they depend only
        // on the internal types.
        assert(st0 == st1);

        // Assign the internal value.
        pv_iface1->copy_assign_value(pv_iface0->value_ptr(detail::vtag{}), detail::vtag{});

        return *this;
    }

    // NOTE: this object is invalid if the storage type is dynamic and
    // it has been moved from. In such a case, the move operation
    // will have set the interface pointers to null.
    // The only valid operations on an invalid object are:
    // - destruction,
    // - revival via copy/move assignment.
    [[nodiscard]] bool is_invalid() const noexcept
    {
        return std::get<0>(stype()) == nullptr;
    }

    [[nodiscard]] std::type_index value_type_index() const noexcept
    {
        return std::get<1>(stype())->value_type_index(detail::vtag{});
    }

    const IFace *operator->() const noexcept
    {
        return std::get<0>(stype());
    }
    IFace *operator->() noexcept
    {
        return std::get<0>(stype());
    }

    const IFace &operator*() const noexcept
    {
        return *operator->();
    }
    IFace &operator*() noexcept
    {
        return *operator->();
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator const IFace *() const noexcept
    {
        return operator->();
    }
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator IFace *() noexcept
    {
        return operator->();
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator const IFace &() const noexcept
    {
        return operator*();
    }
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator IFace &() noexcept
    {
        return operator*();
    }
};

TANUKI_END_NAMESPACE

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

#undef TANUKI_ABI_TAG_ATTR
#undef TANUKI_NO_UNIQUE_ADDRESS

#endif
