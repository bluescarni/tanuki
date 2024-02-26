#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "input_iterator.hpp"
#include "sentinel.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename T>
concept can_make_input_iterator = requires(T it) { facade::make_input_iterator(it); };

TEST_CASE("basic")
{
    using int_iter = facade::input_iterator<int, int &, int &&>;

    REQUIRE(std::input_or_output_iterator<int_iter>);
    REQUIRE(std::input_iterator<int_iter>);
    REQUIRE(!std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);
    REQUIRE(!can_make_input_iterator<int>);

    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);
    REQUIRE(std::same_as<int, std::iter_value_t<int_iter>>);
    REQUIRE(std::same_as<int &, std::iter_reference_t<int_iter>>);
    REQUIRE(std::same_as<int &&, std::iter_rvalue_reference_t<int_iter>>);
    REQUIRE(std::same_as<std::input_iterator_tag, int_iter::iterator_concept>);

    {
        int arr[] = {1, 2, 3};
        int_iter it(std::begin(arr));
        REQUIRE(has_static_storage(it));
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*std::as_const(it) == 3);

        // Check that make_input_iterator() on an io_iterator
        // returns a copy.
        auto it2 = facade::make_input_iterator(facade::make_input_iterator(std::begin(arr)));
        REQUIRE(value_isa<int *>(it2));

        REQUIRE(it == facade::sentinel(arr + 2));
        REQUIRE(it != facade::sentinel(arr + 1));
        REQUIRE(std::sentinel_for<facade::sentinel, int_iter>);
    }

    {
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_input_iterator(std::begin(vec));
        REQUIRE(std::same_as<decltype(it), int_iter>);
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*std::as_const(it) == 3);

        REQUIRE(it == facade::sentinel(vec.begin() + 2));
        REQUIRE(it != facade::sentinel(vec.begin() + 1));
        REQUIRE(std::sentinel_for<facade::sentinel, int_iter>);
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_input_iterator(vec.cbegin());
        REQUIRE(std::same_as<decltype(it), facade::input_iterator<int, const int &, const int &&>>);
        REQUIRE(std::input_iterator<decltype(it)>);
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*std::as_const(it) == 3);
    }

    {
        std::list<int> lst = {1, 2, 3};
        int_iter it(std::begin(lst));
        REQUIRE(*std::as_const(it) == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*std::as_const(it) == 3);
    }
}

// LCOV_EXCL_START

struct noniter1 {
    struct sentinel_t {
    };

    double operator*() const
    {
        return {};
    }
    void operator++() {}
    bool operator==(const sentinel_t &) const
    {
        return false;
    };
};

struct noniter2 {
    struct sentinel_t {
    };

    double &operator*() const
    {
        static double x = 56;
        return x;
    }
    void operator++() {}
    bool operator==(const sentinel_t &) const
    {
        return false;
    };
};

struct noniter3 {
    struct sentinel_t {
    };

    const double &operator*() const
    {
        static const double x = 56;
        return x;
    }
    void operator++() {}
    bool operator==(const sentinel_t &) const
    {
        return false;
    };
};

struct noniter4 {
    struct sentinel_t {
    };

    double &&operator*() const
    {
        static double x = 56;
        return static_cast<double &&>(x);
    }
    void operator++() {}
    bool operator==(const sentinel_t &) const
    {
        return false;
    };
};

struct noniter5 {
    struct sentinel_t {
    };

    const double &&operator*() const
    {
        static const double x = 56;
        return static_cast<const double &&>(x);
    }
    void operator++() {}
    bool operator==(const sentinel_t &) const
    {
        return false;
    };
};

// LCOV_EXCL_STOP

TEST_CASE("noniter")
{
    {
        using iter_t = facade::input_iterator<double, double, double>;

        auto nit = facade::make_input_iterator(noniter1{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::input_iterator<iter_t>);
        REQUIRE(!std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::input_iterator<double, double &, double &&>;

        auto nit = facade::make_input_iterator(noniter2{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::input_iterator<iter_t>);
        REQUIRE(!std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::input_iterator<double, const double &, const double &&>;

        auto nit = facade::make_input_iterator(noniter3{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::input_iterator<iter_t>);
        REQUIRE(!std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::input_iterator<double, double &&, double &&>;

        auto nit = facade::make_input_iterator(noniter4{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::input_iterator<iter_t>);
        REQUIRE(!std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }

    {
        using iter_t = facade::input_iterator<double, const double &&, const double &&>;

        auto nit = facade::make_input_iterator(noniter5{});
        REQUIRE(std::same_as<iter_t, decltype(nit)>);

        REQUIRE(std::input_iterator<iter_t>);
        REQUIRE(!std::default_initializable<iter_t>);
        REQUIRE(!std::constructible_from<iter_t, int>);
    }
}

namespace ns
{

// LCOV_EXCL_START

struct iter_move1 {
    struct sentinel_t {
    };

    double operator*() const
    {
        return {};
    }
    void operator++() {}
    bool operator==(const sentinel_t &) const
    {
        return false;
    };
};

// LCOV_EXCL_STOP

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int iter_move1_counter = 0;

double iter_move(const iter_move1 &)
{
    ++iter_move1_counter;

    return 0;
}

} // namespace ns

// A test to check that the iter_move() customisation
// in the reference interface is picked up correctly.
TEST_CASE("iter_move")
{
    auto nit = facade::make_input_iterator(ns::iter_move1{});
    REQUIRE(std::same_as<facade::input_iterator<double, double, double>, decltype(nit)>);
    (void)std::ranges::iter_move(nit);
    (void)std::ranges::iter_move(nit);
    (void)std::ranges::iter_move(nit);
    REQUIRE(ns::iter_move1_counter == 3);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
