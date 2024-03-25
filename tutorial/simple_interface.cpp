#include <iostream>

#include <tanuki/tanuki.hpp>

template <typename Base, typename Holder, typename T>
struct foo1_iface_impl : public Base {
    void foo() const override
    {
        std::cout << "foo1_iface_impl calling foo()\n";
        static_cast<const Holder *>(this)->m_value.foo();
    }
};

struct foo1_iface {
    virtual ~foo1_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo1_iface_impl<Base, Holder, T>;
};

struct foo_model {
    void foo() const
    {
        std::cout << "foo_model calling foo()\n";
    }
};

template <typename Base, typename Holder, typename T>
struct foo2_iface_impl : public Base {
    void foo() const override
    {
        std::cout << "foo2_iface_impl calling foo()\n";
        getval<Holder>(this).foo();
    }
};

struct foo2_iface {
    virtual ~foo2_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo2_iface_impl<Base, Holder, T>;
};

template <typename T>
concept fooable = requires(const T &x) { x.foo(); };

template <typename Base, typename Holder, typename T>
    requires fooable<T>
struct foo3_iface_impl : public Base {
    void foo() const override
    {
        std::cout << "foo3_iface_impl calling foo()\n";
        getval<Holder>(this).foo();
    }
};

struct foo3_iface {
    virtual ~foo3_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo3_iface_impl<Base, Holder, T>;
};

template <typename Base, typename Holder, typename T>
struct foo4_iface_impl {
};

template <typename Base, typename Holder, typename T>
    requires fooable<T>
struct foo4_iface_impl<Base, Holder, T> : public Base {
    void foo() const override
    {
        std::cout << "foo4_iface_impl calling foo()\n";
        getval<Holder>(this).foo();
    }
};

template <typename Base, typename Holder>
struct foo4_iface_impl<Base, Holder, int> : public Base {
    void foo() const override
    {
        std::cout << "foo4_iface_impl implementing foo() for the integer " << getval<Holder>(this) << "\n";
    }
};

struct foo4_iface {
    virtual ~foo4_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo4_iface_impl<Base, Holder, T>;
};

int main()
{
    using foo1_wrap = tanuki::wrap<foo1_iface>;

    foo1_wrap w1(foo_model{});
    w1->foo();

    using foo2_wrap = tanuki::wrap<foo2_iface>;

    foo2_wrap w2(foo_model{});
    w2->foo();

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
