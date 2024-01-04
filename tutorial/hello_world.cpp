#include <memory>
#include <string>
#include <utility>

#include <tanuki/tanuki.hpp>

template <typename Base, typename Holder, typename T>
struct any_iface_impl : public Base {
};

struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

// An empty implementation.
struct any1 : any_iface {
};

// An implementation storing a string.
struct any2 : any_iface {
    std::string str_;

    explicit any2(std::string s) : str_(std::move(s)) {}
};

int main()
{
    // Traditional OO-style.
    std::unique_ptr<any_iface> ptr1 = std::make_unique<any1>();
    std::unique_ptr<any_iface> ptr2 = std::make_unique<any2>("hello world");

    // Type-erasure approach.
    using any_wrap = tanuki::wrap<any_iface>;

    // Store an integer.
    any_wrap w1(42);

    // Store a string.
    any_wrap w2(std::string("hello world"));

    // Store anything...
    struct foo {
    };
    any_wrap w3(foo{});
}
