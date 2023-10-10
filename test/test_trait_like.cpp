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
struct summary_iface<Holder, T> : virtual summary_iface<void, void>, tanuki::iface_impl_helper<Holder, T> {
    [[nodiscard]] std::string summarize() const final
    {
        return this->value().headline + ", by " + this->value().author + " (" + this->value().location + ")";
    }
};

template <typename Holder, typename T>
    requires tanuki::same_or_ref_for<T, tweet>
struct summary_iface<Holder, T> : virtual summary_iface<void, void>, tanuki::iface_impl_helper<Holder, T> {
    [[nodiscard]] std::string summarize() const final
    {
        return this->value().username + ": " + this->value().content;
    }
};

// Definition of the wrapper.
using summary = tanuki::wrap<summary_iface, tanuki::config<>{.explicit_generic_ctor = false}>;

// A function with summary input param.
void notify(const summary &s)
{
    std::cout << "Breaking news! " << s->summarize() << '\n';
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
}

template <typename, typename>
struct any_iface;

template <>
// NOLINTNEXTLINE
struct any_iface<void, void> {
    virtual ~any_iface() = default;
};

template <typename Holder, typename T>
struct any_iface : virtual any_iface<void, void> {
};

using whatever = tanuki::wrap<any_iface, tanuki::config<>{.explicit_generic_ctor = false}>;

TEST_CASE("blaf")
{
    using frip = tanuki::composite_wrap<summary, whatever>;

    // (void)static_cast<frip *>(nullptr);

    const frip f(tweet{});
}

// template <typename Wrap0, typename Wrap1, typename... WrapN>
// requires any_wrap<Wrap0> && any_wrap<Wrap1> && (any_wrap<WrapN> && ...)

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while,fuchsia-multiple-inheritance,fuchsia-virtual-inheritance)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
