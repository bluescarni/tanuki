Implementing ``std::function``
==============================

In this case study, we will be implementing (an approximation of)
``std::function`` with tanuki. This will not be a full drop-in
replacement for ``std::function``, for a couple of reason:

- there is a (small) part of the ``std::function`` API which cannot
  currently be implemented with tanuki,
- we aim to write an implementation which, contrary to ``std::function``
  and similarly to `std::move_only_function <https://en.cppreference.com/w/cpp/utility/functional/move_only_function>`__,
  correctly respects const-correctness.

The interface
-------------

Let us begin by taking a look at the interface:

.. literalinclude:: ../test/std_function.cpp
   :language: c++
   :lines: 32,34-41

Note that this interface depends on one or more template parameters:
the return type and the argument type(s).

The two member functions in the interface are the call operator and the conversion
operator to ``bool``, which will be used to detect the empty state
of the function.

The default implementation of the interface is left empty (so that, as explained
in :ref:`the tutorial <simple_interface>`, this will be an invalid implementation):

.. literalinclude:: ../test/std_function.cpp
   :language: c++
   :lines: 28-30

Next, we get to the "real" interface implementation. Let us parse it
a bit at a time, beginning with the declaration:

.. literalinclude:: ../test/std_function.cpp
   :language: c++
   :lines: 43-45

We can see that the ``requires`` clause enables this implementation for all
value types (or :ref:`references <wrap_reference>`) which are
`invocable <https://en.cppreference.com/w/cpp/types/is_invocable>`__.

Next, we see how the call operator is implemented:

.. literalinclude:: ../test/std_function.cpp
   :language: c++
   :lines: 46-69

The call operator of ``std::function`` must do two things:

- check that we the contained value is in a usable state,
- invoke the contained value with the input arguments.

The internal value could be unusable in the following case:

- it is a null pointer to a (member) function,
- it is a :cpp:class:`wrap` in the :ref:`invalid state <invalid_state>`,
- it is an empty ``std::function``.

We use several type traits and concepts in conjunction with ``if constexpr``
in order to detect unusable values (in which case, we throw a ``std::bad_function_call``
just like ``std::function``).
