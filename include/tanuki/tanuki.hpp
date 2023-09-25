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
#include <memory>
#include <new>
#include <stdexcept>
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
    [[nodiscard]] virtual std::pair<IFace *, value_iface *> copy_init_holder(void *, vtag) const = 0;
    [[nodiscard]] virtual std::pair<IFace *, value_iface *> move_init_holder(void *, vtag) && noexcept = 0;
    virtual void copy_assign_value(void *, vtag) const = 0;
    virtual void move_assign_value(void *, vtag) && noexcept = 0;
    virtual void swap_value(void *, vtag) noexcept = 0;
};

// NOTE: constrain value types to be non-cv qualified objects for the time being.
// References and cv-qualified objects might be useful as future extensions,
// but we must thread carefully as typeid() removes them.
template <typename T>
concept valid_value_type = std::is_object_v<T> && (!std::is_const_v<T>)&&(!std::is_volatile_v<T>)&&std::destructible<T>;

template <typename T, template <typename, typename...> typename IFaceT, typename... Args>
    requires valid_value_type<T>
struct holder final : public value_iface<IFaceT<void, Args...>>, public IFaceT<holder<T, IFaceT, Args...>, Args...> {
    TANUKI_NO_UNIQUE_ADDRESS T m_value;

    using value_type = T;

    // Make sure we don't end up accidentally copying/moving
    // this class.
    holder(const holder &) = delete;
    holder(holder &&) noexcept = delete;
    holder &operator=(const holder &) = delete;
    holder &operator=(holder &&) noexcept = delete;

    ~holder() final = default;

    // NOTE: special-casing to avoid the single-argument ctor
    // potentially competing with the copy/move ctors.
    template <typename U>
    explicit holder(U &&x)
        requires(!std::same_as<holder, std::remove_cvref_t<U>>) && std::constructible_from<T, U &&>
        : m_value(std::forward<U>(x))
    {
    }
    template <typename... U>
    explicit holder(U &&...x)
        requires(sizeof...(U) != 1u) && std::constructible_from<T, U &&...>
        : m_value(std::forward<U>(x)...)
    {
    }

    // NOTE: mark everything else as private so that it is going to be
    // unreachable from the interface implementation.
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
    [[nodiscard]] std::pair<IFaceT<void, Args...> *, value_iface<IFaceT<void, Args...>> *> clone(vtag) const final
    {
        if constexpr (std::copy_constructible<T>) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *ret = new holder(m_value);
            return {ret, ret};
        } else {
            throw std::invalid_argument("Attempting to copy-construct a non-copyable value type");
        }
    }
    // Copy-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    [[nodiscard]] std::pair<IFaceT<void, Args...> *, value_iface<IFaceT<void, Args...>> *>
    copy_init_holder(void *ptr, vtag) const final
    {
        if constexpr (std::copy_constructible<T>) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *ret = ::new (ptr) holder(m_value);
            return {ret, ret};
        } else {
            throw std::invalid_argument("Attempting to copy-construct a non-copyable value type");
        }
    }
    // Move-init a new holder into the storage beginning at ptr.
    // Then cast the result to the two bases and return.
    [[nodiscard]] std::pair<IFaceT<void, Args...> *, value_iface<IFaceT<void, Args...>> *>
    // NOLINTNEXTLINE(bugprone-exception-escape)
    move_init_holder(void *ptr, vtag) && noexcept final
    {
        if constexpr (std::move_constructible<T>) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto *ret = ::new (ptr) holder(std::move(m_value));
            return {ret, ret};
        } else {
            throw std::invalid_argument("Attempting to move-construct a non-movable value type");
        }
    }
    // Copy-assign m_value into the object of type T assumed to be stored in ptr.
    void copy_assign_value(void *ptr, vtag) const final
    {
        if constexpr (std::is_copy_assignable_v<T>) {
            // NOTE: I don't think it is necessary to invoke launder here,
            // as ptr is always supposed to come from an invocation of value_ptr(),
            // which just does a static cast to void *. Since we are assuming that
            // copy_assign_value() is called only when assigning holders containing
            // the same T, the conversion chain should boil down to T * -> void * -> T *, which
            // does not require laundering.
            *static_cast<T *>(ptr) = m_value;
        } else {
            throw std::invalid_argument("Attempting to copy-assign a non-copyable value type");
        }
    }
    // Move-assign m_value into the object of type T assumed to be stored in ptr.
    // NOLINTNEXTLINE(bugprone-exception-escape)
    void move_assign_value(void *ptr, vtag) && noexcept final
    {
        if constexpr (std::is_move_assignable_v<T>) {
            *static_cast<T *>(ptr) = std::move(m_value);
        } else {
            throw std::invalid_argument("Attempting to move-assign a non-movable value type");
        }
    }
    // Swap m_value with the object of type T assumed to be stored in ptr.
    // NOLINTNEXTLINE(bugprone-exception-escape)
    void swap_value(void *ptr, vtag) noexcept final
    {
        if constexpr (std::swappable<T>) {
            using std::swap;
            swap(m_value, *static_cast<T *>(ptr));
        } else {
            throw std::invalid_argument("Attempting to swap a non-swappable value type");
        }
    }
};

