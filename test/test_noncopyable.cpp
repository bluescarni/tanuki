#include <concepts>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename Base, typename, typename>
struct any_iface_impl : Base {
};

// NOLINTNEXTLINE
struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

template <int N>
struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable &) = delete;
    noncopyable(noncopyable &&) noexcept = default;
    noncopyable &operator=(const noncopyable &) = delete;
    noncopyable &operator=(noncopyable &&) noexcept = default;
    ~noncopyable() = default;

    int m_value[N] = {};
};

TEST_CASE("noncopyable")
{
    using wrap_t = tanuki::wrap<any_iface>;

    REQUIRE(!std::constructible_from<wrap_t, noncopyable<1>>);
    REQUIRE(!std::constructible_from<wrap_t, noncopyable<100>>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
