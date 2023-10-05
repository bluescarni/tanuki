#include <stdexcept>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename, typename>
struct any_iface;

template <>
// NOLINTNEXTLINE
struct any_iface<void, void> {
    virtual ~any_iface() = default;
};

template <typename Holder, typename T>
struct any_iface : any_iface<void, void> {
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
    using Catch::Matchers::Message;

    using wrap_t = tanuki::wrap<any_iface>;

    wrap_t w(noncopyable<1>{});

    REQUIRE_THROWS_MATCHES(wrap_t(w), std::invalid_argument,
                           Message("Attempting to copy-construct a non-copyable value type"));

    w = wrap_t(noncopyable<100>{});

    REQUIRE_THROWS_MATCHES(wrap_t(w), std::invalid_argument, Message("Attempting to clone a non-copyable value type"));

    wrap_t w2(noncopyable<100>{});
    REQUIRE_THROWS_MATCHES(w2 = w, std::invalid_argument,
                           Message("Attempting to copy-assign a non-copyable value type"));
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
