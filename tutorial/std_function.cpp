#include <cassert>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

// Machinery to detect std::function.
template <typename>
struct is_any_std_func : std::false_type {
};

template <typename R, typename... Args>
struct is_any_std_func<std::function<R(Args...)>> : std::true_type {
};

// Machinery to detect callable instances.
template <typename>
struct is_any_callable : std::false_type {
};

template <typename Base, typename Holder, typename T, typename R, typename... Args>
struct callable_iface_impl {
};

template <typename R, typename... Args>
struct callable_iface {
    virtual ~callable_iface() = default;
    virtual R operator()(Args... args) const = 0;
    virtual explicit operator bool() const noexcept = 0;

    template <typename Base, typename Holder, typename T>
    using impl = callable_iface_impl<Base, Holder, T, R, Args...>;
};

template <typename Base, typename Holder, typename T, typename R, typename... Args>
    requires std::is_invocable_r_v<R, const tanuki::unwrap_cvref_t<T> &, Args...> && std::copy_constructible<T>
struct callable_iface_impl<Base, Holder, T, R, Args...> : public Base {
    R operator()(Args... args) const final
    {
        using unrefT = tanuki::unwrap_cvref_t<T>;

        // Check for null function pointer.
        if constexpr (std::is_pointer_v<unrefT> || std::is_member_pointer_v<unrefT>) {
            if (getval<Holder>(this) == nullptr) {
                throw std::bad_function_call{};
            }
        }

        if constexpr (std::is_same_v<R, void>) {
            static_cast<void>(std::invoke(getval<Holder>(this), std::forward<Args>(args)...));
        } else {
            return std::invoke(getval<Holder>(this), std::forward<Args>(args)...);
        }
    }

    explicit operator bool() const noexcept final
    {
        using unrefT = tanuki::unwrap_cvref_t<T>;

        if constexpr (std::is_pointer_v<unrefT> || std::is_member_pointer_v<unrefT>) {
            return getval<Holder>(this) != nullptr;
        } else if constexpr (is_any_callable<unrefT>::value || is_any_std_func<unrefT>::value) {
            return static_cast<bool>(getval<Holder>(this));
        } else {
            return true;
        }
    }
};

// Reference interface.
template <typename R, typename... Args>
struct callable_ref_iface {
    template <typename Wrap>
    struct impl {
        using result_type = R;

        template <typename JustWrap = Wrap, typename... FArgs>
        auto operator()(FArgs &&...fargs) const
            -> decltype(iface_ptr(*static_cast<const JustWrap *>(this))->operator()(std::forward<FArgs>(fargs)...))
        {
            // NOTE: a wrap in the invalid state is considered empty.
            if (is_invalid(*static_cast<const Wrap *>(this))) {
                throw std::bad_function_call{};
            }

            return iface_ptr(*static_cast<const Wrap *>(this))->operator()(std::forward<FArgs>(fargs)...);
        }

        explicit operator bool() const noexcept
        {
            // NOTE: a wrap in the invalid state is considered empty.
            if (is_invalid(*static_cast<const Wrap *>(this))) {
                return false;
            } else {
                return static_cast<bool>(*iface_ptr(*static_cast<const Wrap *>(this)));
            }
        }
    };
};

// Configuration of the callable wrap.
template <typename R, typename... Args>
inline constexpr auto callable_wrap_config = tanuki::config<R (*)(Args...), callable_ref_iface<R, Args...>>{
    .static_size = tanuki::holder_size<R (*)(Args...), callable_iface<R, Args...>>,
    .static_align = tanuki::holder_align<R (*)(Args...), callable_iface<R, Args...>>,
    .pointer_interface = false,
    .explicit_ctor = tanuki::wrap_ctor::always_implicit};

// Definition of the callable wrap.
template <typename R, typename... Args>
using callable_wrap_t = tanuki::wrap<callable_iface<R, Args...>, callable_wrap_config<R, Args...>>;

// Specialise is_any_callable to detect callables.
template <typename R, typename... Args>
struct is_any_callable<callable_wrap_t<R, Args...>> : std::true_type {
};

template <typename T>
struct callable_impl {
};

template <typename R, typename... Args>
struct callable_impl<R(Args...) const> {
    using type = callable_wrap_t<R, Args...>;
};

// Definition of the callable object.
template <typename T>
    requires(requires() { typename callable_impl<T>::type; })
using callable = typename callable_impl<T>::type;

int doubler(int n)
{
    return n * 2;
}

int main()
{
    assert(!callable<void() const>{});
    assert(!callable<void() const>{static_cast<void (*)()>(nullptr)});
    assert(!callable<void() const>{std::function<void()>{}});
    assert(!callable<void() const>{callable<int() const>{}});

    auto lambda_double = [](int n) { return n * 2; };
    callable<int(int) const> c0 = lambda_double;
    assert(c0(2) == 4);

    callable<int(int) const> c0_ref = std::ref(lambda_double);
    assert(c0_ref(2) == 4);
    assert(&value_ref<std::reference_wrapper<decltype(lambda_double)>>(c0_ref).get() == &lambda_double);

    auto mutable_lambda = [m = 5](int n) mutable {
        m = m + n;
        return n * 2;
    };
    assert((!std::constructible_from<callable<int(int) const>, decltype(mutable_lambda)>));

    callable<int(int) const> c1 = doubler;
    assert(c1(2) == 4);
}
