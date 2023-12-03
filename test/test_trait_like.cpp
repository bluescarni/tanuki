#include <functional>
#include <iostream>
#include <string>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-multiple-inheritance,fuchsia-virtual-inheritance)

// Fwd-declaration of the interface template.
template <typename, typename>
struct summary_iface;

// Trait definition.
template <>
// NOLINTNEXTLINE
struct summary_iface<void, void> {
    virtual ~summary_iface() = default;
    [[nodiscard]] virtual std::string summarize() const
    {
        return "(Read more...)";
    }
};

// A couple of classes for which we might want to
// implement the trait.
struct news_article {
    std::string headline;
    std::string location;
    std::string author;
    std::string content;
};

struct tweet {
    std::string username;
    std::string content;
    bool reply = false;
    bool retweet = false;
};

// Default implementation of the trait.
template <typename Holder, typename T>
struct summary_iface : virtual summary_iface<void, void> {
};

// Implement the summary trait for news_article and tweet.
template <typename Holder, typename T>
    requires tanuki::same_or_ref_for<T, news_article>
struct summary_iface<Holder, T> : virtual summary_iface<void, void>,
                                  tanuki::iface_impl_helper<Holder, T, summary_iface> {
    [[nodiscard]] std::string summarize() const final
    {
        return this->value().headline + ", by " + this->value().author + " (" + this->value().location + ")";
    }
};

template <typename Holder, typename T>
    requires tanuki::same_or_ref_for<T, tweet>
struct summary_iface<Holder, T> : virtual summary_iface<void, void>,
                                  tanuki::iface_impl_helper<Holder, T, summary_iface> {
    [[nodiscard]] std::string summarize() const final
    {
        return this->value().username + ": " + this->value().content;
    }
};

// Definition of the wrapper.
using summary = tanuki::wrap<summary_iface, tanuki::config<>{.explicit_generic_ctor = false}>;

// A function with summary input param.
summary notify(const summary &s)
{
    std::cout << "Breaking news! " << s->summarize() << '\n';

    return s;
}

TEST_CASE("summary example")
{
    struct foo {
    };

    notify(news_article{.headline = "Aliens!", .location = "New York", .author = "Goofy"});
    notify(tweet{.username = "Donald Duck", .content = "Big, if true!"});
    notify(foo{});

    news_article na{.headline = "Aliens!", .location = "New York", .author = "Goofy"};
    tweet t{.username = "Donald Duck", .content = "Big, if true!"};
    foo f;

    notify(std::ref(na));
    notify(std::ref(t));
    notify(std::ref(f));

    REQUIRE(&value_ref<std::reference_wrapper<news_article>>(notify(std::ref(na))).get() == &na);
    REQUIRE(&value_ref<std::reference_wrapper<tweet>>(notify(std::ref(t))).get() == &t);
    REQUIRE(&value_ref<std::reference_wrapper<foo>>(notify(std::ref(f))).get() == &f);
}

template <typename, typename>
struct fooable_iface;

template <>
// NOLINTNEXTLINE
struct fooable_iface<void, void> {
    virtual ~fooable_iface() = default;
    [[nodiscard]] virtual std::string foo() const
    {
        return "default foo!";
    }
};

struct foo_capable {
    std::string foo;
};

template <typename Holder, typename T>
struct fooable_iface : virtual fooable_iface<void, void> {
};

// Implement the summary trait for news_article and tweet.
template <typename Holder, typename T>
    requires tanuki::same_or_ref_for<T, foo_capable>
struct fooable_iface<Holder, T> : virtual fooable_iface<void, void>,
                                  tanuki::iface_impl_helper<Holder, T, fooable_iface> {
    [[nodiscard]] std::string foo() const final
    {
        return this->value().foo;
    }
};

using fooable = tanuki::wrap<fooable_iface, tanuki::config<>{.explicit_generic_ctor = false}>;

using fooable_summary = tanuki::composite_cwrap<tanuki::config<>{.explicit_generic_ctor = false}, summary, fooable>;

fooable_summary notify_fooable(const fooable_summary &s)
{
    std::cout << "Breaking news! " << s->summarize() << '\n';
    std::cout << "About to foo: " << s->foo() << '\n';

    return s;
}

TEST_CASE("composite wrap")
{
    notify_fooable(tweet{.username = "Donald Duck", .content = "Big, if true!"});

    tweet t{.username = "Donald Duck", .content = "Big, if true!"};

    REQUIRE(&value_ref<std::reference_wrapper<tweet>>(notify_fooable(std::ref(t))).get() == &t);

    foo_capable f{.foo = "frob the niz"};

    REQUIRE(&value_ref<std::reference_wrapper<foo_capable>>(notify_fooable(std::ref(f))).get() == &f);

    REQUIRE(std::is_same_v<const tanuki::wrap_interface_t<fooable_summary> *,
                           decltype(iface_ptr(
                               fooable_summary{tweet{.username = "Donald Duck", .content = "Big, if true!"}}))>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-multiple-inheritance,fuchsia-virtual-inheritance)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
