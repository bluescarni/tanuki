#include <tuple>
#include <vector>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename, typename, typename>
struct foo_iface_impl {
};

template <typename T>
concept fooable = requires(const T &x) { static_cast<void>(x.foo()); };

template <typename Base, typename Holder, typename T>
    requires fooable<T>
struct foo_iface_impl<Base, Holder, T> : Base, tanuki::iface_impl_helper<Base, Holder> {
    void foo() const final
    {
        static_cast<void>(this->value().foo());
    }
};

// NOLINTNEXTLINE
struct foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

struct f1 {
    void foo() const {}
};

TEST_CASE("inf loop")
{
    using wrap1_t = tanuki::wrap<foo_iface, tanuki::config<f1>{.explicit_generic_ctor = false}>;

    std::vector<std::tuple<wrap1_t>> v;
    // NOTE: GCC and clang (but not MSVC strangely enough) would error out
    // on the next line, before we re-ordered the concept checks in ctible_holder.
    // Apparently, for some reason I do not fully understand (but I think related
    // to the implicitness of the generic ctor), the compiler would end
    // up attempting to construct a wrap1_t from std::tuple<wrap1_t>, deep within
    // type traits/concept checking in the implementation of std::vector/std::tuple.
    // This would result in some sort of infinite recursion in concept checking
    // due to the satisfaction of a constraint depending on itself.
    v.resize(10u);
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
