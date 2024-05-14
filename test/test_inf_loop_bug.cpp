#include <tuple>
#include <vector>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

// LCOV_EXCL_START

template <typename, typename, typename>
struct foo_iface_impl {
};

template <typename T>
concept fooable = requires(const T &x) { static_cast<void>(x.foo()); };

template <typename Base, typename Holder, typename T>
    requires fooable<T>
struct foo_iface_impl<Base, Holder, T> : Base {
    void foo() const final
    {
        static_cast<void>(getval<Holder>(this).foo());
    }
};

// NOLINTNEXTLINE
struct foo_iface {
    virtual void foo() const = 0;

    template <typename Base, typename Holder, typename T>
    using impl = foo_iface_impl<Base, Holder, T>;
};

struct f1 {
    void foo() const {}
};

// LCOV_EXCL_STOP

TEST_CASE("inf loop")
{
    using wrap1_t = tanuki::wrap<foo_iface, tanuki::config<f1>{.explicit_ctor = tanuki::wrap_ctor::always_implicit}>;

    std::vector<std::tuple<wrap1_t>> v;
    // NOTE: GCC and clang (but not MSVC strangely enough) would error out
    // on the next line, before we re-ordered the concept checks in wrap::ctor_impl().
    // Apparently, for some reason I do not fully understand (but I think related
    // to the implicitness of the generic ctor), the compiler would end
    // up attempting to construct a wrap1_t from std::tuple<wrap1_t>, deep within
    // type traits/concept checking in the implementation of std::vector/std::tuple.
    // This would result in some sort of infinite recursion in concept checking
    // due to the satisfaction of a constraint depending on itself.
    // It is not clear whether this is a compiler bug or a legitimate compilation
    // failure, as both MSVC and the EDG compiler are fine with this.
    //
    // UPDATE: after further investigations, it seems this may be related to the way
    // std::tuple is implemented in libstdc++, that is, as a recursive inheritance
    // hierarchy. The sequence of events is not 100% clear, but it seems to involve
    // an explicit static_cast in the tuple code to a "Base&&" that ends up attempting
    // to invoke the implicit generic constructor of the wrap class, leading
    // to the circular dependency of the constraint onto itself. In any case, this
    // is a manifestation of the danger of having an implicit catch-all constructor.
    v.resize(10u);
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
