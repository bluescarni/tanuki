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
struct print_iface_impl : Base, tanuki::iface_impl_helper<Base, Holder> {
    void print() const final
    {
        std::cout << this->value() << '\n';
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

    w1->print();
    w2->print();

    print_wrap w3(foo{});
    w3->print();

    struct bar {
    };
    REQUIRE(!std::constructible_from<print_wrap, bar>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
