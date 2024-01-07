.. _wrap_reference:

.. cpp:namespace-push:: tanuki

Type-erasure for references
===========================

In all the examples we have seen so far, :cpp:class:`wrap` objects were constructed
by copying/moving values. It is however also possible
to construct :cpp:class:`wrap` objects that contain references to existing values,
rather than copies of the values. References are stored in a :cpp:class:`wrap` via
`std::reference_wrapper <https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper>`__, and,
with a small additional effort, it is possible
to write interface implementations which work seamlessly with both values and references.

Consider the following interface:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 31-38

This is similar to the interface considered in an :ref:`earlier tutorial <simple_interface>`.
We introduce two concepts to check the presence of the ``foo()`` and ``bar()`` member functions:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 10-14

And a ``foobar_model`` class implementing the ``foo()`` and ``bar()`` member functions:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 40-49

We then provide an empty default interface implementation:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 6-8

And here is the implementation for values *or references* providing ``foo()`` and ``bar()`` member functions:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 16-29

The only thing that has changed with respect to the :ref:`original example <simple_interface>` is that
the concept checks ``fooable`` and ``barable`` are not applied any more to ``T`` directly, but
rather to :cpp:type:`unwrap_cvref_t` of ``T``.

Here is what is going on: when a :cpp:class:`wrap` is constructed from
a `std::reference_wrapper <https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper>`__,
the :cpp:func:`iface_impl_helper::value()` functions will automatically unwrap the reference
wrapper and return a reference to the pointed-to value. This allows to use the same code
for the implementation of an interface, regardless of whether a copy or a reference is being
stored in the ``Holder``. Let us see a usage example:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 53-62

.. code-block:: console

   foobar_iface_impl calling foo()
   foobar_model calling foo()
   foobar_iface_impl calling bar()
   foobar_model calling bar()

Because the :cpp:class:`wrap` ``w2`` is constructed from ``std::ref(f)``, it will store a reference
to ``f`` rather than a copy. We can confirm that this is the case by comparing the address of ``f``
to the address of the object contained in ``w2``:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 64-66

.. code-block:: console

   f points to              : 0x7f7a0a000030
   The value in w2 points to: 0x7f7a0a000030

Note that, just like regular references, ``w2`` is a non-owning view of ``f``: it will remain
valid as long as the ``f`` object is alive, and it is your responsibility to ensure that ``w2``
is not used after the destruction of ``f``.

A caveat about const references
-------------------------------

Storing const references in a :cpp:class:`wrap` object is possible, but it come with a caveat:
if you try to invoke a non-const member function of the interface on a :cpp:class:`wrap` object
containing a const reference, then a ``std::runtime_error`` exception will be thrown by the
:cpp:func:`iface_impl_helper::value()` accessor:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 68-71

In order to prevent runtime errors, you should ensure that a :cpp:class:`wrap` containing a const
reference is itself declared as ``const``:

.. literalinclude:: ../tutorial/wrap_reference.cpp
   :language: c++
   :lines: 73-76

This way, the error will happen at compile time rather than at runtime.
