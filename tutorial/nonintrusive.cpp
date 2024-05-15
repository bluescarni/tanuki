#include <iostream>
#include <string>

#include <tanuki/tanuki.hpp>

namespace ns
{

// An existing OO interface.
struct my_iface {
    virtual ~my_iface() = default;
    virtual int foo() const = 0;
};

} // namespace ns

namespace tanuki
{

// Non-intrusive implementation for the ns::my_iface interface.
template <typename Base, typename Holder, typename T>
struct iface_impl<ns::my_iface, Base, Holder, T> : public Base {
    int foo() const override
    {
        return 42;
    }
};

} // namespace tanuki

int main()
{
    // Define a wrap for ns::my_iface.
    using wrap_t = tanuki::wrap<ns::my_iface>;

    wrap_t w1(123);
    wrap_t w2(std::string("hello world!"));

    std::cout << "The final answer is " << w1->foo() << '\n';
}
