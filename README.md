tanuki
======

[![Build Status](https://img.shields.io/github/actions/workflow/status/bluescarni/tanuki/gha.yml?branch=main&style=for-the-badge)](https://github.com/bluescarni/tanuki/actions?query=workflow%3A%22GitHub+CI%22)
[![Build Status](https://img.shields.io/circleci/project/github/bluescarni/tanuki/main.svg?style=for-the-badge)](https://circleci.com/gh/bluescarni/tanuki)
![language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg?style=for-the-badge)
[![Code Coverage](https://img.shields.io/codecov/c/github/bluescarni/tanuki.svg?style=for-the-badge)](https://codecov.io/github/bluescarni/tanuki?branch=main)

<!-- PROJECT LOGO -->
<br />
<p align="center">
  <p align="center">
    A type-erasure library for C++20
    <br />
    <a href="https://bluescarni.github.io/tanuki/index.html"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/bluescarni/tanuki/issues/new/choose">Report bug</a>
    ·
    <a href="https://github.com/bluescarni/tanuki/issues/new/choose">Request feature</a>
    ·
    <a href="https://github.com/bluescarni/tanuki/discussions">Discuss</a>
  </p>
</p>

> [tanukis](https://en.wikipedia.org/wiki/Japanese_raccoon_dog) are a kind of supernatural beings found in the
> classics and in the folklore and legends of various places in Japan. They are reputed to be mischievous and jolly,
> masters of disguise and shapeshifting but somewhat gullible and absent-minded.

tanuki is a small, single-header and self-contained C++20 toolkit for
[type-erasure](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Type_Erasure).
Main features include:

- high configurability,
- support for both value and reference semantics,
- small object optimisation to avoid dynamic memory allocation,
- the ability to type-erase references,
- support for composite interfaces,
- optional support for [Boost.serialization](https://www.boost.org/doc/libs/release/libs/serialization/doc/index.html).

Interfaces are defined and implemented blending the familiar language of
traditional object-oriented programming (i.e., abstract base clasess,
default implementations, single/multiple inheritance, etc.) with C++20 concepts.

The elevator pitch
------------------

Here is a minimal approximation of [std::any](https://en.cppreference.com/w/cpp/utility/any)
in tanuki:

```c++
#include <string>

#include <tanuki/tanuki.hpp>

// Interface implementation.
template <typename Base, typename Holder, typename T>
struct any_iface_impl : public Base {
};

// Interface.
struct any_iface {
    virtual ~any_iface() = default;

    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

int main()
{
    using any_wrap = tanuki::wrap<any_iface>;

    // Store an integer.
    any_wrap w1(42);

    // Store a string.
    any_wrap w2(std::string("hello world"));

    // Store anything...
    struct foo {
    };
    any_wrap w3(foo{});
}
```

[Try it](https://godbolt.org/z/T3r6eoafT) on [compiler explorer](https://godbolt.org/)!

Documentation
-------------

The full documentation can be found [here](https://bluescarni.github.io/tanuki).
