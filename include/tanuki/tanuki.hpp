// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef TANUKI_TANUKI_HPP
#define TANUKI_TANUKI_HPP

#include <algorithm>
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
    virtual void swap_value(void *, vtag) noexcept = 0;
};

template <typename T, template <typename, typename...> typename IFaceT, typename... Args>
struct holder final : public value_iface<IFaceT<void, Args...>>, public IFaceT<holder<T, IFaceT, Args...>, Args...> {
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
    // unreachable from the interface implementation.
private:
    using iface_t = IFaceT<void, Args...>;

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
    [[nodiscard]] std::pair<iface_t *, value_iface<iface_t> *> clone(vtag) const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = new holder(m_value);
        return {ret, ret};
    }
    // Copy-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    std::pair<iface_t *, value_iface<iface_t> *> copy_init_holder(void *ptr, vtag) const final
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *ret = ::new (ptr) holder(m_value);
        return {ret, ret};
    }
    // Move-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    std::pair<iface_t *, value_iface<iface_t> *> move_init_holder(void *ptr, vtag) && noexcept final
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
        *static_cast<T *>(ptr) = std::move(m_value);
    }
    // Swap m_value with the object of type T assumed to be stored in ptr.
    void swap_value(void *ptr, vtag) noexcept final
    {
        using std::swap;
        swap(m_value, *static_cast<T *>(ptr));
    }
};

// Implementation of basic storage for the wrap class.
template <typename IFace, std::size_t StaticStorageSize, std::size_t StaticStorageAlignment>
struct wrap_storage {
    static_assert(StaticStorageSize > 0u);

    // NOTE: the static storage is used to store an IFace * in dynamic
    // storage mode, thus it has minimum size and alignment requirements.
    alignas(std::max(StaticStorageAlignment,
                     alignof(IFace *))) std::byte static_storage[std::max(StaticStorageSize, sizeof(IFace *))];
    IFace *m_p_iface;
    value_iface<IFace> *m_pv_iface;
};

template <typename IFace, std::size_t StaticStorageAlignment>
struct wrap_storage<IFace, 0, StaticStorageAlignment> {
    IFace *m_p_iface;
    value_iface<IFace> *m_pv_iface;
};

} // namespace detail

// Configuration structure for the wrap class.
struct config {
    std::size_t static_size = 48;
    std::size_t static_alignment = alignof(std::max_align_t);
    bool explicit_iface_conversion = true;
};

inline constexpr auto default_config = config{};

template <typename, template <typename, typename...> typename>
inline constexpr bool is_wrappable = true;

template <typename>
struct no_ref_iface {
};

#define TANUKI_MAKE_REF_IFACE_MEMFUN(name)                                                                             \
    template <typename JustWrap = Wrap, typename... Args>                                                              \
    auto name(Args &&...args) & noexcept(                                                                              \
        noexcept(static_cast<JustWrap *>(this)->get_iface_ptr()->name(std::forward<Args>(args)...)))                   \
        ->decltype(static_cast<JustWrap *>(this)->get_iface_ptr()->name(std::forward<Args>(args)...))                  \
    {                                                                                                                  \
        return static_cast<Wrap *>(this)->get_iface_ptr()->name(std::forward<Args>(args)...);                          \
    }                                                                                                                  \
    template <typename JustWrap = Wrap, typename... Args>                                                              \
    auto name(Args &&...args) const & noexcept(                                                                        \
        noexcept(static_cast<const JustWrap *>(this)->get_iface_ptr()->name(std::forward<Args>(args)...)))             \
        ->decltype(static_cast<const JustWrap *>(this)->get_iface_ptr()->name(std::forward<Args>(args)...))            \
    {                                                                                                                  \
        return static_cast<const Wrap *>(this)->get_iface_ptr()->name(std::forward<Args>(args)...);                    \
    }                                                                                                                  \
    template <typename JustWrap = Wrap, typename... Args>                                                              \
    auto name(Args &&...args) && noexcept(                                                                             \
        noexcept(std::move(*static_cast<JustWrap *>(this)->get_iface_ptr()).name(std::forward<Args>(args)...)))        \
        ->decltype(std::move(*static_cast<JustWrap *>(this)->get_iface_ptr()).name(std::forward<Args>(args)...))       \
    {                                                                                                                  \
        return std::move(*static_cast<Wrap *>(this)->get_iface_ptr()).name(std::forward<Args>(args)...);               \
    }

template <std::size_t N>
concept power_of_two = (N > 0u) && ((N & (N - 1u)) == 0u);

