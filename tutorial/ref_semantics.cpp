#include <ios>
#include <iostream>
#include <string>

#include <tanuki/tanuki.hpp>

template <typename Base, typename Holder, typename T>
struct any_iface_impl : public Base {
};

struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

int main()
{
    using any_wrap = tanuki::wrap<any_iface, tanuki::config<>{.semantics = tanuki::wrap_semantics::reference}>;

    any_wrap w1(std::string("hello world"));

    auto w2 = w1;

    std::cout << std::boolalpha;

    std::cout << "Address of the value wrapped by w1: " << tanuki::value_ptr<std::string>(w1) << '\n';
    std::cout << "Address of the value wrapped by w2: " << tanuki::value_ptr<std::string>(w2) << '\n';

    std::cout << "Do w1 and w2 share ownership? " << same_value(w1, w2) << '\n';

    auto w3 = copy(w1);

    std::cout << "Address of the value wrapped by w1: " << tanuki::value_ptr<std::string>(w1) << '\n';
    std::cout << "Address of the value wrapped by w3: " << tanuki::value_ptr<std::string>(w3) << '\n';

    std::cout << "Do w1 and w3 share ownership? " << same_value(w1, w3) << '\n';
}
