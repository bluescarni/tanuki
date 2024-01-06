.. tanuki documentation master file, created by
   sphinx-quickstart on Tue Jan  2 15:35:50 2024.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

tanuki
======

    `tanukis <https://en.wikipedia.org/wiki/Japanese_raccoon_dog>`__ are a kind of supernatural beings found in the
    classics and in the folklore and legends of various places in Japan. They are reputed to be mischievous and jolly,
    masters of disguise and shapeshifting but somewhat gullible and absent-minded.

tanuki is a small, single-header and self-contained C++20 library for
`type-erasure <https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Type_Erasure>`__.
Main features include:

- high configurability,
- support for both value and reference semantics,
- small object optimisation to avoid dynamic memory allocation,
- the ability to type-erase references,
- support for composite interfaces,
- optional support for `Boost.serialization <https://www.boost.org/doc/libs/release/libs/serialization/doc/index.html>`__.

.. toctree::
   :maxdepth: 1

   tutorials.rst
   api_reference.rst
