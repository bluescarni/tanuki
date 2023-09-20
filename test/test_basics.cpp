#include <functional>
#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iostream>

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace)

// NOLINTNEXTLINE
struct iface0 {
    virtual ~iface0() = default;
};

template <typename Holder>
struct iface0_impl : iface0 {
};

struct foo_iface {
    virtual ~foo_iface() = default;
    virtual void foo() const = 0;
};

struct bar_iface : foo_iface {
    ~bar_iface() override = default;
    virtual void bar() const = 0;
};

template <typename Holder, typename IFace>
struct foo_iface_impl : IFace {
    void foo() const override
    {
        static_cast<Holder const *>(this)->m_value.foo();
    }
};

template <typename Holder, typename IFace>
struct bar_iface_impl : IFace {
    void bar() const override
    {
        static_cast<Holder const *>(this)->m_value.bar();
    }
};

template <typename Holder>
using foobar_iface_impl = tanuki::composite_iface_impl<Holder, bar_iface, foo_iface_impl, bar_iface_impl>;

// ---------------------------------------------------------------------

struct frob_iface {
    virtual ~frob_iface() = default;
    virtual void frob() const = 0;
};

struct niz_iface {
    virtual ~niz_iface() = default;
    virtual void niz() const = 0;
};

struct frobniz_iface : frob_iface, niz_iface {
};

template <typename Holder, typename IFace>
struct frob_iface_impl : IFace {
    void frob() const override
    {
        static_cast<Holder const *>(this)->m_value.frob();
    }
};

template <typename Holder, typename IFace>
struct niz_iface_impl : IFace {
    void niz() const override
    {
        static_cast<Holder const *>(this)->m_value.niz();
    }
};

template <typename Holder>
using frobniz_iface_impl = tanuki::composite_iface_impl<Holder, frobniz_iface, frob_iface_impl, niz_iface_impl>;

using tanuki::wrap;

TEST_CASE("basics")
{
    struct blaf {
        char buffer[100];
    };

    wrap<iface0, iface0_impl> w1(3.), w2(blaf{}), w3(std::function<void()>{});

    auto w4 = w3;
    auto w5(std::move(w2));
    // NOLINTNEXTLINE(bugprone-use-after-move)
    // REQUIRE(w2.is_invalid());

    w1 = std::move(w3);

    w3 = wrap<iface0, iface0_impl>(std::function<void()>{});

    auto w1a = w1;
    w1 = w1a;

    auto w5a = wrap<iface0, iface0_impl>(blaf{});
    w5a = std::move(w5);

    w2 = wrap<iface0, iface0_impl>(blaf{});
    auto w2a = wrap<iface0, iface0_impl>(blaf{});
    w2 = w2a;
}

struct my_foobar {
    void foo() const
    {
        std::cout << "foo\n";
    };
    void bar() const
    {
        std::cout << "bar\n";
    };
};

struct my_frobniz {
    void frob() const
    {
        std::cout << "frob\n";
    };
    void niz() const
    {
        std::cout << "niz\n";
    };
};

void dooda(const bar_iface &) {}

void diida(const foo_iface &) {}

void daada(const frobniz_iface &) {}

void daadauda(const frob_iface &) {}

TEST_CASE("foo and bar")
{
    wrap<bar_iface, foobar_iface_impl> g{my_foobar{}};

    g->foo();
    g->bar();

    wrap<frobniz_iface, frobniz_iface_impl> h{my_frobniz{}};

    h->frob();
    h->niz();

    dooda(*g);
    diida(*g);
    daada(*h);
    daadauda(*h);

    static_cast<bar_iface *>(g);
}

TEST_CASE("dasda")
{
    wrap<iface0, iface0_impl, tanuki::config{.static_size = 0}> w1(4);
    auto w2 = std::move(w1);
    wrap<iface0, iface0_impl, tanuki::config{.static_size = 0}> w3(42.);

    using std::swap;

    swap(w2, w3);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace)
