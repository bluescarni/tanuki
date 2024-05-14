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

template <typename Base, typename Holder, typename T, typename R, typename... Args>
struct func_iface_impl {
};

template <typename R, typename... Args>
// NOLINTNEXTLINE
struct func_iface {
    virtual R operator()(Args... args) const = 0;
    virtual explicit operator bool() const noexcept = 0;

    template <typename Base, typename Holder, typename T>
    using impl = func_iface_impl<Base, Holder, T, R, Args...>;
};

template <typename Base, typename Holder, typename T, typename R, typename... Args>
    requires std::is_invocable_r_v<R, const tanuki::unwrap_cvref_t<T> &, Args...>
struct func_iface_impl<Base, Holder, T, R, Args...> : Base {
    R operator()(Args... args) const final
    {
        using uT = tanuki::unwrap_cvref_t<T>;

        if constexpr (std::is_pointer_v<uT> || std::is_member_pointer_v<uT>) {
            if (getval<Holder>(this) == nullptr) {
                throw std::bad_function_call{};
            }
        } else if constexpr (tanuki::any_wrap<uT>) {
            if (is_invalid(getval<Holder>(this))) {
                throw std::bad_function_call{};
            }
        } else if constexpr (is_any_function<uT>::value) {
            if (!getval<Holder>(this)) {
                throw std::bad_function_call{};
            }
        }

        if constexpr (std::same_as<R, void>) {
            static_cast<void>(std::invoke(getval<Holder>(this), std::forward<Args>(args)...));
        } else {
            return std::invoke(getval<Holder>(this), std::forward<Args>(args)...);
        }
    }
    explicit operator bool() const noexcept final
    {
        using uT = tanuki::unwrap_cvref_t<T>;

        if constexpr (std::is_pointer_v<uT> || std::is_member_pointer_v<uT>) {
            return getval<Holder>(this) != nullptr;
        } else if constexpr (tanuki::any_wrap<uT>) {
            return !is_invalid(getval<Holder>(this));
        } else if constexpr (is_any_function<uT>::value) {
            return static_cast<bool>(getval<Holder>(this));
        } else {
            return true;
        }
    }
};

template <typename R, typename... Args>
struct std_func_ref_iface {
    template <typename Wrap>
    struct impl {
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
};

template <typename>
struct std_func_impl {
};

template <typename R, typename... Args>
struct std_func_impl<R(Args...)> {
    using type
        = tanuki::wrap<func_iface<R, Args...>,
                       tanuki::config<R (*)(Args...), std_func_ref_iface<R, Args...>>{.pointer_interface = false}>;
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
    REQUIRE(!std::constructible_from<std_func<double(int)>, void>);
    REQUIRE(!std::constructible_from<std_func<void(int)>, int>);

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