// Implementation of basic storage for the wrap class.
template <typename IFace, std::size_t StaticStorageSize, std::size_t StaticStorageAlignment>
struct wrap_storage {
    // NOTE: static storage optimisation enabled. The m_p_iface member is used as a flag:
    // if it is null, then the current storage type is dynamic and the interface pointer
    // (which may be null for the invalid state) is stored in static_storage. If m_p_iface
    // is *not* null, then the current storage type is static and both m_p_iface and m_pv_iface
    // point somewhere in static_storage.
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

// NOTE: this is used to check that a config instance
// is a specialisation from the primary config template.
struct config_base {
};

} // namespace detail

// Configuration settings for the wrap class.
// NOTE: the DefaultValueType is subject to the constraints
// for valid value types.
template <typename DefaultValueType = void>
    requires std::same_as<DefaultValueType, void> || detail::valid_value_type<DefaultValueType>
struct config final : detail::config_base {
    using default_value_type = DefaultValueType;

    // Size of the static storage.
    std::size_t static_size = 48;
    // Alignment of the static storage.
    std::size_t static_alignment = alignof(std::max_align_t);
    // Default constructor initialises to the invalid state.
    bool definit_invalid = false;
    // Provide pointer interface.
    bool pointer_interface = true;
    // Explicit conversion operators to interface reference/pointer.
    bool explicit_iface_conversion = true;
    // Enable copy construction/assignment.
    bool copyable = true;
    // Enable move construction/assignment.
    bool movable = true;
    // Enable swap.
    bool swappable = true;
};

// Default configuration for the wrap class.
inline constexpr auto default_config = config{};

namespace detail
{

template <std::size_t N>
concept power_of_two = (N > 0u) && ((N & (N - 1u)) == 0u);

} // namespace detail

// Concept for checking that Cfg is a valid config instance.
template <auto Cfg>
concept valid_config =
    // This checks that decltype(Cfg) is a specialisation from the primary config template.
    std::derived_from<std::remove_const_t<decltype(Cfg)>, detail::config_base> &&
    // The static alignment value must be a power of 2.
    detail::power_of_two<Cfg.static_alignment>;

// Default implementation of value type checking.
template <typename, template <typename, typename...> typename, typename...>
inline constexpr bool is_wrappable = true;

// Default reference interface implementation.
template <typename, template <typename, typename...> typename, typename...>
struct ref_iface {
};

namespace detail
{

// Meta-programming to establish a holder value type for a type T.
// This is used in the generic ctor of wrap.
template <typename T>
struct value_t_from_impl {
    // By default, the value type is T itself.
    using type = T;
};

template <typename R, typename... Args>
struct value_t_from_impl<R(Args...)> {
    // For function types, let it decay so that
    // the stored value is a function pointer.
    using type = std::decay_t<R(Args...)>;
};

template <typename T>
using value_t_from = typename value_t_from_impl<std::remove_cvref_t<T>>::type;

} // namespace detail

