#include <iostream>

#include <tanuki/tanuki.hpp>

template <typename Base, typename Holder, typename T>
struct foo_iface_impl : public Base {
    void foo() const override
    {
        std::cout << "foo_iface_impl calling foo()\n";
        getval<Holder>(this).foo();
    }
};

struct foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

struct foo_model {
    void foo() const {}
};

struct foo_ref_iface1 {
    template <typename Wrap>
    struct impl {
        TANUKI_REF_IFACE_MEMFUN(foo)
    };
};

struct foo_ref_iface2 {
    template <typename Wrap>
    struct impl {
        void foo() const
        {
            iface_ptr(*static_cast<const Wrap *>(this))->foo();
        }
    };
};

inline constexpr auto foo_config1 = tanuki::config<void, foo_ref_iface1>{.pointer_interface = false};

inline constexpr auto foo_config2 = tanuki::config<void, foo_ref_iface2>{.pointer_interface = false};

int main()
{
    using foo_wrap1 = tanuki::wrap<foo_iface, foo_config1>;

    foo_wrap1 w1(foo_model{});
    w1.foo();

    using foo_wrap2 = tanuki::wrap<foo_iface, foo_config2>;

    foo_wrap2 w2(foo_model{});
    w2.foo();
}
