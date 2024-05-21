Installation
============

tanuki currently consists of a self-contained single header, with no dependencies
apart from the standard library. Just grab the ``include/tanuki.hpp`` header and copy it
somewhere in your project.

Supported platforms/compilers
-----------------------------

tanuki requires a C++ compiler with decent support for several C++20 features.
Currently the following compilers are tested in the CI pipeline:

- GCC >= 10.4,
- clang >= 14,
- MSVC 2022.

The CI pipeline is run on several platforms, including x86-64/arm64 Linux, x86-64 OSX and Windows.
