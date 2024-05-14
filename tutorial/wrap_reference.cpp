#include <functional>
#include <iostream>

#include <tanuki/tanuki.hpp>

template <typename Base, typename Holder, typename T>
struct foobar_iface_impl {
};

template <typename T>
concept fooable = requires(const T &x) { x.foo(); };

template <typename T>
concept barable = requires(T &x) { x.bar(); };

template <typename Base, typename Holder, typename T>
    requires fooable<tanuki::unwrap_cvref_t<T>> && barable<tanuki::unwrap_cvref_t<T>>
struct foobar_iface_impl<Base, Holder, T> : public Base {
    void foo() const override
    {
        std::cout << "foobar_iface_impl calling foo()\n";
        getval<Holder>(this).foo();
    }
    void bar() override
    {
        std::cout << "foobar_iface_impl calling bar()\n";
        getval<Holder>(this).bar();
    }
};

struct foobar_iface {
    virtual void foo() const = 0;
    virtual void bar() = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foobar_iface_impl<Base, Holder, T>;
};

struct foobar_model {
    void foo() const
    {
        std::cout << "foobar_model calling foo()\n";
    }
    void bar()
    {
        std::cout << "foobar_model calling bar()\n";
    }
};

int main()
{
    using wrap_t = tanuki::wrap<foobar_iface>;

    // Store the copy of an object.
    wrap_t w1{foobar_model{}};
    w1->foo();

    // Store a reference to an existing object.
    foobar_model f;
    wrap_t w2{std::ref(f)};
    w2->bar();

    // Check that the value in w2 points to f.
    std::cout << "f points to              : " << &f << '\n';
    std::cout << "The value in w2 points to: " << &value_ref<std::reference_wrapper<foobar_model>>(w2).get() << '\n';

    // Store a const reference to f.
    wrap_t w3{std::cref(f)};
    // WARNING: this will throw an exception!
    // w3->bar();

    // Store a const reference to f.
    const wrap_t w4{std::cref(f)};
    // OK: this will not compile.
    // w4->bar();
}
