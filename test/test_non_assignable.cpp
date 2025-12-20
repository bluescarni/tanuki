#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// NOTE: these are tests for value type which do not support assignment/swap. We want to check that the wrap
// assignment/swap primitives keep on working ok.

template <typename, typename>
struct any_iface_impl {
};

struct any_iface {
    template <typename Base, typename T>
    struct impl : Base {
    };
};

using wrap_t = tanuki::wrap<any_iface>;

struct non_copy_assignable {
    int n = 0;

    explicit non_copy_assignable(int n = 0) : n(n) {}
    non_copy_assignable(const non_copy_assignable &) = default;
    non_copy_assignable(non_copy_assignable &&) = default;
    non_copy_assignable &operator=(const non_copy_assignable &) = delete;
    non_copy_assignable &operator=(non_copy_assignable &&) = default;
    ~non_copy_assignable() = default;
};

TEST_CASE("copy_assignment")
{
    wrap_t w1{non_copy_assignable{}}, w2{non_copy_assignable{42}};
    w1 = w2;
    REQUIRE(value_ref<non_copy_assignable>(w1).n == 42);

    non_copy_assignable tmp{123};
    w1 = tmp;
    REQUIRE(value_ref<non_copy_assignable>(w1).n == 123);
}

struct non_move_assignable {
    int n = 0;

    explicit non_move_assignable(int n = 0) : n(n) {}
    non_move_assignable(const non_move_assignable &) = default;
    non_move_assignable(non_move_assignable &&) = default;
    non_move_assignable &operator=(const non_move_assignable &) = default;
    non_move_assignable &operator=(non_move_assignable &&) = delete;
    ~non_move_assignable() = default;
};

TEST_CASE("move_assignment")
{
    wrap_t w1{non_move_assignable{}}, w2{non_move_assignable{42}};
    w1 = std::move(w2);
    REQUIRE(value_ref<non_move_assignable>(w1).n == 42);

    w1 = non_move_assignable{123};
    REQUIRE(value_ref<non_move_assignable>(w1).n == 123);
}

// NOLINTEND(misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