#define TANUKI_REF_IFACE_MEMFUN(name)                                                                                  \
    template <typename JustWrap = Wrap, typename... MemFunArgs>                                                        \
    auto name(MemFunArgs &&...args) & noexcept(                                                                        \
        noexcept(get_iface_ptr(*static_cast<JustWrap *>(this))->name(std::forward<MemFunArgs>(args)...)))              \
        -> decltype(get_iface_ptr(*static_cast<JustWrap *>(this))->name(std::forward<MemFunArgs>(args)...))            \
    {                                                                                                                  \
        return get_iface_ptr(*static_cast<Wrap *>(this))->name(std::forward<MemFunArgs>(args)...);                     \
    }                                                                                                                  \
    template <typename JustWrap = Wrap, typename... MemFunArgs>                                                        \
    auto name(MemFunArgs &&...args) const & noexcept(                                                                  \
        noexcept(get_iface_ptr(*static_cast<const JustWrap *>(this))->name(std::forward<MemFunArgs>(args)...)))        \
        -> decltype(get_iface_ptr(*static_cast<const JustWrap *>(this))->name(std::forward<MemFunArgs>(args)...))      \
    {                                                                                                                  \
        return get_iface_ptr(*static_cast<const Wrap *>(this))->name(std::forward<MemFunArgs>(args)...);               \
    }                                                                                                                  \
    template <typename JustWrap = Wrap, typename... MemFunArgs>                                                        \
    auto name(MemFunArgs &&...args) && noexcept(                                                                       \
        noexcept(std::move(*get_iface_ptr(*static_cast<JustWrap *>(this))).name(std::forward<MemFunArgs>(args)...)))   \
        -> decltype(std::move(*get_iface_ptr(*static_cast<JustWrap *>(this))).name(std::forward<MemFunArgs>(args)...)) \
    {                                                                                                                  \
        return std::move(*get_iface_ptr(*static_cast<Wrap *>(this))).name(std::forward<MemFunArgs>(args)...);          \
    }

// Fwd declaration.
template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
    requires std::is_polymorphic_v<IFaceT<void, Args...>> && std::has_virtual_destructor_v<IFaceT<void, Args...>>
             && valid_config<Cfg>
class wrap;

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
    requires(Cfg.swappable)
