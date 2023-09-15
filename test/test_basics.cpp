#include <functional>
#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

// NOLINTNEXTLINE
struct iface0 {
    virtual ~iface0() = default;
};

template <typename Derived>
struct iface0_impl : iface0 {
};

using tanuki::wrap;

// NOLINTNEXTLINE
TEST_CASE("basics")
{
    struct blaf {
        char buffer[100];
    };

    wrap<iface0, iface0_impl> w1(3.), w2(blaf{}), w3(std::function<void()>{});

    // auto w4 = w3;
    auto w5(std::move(w2));

    w1 = std::move(w3);
}