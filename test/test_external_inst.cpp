#include <catch2/catch_test_macros.hpp>

#include "external_inst.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("external inst")
{
    const fooable::foo_wrap<int> f1{fooable::foo_model{}};
    const fooable::foo_wrap<double> f2{fooable::foo_model{}};
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