// Fwd declaration.
template <template <typename, typename...> typename IFaceT, config Cfg, template <typename> typename, typename... Args>
    requires std::is_polymorphic_v<IFaceT<void, Args...>> && std::has_virtual_destructor_v<IFaceT<void, Args...>>
             && power_of_two<Cfg.static_alignment>
class wrap;

template <template <typename, typename...> typename IFaceT, config Cfg, template <typename> typename RefIFace,
          typename... Args>
void swap(wrap<IFaceT, Cfg, RefIFace, Args...> &, wrap<IFaceT, Cfg, RefIFace, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, config Cfg, template <typename> typename RefIFace,
          typename... Args>
[[nodiscard]] bool is_invalid(const wrap<IFaceT, Cfg, RefIFace, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, config Cfg, template <typename> typename RefIFace,
          typename... Args>
[[nodiscard]] std::type_index value_type_index(const wrap<IFaceT, Cfg, RefIFace, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, config Cfg = default_config,
          template <typename> typename RefIFace = no_ref_iface, typename... Args>
    requires std::is_polymorphic_v<IFaceT<void, Args...>> && std::has_virtual_destructor_v<IFaceT<void, Args...>> &&
                 // The static alignment value must be a power of 2.
                 power_of_two<Cfg.static_alignment>
class wrap : private detail::wrap_storage<IFaceT<void, Args...>, Cfg.static_size, Cfg.static_alignment>,
             public RefIFace<wrap<IFaceT, Cfg, RefIFace, Args...>>
{
    friend void swap<IFaceT, Cfg, RefIFace, Args...>(wrap &, wrap &) noexcept;
    friend bool is_invalid<IFaceT, Cfg, RefIFace, Args...>(const wrap &) noexcept;
    friend std::type_index value_type_index<IFaceT, Cfg, RefIFace, Args...>(const wrap &) noexcept;

    using iface_t = IFaceT<void, Args...>;

    using value_iface_t = detail::value_iface<iface_t>;

    using ref_iface_t = RefIFace<wrap<IFaceT, Cfg, RefIFace, Args...>>;
    friend ref_iface_t;
    static constexpr bool has_ref_iface
        = !std::is_same_v<ref_iface_t, no_ref_iface<wrap<IFaceT, Cfg, no_ref_iface, Args...>>>;

    // Helpers to fetch the interface pointers and the storage type.
    std::tuple<const iface_t *, const value_iface_t *, bool> stype() const noexcept
        requires(Cfg.static_size > 0u)
    {
        if (this->m_p_iface == nullptr) {
            // Dynamic storage.
            const auto *ret = *std::launder(reinterpret_cast<iface_t *const *>(this->static_storage));
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
    std::tuple<iface_t *, value_iface_t *, bool> stype() noexcept
        requires(Cfg.static_size > 0u)
    {
        if (this->m_p_iface == nullptr) {
            auto *ret = *std::launder(reinterpret_cast<iface_t **>(this->static_storage));
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
    using make_holder_t = detail::holder<std::remove_cvref_t<T>, IFaceT, Args...>;

public:
    wrap() = delete;

    // Generic ctor.
    template <typename T>
        requires
        // Must not compete with copy/move.
        (!std::same_as<std::remove_cvref_t<T>, wrap>) &&
        // T must be destructible.
        std::destructible<std::remove_cvref_t<T>> &&
        // These checks are for verifying that iface_t is a base
        // of the interface implementation and that all
        // interface requirements have been implemented.
        std::derived_from<make_holder_t<T &&>, iface_t> && std::constructible_from<make_holder_t<T &&>, T &&> &&
        // Alignment checks.
        (sizeof(make_holder_t<T &&>) > Cfg.static_size || alignof(make_holder_t<T &&>) <= Cfg.static_alignment) &&
        // Type checks.
        std::same_as<const bool, decltype(is_wrappable<std::remove_cvref_t<T>, IFaceT>)>
        && is_wrappable<std::remove_cvref_t<T>, IFaceT>
        // NOLINTNEXTLINE(bugprone-forwarding-reference-overload,cppcoreguidelines-pro-type-member-init,hicpp-member-init)
        explicit wrap(T &&x)
    {
        using holder_t = make_holder_t<T &&>;

        if constexpr (Cfg.static_size == 0u) {
            // Static storage disabled.
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto d_ptr = new holder_t(std::forward<T>(x));
            this->m_p_iface = d_ptr;
            this->m_pv_iface = d_ptr;
        } else {
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
                ::new (this->static_storage) iface_t *(d_ptr);
                this->m_p_iface = nullptr;
                this->m_pv_iface = d_ptr;
            }
        }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    wrap(const wrap &other)
    {
        if constexpr (Cfg.static_size == 0u) {
            // Static storage disabled.
            std::tie(this->m_p_iface, this->m_pv_iface) = other.m_pv_iface->clone(detail::vtag{});
        } else {
            const auto [_, pv_iface, st] = other.stype();

            if (st) {
                // Other has static storage.
                auto [new_p_iface, new_pv_iface] = pv_iface->copy_init_holder(this->static_storage, detail::vtag{});
                this->m_p_iface = new_p_iface;
                this->m_pv_iface = new_pv_iface;
            } else {
                // Other has dynamic storage.
                auto [new_p_iface, new_pv_iface] = pv_iface->clone(detail::vtag{});
                ::new (this->static_storage) iface_t *(new_p_iface);
                this->m_p_iface = nullptr;
                this->m_pv_iface = new_pv_iface;
            }
        }
    }

private:
    void move_init_from(wrap &&other) noexcept
    {
        if constexpr (Cfg.static_size == 0u) {
            // Static storage disabled.
            // Shallow copy the pointers.
            this->m_p_iface = other.m_p_iface;
            this->m_pv_iface = other.m_pv_iface;

            // Nullify the interface pointers in other, so that, on destruction,
            // we will be calling delete on a nullptr.
            other.m_p_iface = nullptr;
            other.m_pv_iface = nullptr;
        } else {
            const auto [p_iface, pv_iface, st] = other.stype();

            if (st) {
                // Other has static storage.
                auto [new_p_iface, new_pv_iface]
                    = std::move(*pv_iface).move_init_holder(this->static_storage, detail::vtag{});
                this->m_p_iface = new_p_iface;
                this->m_pv_iface = new_pv_iface;
            } else {
                // Other has dynamic storage.
                ::new (this->static_storage) iface_t *(p_iface);
                this->m_p_iface = nullptr;
                this->m_pv_iface = pv_iface;

                // Nullify the interface pointers in other, so that, on destruction,
                // we will be calling delete on a nullptr.
                // NOTE: re-initing with new() is ok here: we know that
                // other.static_storage contains a pointer and we can overwrite
                // it with another pointer without calling the destructor first.
                ::new (other.static_storage) iface_t *(nullptr);
                assert(other.m_p_iface == nullptr);
                other.m_pv_iface = nullptr;
            }
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
        if constexpr (Cfg.static_size == 0u) {
            // NOTE: if one pointer is null, the other one must be as well.
            assert((this->m_p_iface == nullptr) == (this->m_pv_iface == nullptr));

            delete this->m_p_iface;
        } else {
            const auto [p_iface, _, st] = stype();

            if (st) {
                p_iface->~iface_t();
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                delete p_iface;
            }
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
        if (is_invalid(*this)) {
            // No need to destroy, just move init
            // from other is sufficient.
            move_init_from(std::move(other));
            return *this;
        }

        // Handle different internal types.
        if (value_type_index(*this) != value_type_index(other)) {
            destroy();
            move_init_from(std::move(other));
            return *this;
        }

        // The internal types are the same.
        if constexpr (Cfg.static_size == 0u) {
            // For dynamic storage, delete the current value and
            // then shallow copy the interface pointers
            // from other.
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete this->m_p_iface;

            this->m_p_iface = other.m_p_iface;
            this->m_pv_iface = other.m_pv_iface;

            // Nullify the interface pointers in other, so that, on destruction,
            // we will be calling delete on a nullptr.
            other.m_p_iface = nullptr;
            other.m_pv_iface = nullptr;
        } else {
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
                ::new (this->static_storage) iface_t *(p_iface1);
                assert(this->m_p_iface == nullptr);
                this->m_pv_iface = pv_iface1;

                // Nullify the interface pointers in other, so that, on destruction,
                // we will be calling delete on a nullptr.
                ::new (other.static_storage) iface_t *(nullptr);
                assert(other.m_p_iface == nullptr);
                other.m_pv_iface = nullptr;
            }
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
        if (is_invalid(*this) || value_type_index(*this) != value_type_index(other)) {
            *this = wrap(other);
            return *this;
        }

        // The internal types are the same.
        if constexpr (Cfg.static_size == 0u) {
            // Assign the internal value.
            other.m_pv_iface->copy_assign_value(this->m_pv_iface->value_ptr(detail::vtag{}), detail::vtag{});
        } else {
            const auto [p_iface0, pv_iface0, st0] = stype();
            const auto [p_iface1, pv_iface1, st1] = other.stype();

            // The storage flags must match, as they depend only
            // on the internal types.
            assert(st0 == st1);

            // Assign the internal value.
            pv_iface1->copy_assign_value(pv_iface0->value_ptr(detail::vtag{}), detail::vtag{});
        }

        return *this;
    }

private:
    const iface_t *get_iface_ptr() const noexcept
    {
        if constexpr (Cfg.static_size == 0u) {
            return this->m_p_iface;
        } else {
            return std::get<0>(stype());
        }
    }
    iface_t *get_iface_ptr() noexcept
    {
        if constexpr (Cfg.static_size == 0u) {
            return this->m_p_iface;
        } else {
            return std::get<0>(stype());
        }
    }

public:
    const iface_t *operator->() const noexcept
        requires(!has_ref_iface)
    {
        return get_iface_ptr();
    }
    iface_t *operator->() noexcept
        requires(!has_ref_iface)
    {
        return get_iface_ptr();
    }

    const iface_t &operator*() const noexcept
        requires(!has_ref_iface)
    {
        return *get_iface_ptr();
    }
    iface_t &operator*() noexcept
        requires(!has_ref_iface)
    {
        return *get_iface_ptr();
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator const iface_t *() const noexcept
        requires(!has_ref_iface)
    {
        return get_iface_ptr();
    }
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator iface_t *() noexcept
        requires(!has_ref_iface)
    {
        return get_iface_ptr();
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator const iface_t &() const noexcept
        requires(!has_ref_iface)
    {
        return *get_iface_ptr();
    }
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator iface_t &() noexcept
        requires(!has_ref_iface)
    {
        return *get_iface_ptr();
    }
};

// NOTE: w is invalid if the storage type is dynamic and
// it has been moved from. In such a case, the move operation
// will have set the interface pointers to null.
// The only valid operations on an invalid object are:
// - destruction,
// - revival via copy/move assignment.
template <template <typename, typename...> typename IFaceT, config Cfg, template <typename> typename RefIFace,
          typename... Args>
bool is_invalid(const wrap<IFaceT, Cfg, RefIFace, Args...> &w) noexcept
{
    if constexpr (Cfg.static_size == 0u) {
        return w.m_p_iface == nullptr;
    } else {
        return std::get<0>(w.stype()) == nullptr;
    }
}

template <template <typename, typename...> typename IFaceT, config Cfg, template <typename> typename RefIFace,
          typename... Args>
std::type_index value_type_index(const wrap<IFaceT, Cfg, RefIFace, Args...> &w) noexcept
{
    if constexpr (Cfg.static_size == 0u) {
        return w.m_pv_iface->value_type_index(detail::vtag{});
    } else {
        return std::get<1>(w.stype())->value_type_index(detail::vtag{});
    }
}

template <template <typename, typename...> typename IFaceT, config Cfg, template <typename> typename RefIFace,
          typename... Args>
void swap(wrap<IFaceT, Cfg, RefIFace, Args...> &w1, wrap<IFaceT, Cfg, RefIFace, Args...> &w2) noexcept
{
    // Handle self swap.
    if (&w1 == &w2) {
        return;
    }

    // Handle different types with the canonical swap() implementation.
    if (value_type_index(w1) != value_type_index(w2)) {
        auto temp(std::move(w1));
        w1 = std::move(w2);
        w2 = std::move(temp);
        return;
    }

    // The types are the same.
    if constexpr (Cfg.static_size == 0u) {
        // For dynamic storage, swap the pointers.
        std::swap(w1.m_p_iface, w2.m_p_iface);
        std::swap(w1.m_pv_iface, w2.m_pv_iface);
    } else {
        const auto [p_iface1, pv_iface1, st1] = w1.stype();
        const auto [p_iface2, pv_iface2, st2] = w2.stype();

        // The storage flags must match, as they depend only
        // on the internal types.
        assert(st1 == st2);

        if (st1) {
            // For static storage, directly swap the internal values.
            pv_iface2->swap_value(pv_iface1->value_ptr(detail::vtag{}), detail::vtag{});
        } else {
            // For dynamic storage, swap the pointers.
            assert(w1.m_p_iface == nullptr);
            assert(w2.m_p_iface == nullptr);

            using iface_t = typename wrap<IFaceT, Cfg, RefIFace, Args...>::iface_t;

            std::swap(*std::launder(reinterpret_cast<iface_t **>(w1.static_storage)),
                      *std::launder(reinterpret_cast<iface_t **>(w2.static_storage)));
            std::swap(w1.m_pv_iface, w2.m_pv_iface);
        }
    }
}

TANUKI_END_NAMESPACE

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

#undef TANUKI_ABI_TAG_ATTR
#undef TANUKI_NO_UNIQUE_ADDRESS

#endif
