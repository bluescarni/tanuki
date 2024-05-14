#include <iostream>

#include <tanuki/tanuki.hpp>

template <typename Base, typename Holder, typename T>
struct any_iface_impl : public Base {
};

struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

inline constexpr auto custom_config1 = tanuki::config<>{.static_size = tanuki::holder_size<void *, any_iface>};

int main()
{
    using wrap1_t = tanuki::wrap<any_iface, custom_config1>;

    wrap1_t w1(nullptr);

    std::cout << std::boolalpha;
    std::cout << "Is w1 using static storage? " << has_static_storage(w1) << '\n';

    struct two_ptrs {
        void *p1 = nullptr;
        void *p2 = nullptr;
    };

    wrap1_t w2(two_ptrs{});

    std::cout << "Is w2 using static storage? " << has_static_storage(w2) << '\n';

    constexpr auto custom_config2 = tanuki::config<>{.static_size = tanuki::holder_size<void *, any_iface>,
                                                     .static_align = tanuki::holder_align<void *, any_iface>};

    using wrap2_t = tanuki::wrap<any_iface, custom_config2>;

    wrap2_t w3(nullptr);

    std::cout << "Is w3 using static storage? " << has_static_storage(w3) << '\n';

    std::cout << "sizeof(wrap1_t) is " << sizeof(wrap1_t) << ", sizeof(wrap2_t) is " << sizeof(wrap2_t) << '\n';
}
