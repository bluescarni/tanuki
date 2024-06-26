#include <iostream>

#include <tanuki/tanuki.hpp>

template <typename Base, typename Holder, typename T>
struct foo_iface_impl : public Base {
    void foo() const final
    {
        getval<Holder>(this).foo();
    }
};

// The foo() interface.
struct foo_iface {
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename T>
struct bar_iface_impl : foo_iface_impl<Base, Holder, T> {
    void bar() const final
    {
        getval<Holder>(this).bar();
    }
};

// The bar() interface.
struct bar_iface : foo_iface {
    virtual void bar() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = bar_iface_impl<Base, Holder, T>;
};

struct foobar_model {
    void foo() const
    {
        std::cout << "Invoking foobar_model::foo()" << '\n';
    };
    void bar() const
    {
        std::cout << "Invoking foobar_model::bar()" << '\n';
    };
};

int main()
{
    // Define a wrap for the composite interface.
    using wrap_t = tanuki::wrap<bar_iface>;

    // Wrap an object that satisfies the composite interface.
    wrap_t w1(foobar_model{});

    // Invoke the member functions.
    w1->foo();
    w1->bar();
}
