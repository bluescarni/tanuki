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

Let us see a simple example of reference semantics in action. We begin with our usual, super-simple
interface and its implementation:

.. literalinclude:: ../tutorial/ref_semantics.cpp
   :language: c++
   :lines: 7-14

We introduce a :cpp:class:`wrap` type employing reference semantics:

.. literalinclude:: ../tutorial/ref_semantics.cpp
   :language: c++
   :lines: 18-18

And we create a :cpp:class:`wrap` instance storing a ``std::string``:

.. literalinclude:: ../tutorial/ref_semantics.cpp
   :language: c++
   :lines: 20-20

Let us now make a copy of ``w1``:

.. literalinclude:: ../tutorial/ref_semantics.cpp
   :language: c++
   :lines: 22-22

As explained earlier, this operation will not copy the string inside ``w1``, rather it will create a new reference to it.
We can confirm that this indeed is the case by comparing the addresses of the values stored in ``w1`` and ``w2``:

.. literalinclude:: ../tutorial/ref_semantics.cpp
   :language: c++
   :lines: 26-27

.. code-block:: console

   Address of the value wrapped by w1: 0x606000000038
   Address of the value wrapped by w2: 0x606000000038

We can also invoke the :cpp:func:`wrap::same_value()` function for further confirmation:

.. literalinclude:: ../tutorial/ref_semantics.cpp
   :language: c++
   :lines: 29-29

.. code-block:: console

   Do w1 and w2 share ownership? true

If, on the other hand, we use the :cpp:func:`wrap::copy()` function, we will be performing a copy
of the string stored in ``w1``:

.. literalinclude:: ../tutorial/ref_semantics.cpp
   :language: c++
   :lines: 31-36

.. code-block:: console

   Address of the value wrapped by w1: 0x606000000038
   Address of the value wrapped by w3: 0x606000000098
   Do w1 and w3 share ownership? false

Full code listing
-----------------

.. literalinclude:: ../tutorial/ref_semantics.cpp
    :language: c++