void swap(wrap<IFaceT, Cfg, Args...> &, wrap<IFaceT, Cfg, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
[[nodiscard]] bool is_invalid(const wrap<IFaceT, Cfg, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
[[nodiscard]] std::type_index value_type_index(const wrap<IFaceT, Cfg, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
[[nodiscard]] const IFaceT<void, Args...> *get_iface_ptr(const wrap<IFaceT, Cfg, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
[[nodiscard]] IFaceT<void, Args...> *get_iface_ptr(wrap<IFaceT, Cfg, Args...> &) noexcept;

template <template <typename, typename...> typename IFaceT, auto Cfg = default_config, typename... Args>
    requires std::is_polymorphic_v<IFaceT<void, Args...>> && std::has_virtual_destructor_v<IFaceT<void, Args...>>
                 && valid_config<Cfg>
class wrap : private detail::wrap_storage<IFaceT<void, Args...>, Cfg.static_size, Cfg.static_alignment>,
             public ref_iface<wrap<IFaceT, Cfg, Args...>, IFaceT, Args...>
{
    // Aliases for the two interfaces.
    using iface_t = IFaceT<void, Args...>;
    using value_iface_t = detail::value_iface<iface_t>;

    // Friendship with the free functions.
    friend void swap<IFaceT, Cfg, Args...>(wrap &, wrap &) noexcept;
    friend bool is_invalid<IFaceT, Cfg, Args...>(const wrap &) noexcept;
    friend std::type_index value_type_index<IFaceT, Cfg, Args...>(const wrap &) noexcept;
    friend const iface_t *get_iface_ptr(const wrap<IFaceT, Cfg, Args...> &) noexcept;
    friend iface_t *get_iface_ptr(wrap<IFaceT, Cfg, Args...> &) noexcept;

    // The default value type.
    using default_value_t = typename decltype(Cfg)::default_value_type;

    // Shortcut for the holder type corresponding to the value type T.
    template <typename T>
    using holder_t = detail::holder<T, IFaceT, Args...>;

    // Helpers to fetch the interface pointers and the storage type when
    // static storage is enabled.
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

    // Implementation of generic construction. This will constrcut
    // a holder with value type T using the construction argument(s) x.
    template <typename T, typename... U>
        requires
        // These checks are for verifying that:
        // - iface_t is a base of the interface implementation, and
        // - all interface requirements have been implemented, and
        // - we can construct the value type from the variadic args, and
        // - the value type T satisfies the conditions to be stored in a holder.
        std::constructible_from<holder_t<T>, U &&...> && std::derived_from<holder_t<T>, iface_t> &&
        // Alignment checks: if we are going to use dynamic storage, then no checks are needed
        // as new() takes care of proper alignment; otherwise, we need to ensure that the static
        // storage is sufficiently aligned.
        (sizeof(holder_t<T>) > Cfg.static_size || alignof(holder_t<T>) <= Cfg.static_alignment) &&
        // Value type checks.
        std::same_as<const bool, decltype(is_wrappable<T, IFaceT, Args...>)>
        && is_wrappable<T, IFaceT, Args...>
        void ctor_impl(U &&...x)
    {
        if constexpr (Cfg.static_size == 0u) {
            // Static storage disabled.
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto d_ptr = new holder_t<T>(std::forward<U>(x)...);
            this->m_p_iface = d_ptr;
            this->m_pv_iface = d_ptr;
        } else {
            if constexpr (sizeof(holder_t<T>) <= Cfg.static_size) {
                // Static storage.
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                auto *d_ptr = ::new (this->static_storage) holder_t<T>(std::forward<U>(x)...);
                this->m_p_iface = d_ptr;
                this->m_pv_iface = d_ptr;
            } else {
                // Dynamic storage.
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                auto d_ptr = new holder_t<T>(std::forward<U>(x)...);
                ::new (this->static_storage) iface_t *(d_ptr);
                this->m_p_iface = nullptr;
                this->m_pv_iface = d_ptr;
            }
        }
    }

public:
    wrap()
        requires(Cfg.definit_invalid)
                || (
                    // A default value type must have been specified
                    // in the configuration.
                    (!std::same_as<void, default_value_t>) &&
                    // We must be able to value-init a default_value_t
                    // into the holder.
                    requires(wrap &w) { w.ctor_impl<default_value_t>(); })
    {
        if constexpr (Cfg.definit_invalid) {
            if constexpr (Cfg.static_size != 0u) {
                // Init the interface pointer to null.
                ::new (this->static_storage) iface_t *(nullptr);
            }

            // NOTE: if static storage is enabled, this will indicate
            // that dynamic storage is being employed. Otherwise, this will
            // set the interface pointer to null.
            this->m_p_iface = nullptr;
            this->m_pv_iface = nullptr;
        } else {
            ctor_impl<default_value_t>();
        }
    }

    // Generic ctor from a wrappable value.
    template <typename T>
        requires
        // Must not compete with copy/move.
        (!std::same_as<std::remove_cvref_t<T>, wrap>) &&
        // We must be able to construct a holder from x.
        requires(wrap &w, T &&arg) { w.ctor_impl<detail::value_t_from<T &&>>(std::forward<T>(arg)); }
        // NOLINTNEXTLINE(bugprone-forwarding-reference-overload,cppcoreguidelines-pro-type-member-init,hicpp-member-init)
        explicit wrap(T &&x)
    {
        ctor_impl<detail::value_t_from<T &&>>(std::forward<T>(x));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    wrap(const wrap &other)
        requires(Cfg.copyable)
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
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    void move_init_from(wrap &&other) noexcept
    {
        if constexpr (Cfg.static_size == 0u) {
            // Static storage disabled.
            // Shallow copy the pointers.
            this->m_p_iface = other.m_p_iface;
            this->m_pv_iface = other.m_pv_iface;

            // Invalidate other.
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

                // Invalidate other.
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
        requires(Cfg.movable)
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
        requires(Cfg.movable)
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

            // Invalidate other.
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

                // Invalidate other.
                ::new (other.static_storage) iface_t *(nullptr);
                assert(other.m_p_iface == nullptr);
                other.m_pv_iface = nullptr;
            }
        }

        return *this;
    }

    wrap &operator=(const wrap &other)
        requires(Cfg.copyable)
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

    const iface_t *operator->() const noexcept
        requires(Cfg.pointer_interface)
    {
        return get_iface_ptr(*this);
    }
    iface_t *operator->() noexcept
        requires(Cfg.pointer_interface)
    {
        return get_iface_ptr(*this);
    }

    const iface_t &operator*() const noexcept
        requires(Cfg.pointer_interface)
    {
        return *get_iface_ptr(*this);
    }
    iface_t &operator*() noexcept
        requires(Cfg.pointer_interface)
    {
        return *get_iface_ptr(*this);
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator const iface_t *() const noexcept
        requires(Cfg.pointer_interface)
    {
        return get_iface_ptr(*this);
    }
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator iface_t *() noexcept
        requires(Cfg.pointer_interface)
    {
        return get_iface_ptr(*this);
    }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator const iface_t &() const noexcept
        requires(Cfg.pointer_interface)
    {
        return *get_iface_ptr(*this);
    }
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    explicit(Cfg.explicit_iface_conversion) operator iface_t &() noexcept
        requires(Cfg.pointer_interface)
    {
        return *get_iface_ptr(*this);
    }
};

// NOTE: w is invalid if the storage type is dynamic and
// it has been moved from. In such a case, the move operation
// will have set the interface pointers to null.
// The only valid operations on an invalid object are:
// - destruction,
// - revival via copy/move assignment.
template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
bool is_invalid(const wrap<IFaceT, Cfg, Args...> &w) noexcept
{
    if constexpr (Cfg.static_size == 0u) {
        assert((w.m_p_iface == nullptr) == (w.m_pv_iface == nullptr));
        return w.m_p_iface == nullptr;
    } else {
        return std::get<0>(w.stype()) == nullptr;
    }
}

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
std::type_index value_type_index(const wrap<IFaceT, Cfg, Args...> &w) noexcept
{
    if constexpr (Cfg.static_size == 0u) {
        return w.m_pv_iface->value_type_index(detail::vtag{});
    } else {
        return std::get<1>(w.stype())->value_type_index(detail::vtag{});
    }
}

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
    requires(Cfg.swappable)
void swap(wrap<IFaceT, Cfg, Args...> &w1, wrap<IFaceT, Cfg, Args...> &w2) noexcept
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

            using iface_t = typename wrap<IFaceT, Cfg, Args...>::iface_t;

            std::swap(*std::launder(reinterpret_cast<iface_t **>(w1.static_storage)),
                      *std::launder(reinterpret_cast<iface_t **>(w2.static_storage)));
            std::swap(w1.m_pv_iface, w2.m_pv_iface);
        }
    }
}

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
[[nodiscard]] const IFaceT<void, Args...> *get_iface_ptr(const wrap<IFaceT, Cfg, Args...> &w) noexcept
{
    if constexpr (Cfg.static_size == 0u) {
        return w.m_p_iface;
    } else {
        return std::get<0>(w.stype());
    }
}

template <template <typename, typename...> typename IFaceT, auto Cfg, typename... Args>
[[nodiscard]] IFaceT<void, Args...> *get_iface_ptr(wrap<IFaceT, Cfg, Args...> &w) noexcept
{
    if constexpr (Cfg.static_size == 0u) {
        return w.m_p_iface;
    } else {
        return std::get<0>(w.stype());
    }
}

TANUKI_END_NAMESPACE

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

#undef TANUKI_ABI_TAG_ATTR
#undef TANUKI_NO_UNIQUE_ADDRESS

#endif
