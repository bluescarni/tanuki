#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "io_iterator.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename T>
concept can_make_io_iterator = requires(T it) { facade::make_io_iterator(it); };

TEST_CASE("basic")
{
    using int_iter = facade::io_iterator<int &>;

    REQUIRE(std::input_or_output_iterator<int_iter>);
    REQUIRE(!std::default_initializable<int_iter>);
    REQUIRE(!std::constructible_from<int_iter, int>);
    REQUIRE(!can_make_io_iterator<int>);

    REQUIRE(std::same_as<std::iter_reference_t<int_iter>, int &>);
    REQUIRE(std::same_as<std::iter_reference_t<facade::io_iterator<int>>, int>);
    REQUIRE(std::same_as<std::iter_reference_t<facade::io_iterator<int &&>>, int &&>);
    REQUIRE(std::same_as<std::ptrdiff_t, std::iter_difference_t<int_iter>>);

    {
        int arr[] = {1, 2, 3};
        int_iter it(std::begin(arr));
        REQUIRE(has_static_storage(it));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);

        // Check that make_io_iterator() on an io_iterator
        // returns a copy.
        auto it2 = facade::make_io_iterator(facade::make_io_iterator(std::begin(arr)));
        REQUIRE(value_isa<int *>(it2));
    }

    {
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_io_iterator(std::begin(vec));
        REQUIRE(std::same_as<decltype(it), int_iter>);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);
    }

    {
        // NOLINTNEXTLINE(misc-const-correctness)
        std::vector<int> vec = {1, 2, 3};
        auto it = facade::make_io_iterator(vec.cbegin());
        REQUIRE(std::same_as<decltype(it), facade::io_iterator<const int &>>);
        REQUIRE(std::input_or_output_iterator<decltype(it)>);
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);
    }

    {
        std::list<int> lst = {1, 2, 3};
        int_iter it(std::begin(lst));
        REQUIRE(*it == 1);
        REQUIRE(*++it == 2);
        it++;
        REQUIRE(*it == 3);
    }
}

struct noniter {
    double operator*() const
    {
        return {};
    }
    void operator++() {}
};

// NOTE: missing equality comparability.
struct failiter {
    double operator*() const
    {
        return {};
    }
    void operator++() {}
};

bool operator==(const noniter &, const noniter &);

TEST_CASE("noniter")
{
    using iter_t = facade::io_iterator<double>;

    REQUIRE(std::same_as<iter_t, decltype(facade::make_io_iterator(noniter{}))>);

    REQUIRE(std::input_or_output_iterator<iter_t>);
    REQUIRE(!std::default_initializable<iter_t>);
    REQUIRE(!std::constructible_from<iter_t, int>);

    REQUIRE(!can_make_io_iterator<failiter>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
