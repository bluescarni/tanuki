.. _ref_semantics:

.. cpp:namespace-push:: tanuki

Reference semantics
===================

In the :ref:`first tutorial <hello_world>`, we mentioned how one of the reasons
to prefer type-erasure over traditional object-oriented programming is that with
type-erasure we can implement value semantics,
rather than being forced into pointer/reference semantics.

There are however situations in which pointer/reference semantics might be preferrable.
For instance, the motivating example for the development of reference semantics in tanuki
was the necessity to support large computational graphs featuring a high degree
of internal repetition in the `heyoka library <https://github.com/bluescarni/heyoka>`__.

Reference semantics support in tanuki is activated by setting the :cpp:var:`config::semantics`
config option to :cpp:enumerator:`wrap_semantics::reference`. When reference semantics
is activated, the :cpp:class:`wrap` class will employ internally a
`shared pointer <https://en.cppreference.com/w/cpp/memory/shared_ptr>`__ to store the type-erased
value, and copy/move/swap operations on a :cpp:class:`wrap` will behave like the corresponding
operations on a shared pointer - that is, they will manipulate references to the internal value,
rather than the value itself.

Additionally, there are a couple of extra functions in the API that are available only when
using reference semantics.

The first one, :cpp:func:`wrap::copy()`, is used to make a deep-copy of a :cpp:class:`wrap`
(that is, it enforces the creation of a copy of the internal value, rather than creating
a new shared reference to it).

The second one, :cpp:func:`wrap::same_value()`, can be used to detect if two :cpp:class:`wrap`
objects share ownership of the same value.
