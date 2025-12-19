#include <iostream>

#include <tanuki/tanuki.hpp>

struct foo_model {
    // NOLINTNEXTLINE
    void foo() const
    {
        std::cout << "foo_model calling foo()\n";
    }
};

template <typename T>
concept fooable = requires(const T &x) { x.foo(); };

template <typename Base, typename T>
    requires fooable<T>
struct foo3_iface_impl : public Base {
    void foo() const override
    {
        std::cout << "foo3_iface_impl calling foo()\n";
        getval(this).foo();
    }
};

// NOLINTNEXTLINE
struct foo3_iface {
    virtual void foo() const = 0;

    template <typename Base, typename T>
    using impl = foo3_iface_impl<Base, T>;
};

template <typename Base, typename T>
struct foo4_iface_impl {
};

template <typename Base, typename T>
    requires fooable<T>
struct foo4_iface_impl<Base, T> : public Base {
    void foo() const override
    {
        std::cout << "foo4_iface_impl calling foo()\n";
        getval(this).foo();
    }
};

template <typename Base>
struct foo4_iface_impl<Base, int> : public Base {
    void foo() const override
    {
        std::cout << "foo4_iface_impl implementing foo() for the integer " << getval(this) << "\n";
    }
};

// NOLINTNEXTLINE
struct foo4_iface {
    virtual void foo() const = 0;

    template <typename Base, typename T>
    using impl = foo4_iface_impl<Base, T>;
};

int main()
{
    using foo3_wrap = tanuki::wrap<foo3_iface>;

    foo3_wrap w3(foo_model{});
    w3->foo();

    std::cout << std::boolalpha;
    std::cout << "Is foo3_wrap constructible from an int? " << std::is_constructible_v<foo3_wrap, int> << '\n';

    using foo4_wrap = tanuki::wrap<foo4_iface>;

    foo4_wrap w4(foo_model{});
    w4->foo();
    foo4_wrap w4a(42);
    w4a->foo();

    std::cout << "Is foo4_wrap constructible from a float? " << std::is_constructible_v<foo4_wrap, float> << '\n';
}
