#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "fooable.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// A test to check that registering the serialisation for a wrap
// in a separate library works as expected.
TEST_CASE("test s11n")
{
    fooable::foo_wrap<int> f{fooable::foo_model{.n = 5}};

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << f;
    }

    value_ref<fooable::foo_model>(f).n = 0;

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> f;
    }

    REQUIRE(value_ref<fooable::foo_model>(f).n == 5);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
