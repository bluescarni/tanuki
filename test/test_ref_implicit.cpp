#include <concepts>
#include <functional>
#include <iostream>
#include <string>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-virtual-inheritance)

template <typename Base, typename, typename>
struct summary_iface_impl : Base {
};

// NOLINTNEXTLINE
struct summary_iface {
    virtual ~summary_iface() = default;
    [[nodiscard]] virtual std::string summarize() const
    {
        return "(Read more...)";
    }

    template <typename Base, typename Holder, typename T>
    using impl = summary_iface_impl<Base, Holder, T>;
};

struct news_article {
    std::string headline;
    std::string location;
    std::string author;
    std::string content;
};

template <typename Base, typename Holder, typename T>
    requires std::same_as<tanuki::unwrap_cvref_t<T>, news_article>
struct summary_iface_impl<Base, Holder, T> : Base, tanuki::iface_impl_helper<Base, Holder> {
    [[nodiscard]] std::string summarize() const final
    {
        return this->value().headline + ", by " + this->value().author + " (" + this->value().location + ")";
    }
};

// Definition of the wrapper.
using summary = tanuki::wrap<summary_iface, tanuki::config<>{.explicit_ctor = tanuki::wrap_ctor::ref_implicit}>;

// A function with summary input param.
summary notify(const summary &s)
{
    std::cout << "Breaking news! " << s->summarize() << '\n';

    return s;
}

template <typename T>
concept can_notify = requires(const T &x) { notify(x); };

TEST_CASE("implicit ref ctor")
{
    REQUIRE(std::convertible_to<std::reference_wrapper<news_article>, summary>);
    REQUIRE(std::convertible_to<std::reference_wrapper<const news_article>, summary>);
    REQUIRE(!std::convertible_to<news_article, summary>);

    const summary s1{123};
    notify(s1);

    const news_article n{.author = "Michael Foobar"};
    notify(std::ref(n));

    REQUIRE(!can_notify<news_article>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-virtual-inheritance)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
