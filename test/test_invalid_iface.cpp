#include <concepts>
#include <iostream>
#include <string>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename T>
concept ostreamable = requires(std::ostream &os, const T &x) { os << x; };

template <typename Base, typename Holder, typename T>
    requires ostreamable<T>
struct print_iface_impl : Base {
    void print() const final
    {
        std::cout << getval<Holder>(this) << '\n';
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int counter = 0;

template <typename Base, typename Holder>
struct print_iface_impl<Base, Holder, int> : Base {
    void print() const final
    {
        ++counter;
        std::cout << "int ostream: " << getval<Holder>(this) << '\n';
    }
};

// NOLINTNEXTLINE
struct print_iface {
    virtual ~print_iface() = default;
    virtual void print() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = print_iface_impl<Base, Holder, T>;
};

using print_wrap = tanuki::wrap<print_iface>;

void print_iface_func(print_wrap &);

struct foo {
    friend std::ostream &operator<<(std::ostream &os, const foo &)
    {
        return os << "printing a foo";
    }
};

TEST_CASE("invalid iface")
{
    print_wrap w1(42);
    print_wrap w2(std::string("hello world"));

    REQUIRE(counter == 0);
    w1->print();
    REQUIRE(counter == 1);
    w2->print();

    print_wrap w3(foo{});
    w3->print();

    struct bar {
    };
    REQUIRE(!std::constructible_from<print_wrap, bar>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
