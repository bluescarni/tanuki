#include <concepts>
#include <cstddef>
#include <iterator>
#include <list>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "random_access_iterator.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic")
{
    using Catch::Matchers::Message;

    int n[] = {1, 2, 3};
    auto nit = facade::make_random_access_iterator(&n[0]);

    REQUIRE(nit == nit);
    REQUIRE(!(nit < nit));
    nit += 1;
    REQUIRE(*nit == 2);
    nit -= 1;
    REQUIRE(*nit == 1);

    REQUIRE(*(nit + 2) == 3);
    REQUIRE(*(nit + 2 - 1) == 2);
    REQUIRE(nit[2] == 3);

    REQUIRE(nit - nit == 0);
    REQUIRE(nit + 1 - nit == 1);
    REQUIRE(nit + 2 - 1 - nit == 1);

    const decltype(nit) foo;
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
