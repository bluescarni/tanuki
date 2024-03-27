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

// Default implementation of the summary interface:
// inherits from Base, thus using the default implementation
// of the summarize() function.
template <typename Base, typename, typename>
struct summary_iface_impl : Base {
};

// Trait definition.
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

// Implement the summary trait for news_article and tweet.
template <typename Base, typename Holder, typename T>
    requires std::same_as<tanuki::unwrap_cvref_t<T>, news_article>
struct summary_iface_impl<Base, Holder, T> : Base {
    [[nodiscard]] std::string summarize() const final
    {
        return getval<Holder>(this).headline + ", by " + getval<Holder>(this).author + " ("
               + getval<Holder>(this).location + ")";
    }
};

template <typename Base, typename Holder, typename T>
    requires std::same_as<tanuki::unwrap_cvref_t<T>, tweet>
struct summary_iface_impl<Base, Holder, T> : Base {
    [[nodiscard]] std::string summarize() const final
    {
        return getval<Holder>(this).username + ": " + getval<Holder>(this).content;
    }
};

// Definition of the wrapper.
using summary = tanuki::wrap<summary_iface, tanuki::config<>{.explicit_ctor = tanuki::wrap_ctor::always_implicit}>;

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

template <typename Base, typename, typename>
struct fooable_iface_impl : Base {
};

// NOLINTNEXTLINE
struct fooable_iface {
    virtual ~fooable_iface() = default;
    [[nodiscard]] virtual std::string foo() const
    {
        return "default foo!";
    }

    template <typename Base, typename Holder, typename T>
    using impl = fooable_iface_impl<Base, Holder, T>;
};

struct foo_capable {
    std::string foo;
};

// Implement the fooable trait for foo_capable.
template <typename Base, typename Holder, typename T>
    requires std::same_as<tanuki::unwrap_cvref_t<T>, foo_capable>
struct fooable_iface_impl<Base, Holder, T> : Base {
    [[nodiscard]] std::string foo() const final
    {
        return getval<Holder>(this).foo;
    }
};

// fooable wrap.
using fooable = tanuki::wrap<fooable_iface, tanuki::config<>{.explicit_ctor = tanuki::wrap_ctor::always_implicit}>;

// Composite wrap.
using fooable_summary = tanuki::wrap<tanuki::composite_iface<summary_iface, fooable_iface>,
                                     tanuki::config<>{.explicit_ctor = tanuki::wrap_ctor::always_implicit}>;

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
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-virtual-inheritance)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
