#include <boost/archive/binary_oarchive.hpp>
#include <iostream>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "fooable.hpp"
#include "tanuki/tanuki.hpp"

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// A test to check that registering the serialisation for a wrap
// in a separate library works as expected.
TEST_CASE("test s11n")
{
    fooable::foo_wrap<int> f{fooable::foo_model{.n = 5}};

    std::cout << "Constructed!" << std::endl;

    std::stringstream ss;

    {
        boost::archive::binary_oarchive oa(ss);
        oa << f;
    }

    std::cout << "Serialised!" << std::endl;

    value_ref<fooable::foo_model>(f).n = 0;

    {
        boost::archive::binary_iarchive ia(ss);
        ia >> f;
    }

    std::cout << "Deserialised!" << std::endl;
    std::cout << "The internal value type is: " << tanuki::demangle(value_type_index(f).name()) << std::endl;

    REQUIRE(value_ref<fooable::foo_model>(f).n == 5);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
