#include <array>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename T>
struct is_any_function : std::false_type {
};

template <typename R, typename... Args>
struct is_any_function<std::function<R(Args...)>> : std::true_type {
};

template <typename, typename, typename, typename...>
struct func_iface;

template <typename R, typename... Args>
// NOLINTNEXTLINE
struct func_iface<void, void, R, Args...> {
    virtual ~func_iface() = default;
    virtual R operator()(Args... args) const = 0;
    virtual explicit operator bool() const noexcept = 0;
};

template <typename Holder, typename T, typename R, typename... Args>
struct func_iface : func_iface<void, void, R, Args...>, tanuki::iface_impl_helper<Holder, T, func_iface, R, Args...> {
    R operator()(Args... args) const final
    {
        if constexpr (std::is_pointer_v<T> || std::is_member_pointer_v<T>) {
            if (this->value() == nullptr) {
                throw std::bad_function_call{};
            }
        } else if constexpr (tanuki::any_wrap<T>) {
            if (is_invalid(this->value())) {
                throw std::bad_function_call{};
            }
        } else if constexpr (is_any_function<T>::value) {
            if (!this->value()) {
                throw std::bad_function_call{};
            }
        }

        if constexpr (std::same_as<R, void>) {
            static_cast<void>(std::invoke(this->value(), std::forward<Args>(args)...));
        } else {
            return std::invoke(this->value(), std::forward<Args>(args)...);
        }
    }
    explicit operator bool() const noexcept final
    {
        if constexpr (std::is_pointer_v<T> || std::is_member_pointer_v<T>) {
            return this->value() != nullptr;
        } else if constexpr (tanuki::any_wrap<T>) {
            return !is_invalid(this->value());
        } else if constexpr (is_any_function<T>::value) {
            return static_cast<bool>(this->value());
        } else {
            return true;
        }
    }
};

namespace tanuki
{

template <typename Wrap, typename R, typename... Args>
struct ref_iface<Wrap, func_iface, R, Args...> {
    using result_type = R;

    template <typename JustWrap = Wrap, typename... FArgs>
    auto operator()(FArgs &&...fargs) const
        -> decltype(iface_ptr(*static_cast<const JustWrap *>(this))->operator()(std::forward<FArgs>(fargs)...))
    {
        if (is_invalid(*static_cast<const Wrap *>(this))) {
            throw std::bad_function_call{};
        }

        return iface_ptr(*static_cast<const Wrap *>(this))->operator()(std::forward<FArgs>(fargs)...);
    }

    explicit operator bool() const noexcept
    {
        if (is_invalid(*static_cast<const Wrap *>(this))) {
            return false;
        } else {
            return static_cast<bool>(*iface_ptr(*static_cast<const Wrap *>(this)));
        }
    }
};

template <typename T, typename R, typename... Args>
inline constexpr bool is_wrappable<T, func_iface, R, Args...> = std::is_invocable_r_v<R, const T &, Args...>;

} // namespace tanuki

template <typename>
struct std_func_impl {
};

template <typename R, typename... Args>
struct std_func_impl<R(Args...)> {
    using type = tanuki::wrap<func_iface, tanuki::config<R (*)(Args...)>{.pointer_interface = false}, R, Args...>;
};

template <typename T>
using std_func = typename std_func_impl<T>::type;

double f1(int)
{
    return 42;
}

struct large_func {
    std::array<char, 100> arr = {};

    double operator()(int) const
    {
        return 42;
    }
};

TEST_CASE("std_function")
{
    {
        std_func<double(int)> sf;
        REQUIRE(!sf);
        REQUIRE_THROWS_AS(sf(2), std::bad_function_call);
        sf = std_func<double(int)>(std::function<double(int)>{});
        REQUIRE_THROWS_AS(sf(2), std::bad_function_call);
        REQUIRE(!sf);
    }

    {
        std_func<void(int)> sf_void(std_func<double(int)>{large_func{}});
        const auto moved_out = std::move(value_ref<std_func<double(int)>>(sf_void));
        REQUIRE(is_invalid(value_ref<std_func<double(int)>>(sf_void)));
        REQUIRE_THROWS_AS(sf_void(0), std::bad_function_call);
        REQUIRE(!sf_void);
    }

    {
        std_func<double(int)> sf{large_func{}}, sf2(std::move(sf));
        REQUIRE(is_invalid(sf));
        REQUIRE_THROWS_AS(sf(0), std::bad_function_call);
        REQUIRE(!sf);
    }
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
