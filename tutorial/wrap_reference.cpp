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
struct foobar_iface_impl<Base, Holder, T> : public Base, tanuki::iface_impl_helper<Base, Holder> {
    void foo() const override
    {
        std::cout << "foobar_iface_impl calling foo()\n";
        this->value().foo();
    }
    void bar() override
    {
        std::cout << "foobar_iface_impl calling bar()\n";
        this->value().bar();
    }
};

struct foobar_iface {
    virtual ~foobar_iface() = default;
    virtual void foo() const = 0;
    virtual void bar() = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foobar_iface_impl<Base, Holder, T>;
};

struct foo_model {
    void foo() const
    {
        std::cout << "foo_model calling foo()\n";
    }
    void bar()
    {
        std::cout << "foo_model calling bar()\n";
    }
};

int main()
{
    using wrap_t = tanuki::wrap<foobar_iface>;

    wrap_t w1{foo_model{}};
    w1->foo();

    foo_model f;

    wrap_t w2{std::ref(f)};
    w2->bar();

    std::cout << "f points to              : " << &f << '\n';
    std::cout << "The value in w2 points to: " << &value_ref<std::reference_wrapper<foo_model>>(w2).get() << '\n';
}