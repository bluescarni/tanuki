#include <mutex>
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

int main()
{
    using any_wrap = tanuki::wrap<any_iface>;

    // Emplace-construct with std::mutex.
    any_wrap w(std::in_place_type<std::mutex>);

    // Reset to the invalid state.
    w = tanuki::invalid_wrap;

    // Emplace into existing wrap.
    emplace<std::mutex>(w);
}
