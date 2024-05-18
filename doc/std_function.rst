.. cpp:namespace-push:: tanuki

Implementing a ``std::function`` look-alike
===========================================

In this case study, we will be implementing an approximation of
``std::function`` with tanuki. This will not be a full drop-in
replacement for ``std::function``, because we aim to write an implementation which, contrary to ``std::function``
and similarly to `std::move_only_function <https://en.cppreference.com/w/cpp/utility/functional/move_only_function>`__,
correctly respects const-correctness. Additionally, our polymorphic function wrapper (which we will be naming ``callable``)
will also support wrapping references (thus providing functionality similar to
`std::function_ref <https://en.cppreference.com/w/cpp/utility/functional/function_ref>`__).

The interface and its implementation
------------------------------------

Let us begin by taking a look at the interface:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 27-34

Note that this interface depends on one or more template parameters:
the return type ``R`` and the argument type(s) ``Args``.
The two member functions in the interface are the call operator and the conversion
operator to ``bool``, which will be used to detect the empty state.
Note that the call operator is marked ``const`` - our objective
in this example is to implement an *immutable* function wrapper.

The default implementation of the interface is left empty (so that, as explained
in :ref:`the tutorial <simple_interface>`, this will be an invalid implementation):

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 23-25

Note that the ``R`` and ``Args`` template arguments are appended after the
mandatory ``Base``, ``Holder`` and ``T`` arguments.

Next, we get to the "real" interface implementation. Let us parse it
a bit at a time, beginning with the declaration:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 36-38

We can see that the ``requires`` clause enables this implementation for all
value (or :ref:`reference <wrap_reference>`) types which are
`invocable <https://en.cppreference.com/w/cpp/types/is_invocable>`__ and
`copy-constructible <https://en.cppreference.com/w/cpp/concepts/copy_constructible>`__.
As mentioned earlier, our objective is to implement an immutable function wrapper,
and thus we check for invocability on a ``const`` reference to the value type.
That is, if the value type provides non-``const`` call operator, the implementation
will be disabled.

Next, we see how the call operator is implemented:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 39-55

The call operator will ultimately invoke (via `std::invoke <https://en.cppreference.com/w/cpp/utility/functional/invoke>`__)
the type-erased value with the supplied arguments ``args`` (possibly going through a cast to ``void`` in order
to emulate C++23's `std::invoke_r() <https://en.cppreference.com/w/cpp/utility/functional/invoke>`__).
Before doing that, however, we must check that the object we are invoking is not a null pointer to
a (member) function - if that is the case, we will be raising an exception rather than incurring
in undefined behaviour.

Let us take a look at the ``bool`` conversion operator now:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 57-68

This operator must detect if the internal type-erased value is in an empty state - in which
case the operator will return ``false``. What an "empty state" is depends on
the value type:

- if the value is a pointer, then the empty state is represented by a ``nullptr``;
- if the value is a ``std::function`` or a ``callable`` (yes, we can put
  type-erased containers inside other type-erased containers,
  `Russian doll <https://en.wikipedia.org/wiki/Matryoshka_doll>`__-style),
  then the empty state is detected by invoking the value's ``bool`` conversion
  operator [#f1]_;
- otherwise, we assume the internal value is not in the empty state.

.. rubric:: Footnotes

.. [#f1] See the :ref:`full code listing <std_func_code_listing>` for the implementation of the
   type traits detecting instances of ``std::function`` and ``callable``.

The reference interface
-----------------------

We can now move on to the implementation of the :ref:`reference interface <ref_interface>`.
The reference interface for our ``callable`` class will have to implement a few bits of additional logic,
and thus we cannot simply use the :c:macro:`TANUKI_REF_IFACE_MEMFUN` macro to just forward
the API calls into the interface.

Let us examine it a bit at a time:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 71-76

The first thing we do is to provide a ``result_type`` alias for the return type ``R``. We do this in order
to emulate the behaviour of ``std::function`` (which also provides such an alias) and to show that the reference
interface, in addition to providing member functions, can also provide type aliases (remember that
:cpp:class:`wrap` will be inheriting from the reference interface).

Next, we take a look at the call operator:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 78-88

There is quite a bit going on here visually, but the basic gist is this: before invoking the call operator of the interface,
we need to ensure that the ``callable`` object is not in the :ref:`invalid state <invalid_state>` (this could
happen, for instance, on a moved-from ``callable``). If the ``callable`` is in the invalid state, we
will raise an error.

The implementation of the call operator of the reference interface employs, as usual, the
`CRTP <https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`__ to reach
the pointer to the interface via the :cpp:func:`~wrap::iface_ptr()` function, which is then used to invoke
the interface's call operator. The arguments are perfectly forwarded, and expression SFINAE is used
(via the trailing ``decltype(...)``) to disable the call operator if it is malformed.

Next, we take a look at the ``bool`` conversion operator:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 90-98

The idea here is the same: if the ``callable`` object is in the :ref:`invalid state <invalid_state>`, then
the ``bool`` conversion operator will return ``false``. Otherwise, the ``bool`` conversion operator of
the interface will be invoked.

In other words, with this implementation of the reference interface, we will be able to detect
if a ``callable`` is empty either because the type-erased value is empty or the wrapping object
is in the :ref:`invalid state <invalid_state>`.

Configuration
-------------

Let us move on now to the configuration of the ``callable`` wrap:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 102-108

The configuration object includes the following custom settings:

- the function pointer type ``R (*)(Args...)`` is used as default value type - this means that
  a default-initialised ``callable`` will contain a null function pointer. This matches the
  behaviour of ``std::function``, which default-initialises to an empty object;
- the reference interface is set to ``callable_ref_iface``;
- the :cpp:var:`config::static_size` and :cpp:var:`config::static_align` settings are set up so
  that the ``callable`` wrap is guaranteed to be able to store pointers (and smaller objects)
  in static storage. This again matches the guarantees of ``std::function``;
- the pointer interface is disabled via :cpp:var:`config::pointer_interface`;
- the generic constructors of ``callable`` are marked as implicit, again to in order to match
  the behaviour of ``std::function``.

We are now ready to define our ``callable`` wrap:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 110-112

Finally, we provide some syntactic sugar:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 119-131

We will now be able to take our ``callable`` out for a spin.

Sample usage
------------

Let us begin with a few ``callable`` instances constructed into the empty state:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 140-143

We can see that in all these cases the ``bool`` conversion operator of ``callable``
returns ``false``, as all these objects are constructed into the empty state either
by default construction or by construction from an empty object (e.g., a null function pointer).

Next, let us see an example with a lambda:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 145-147

Here, a copy of ``lambda_double`` is stored in ``c0``. Let us see an example storing a reference
to ``lambda_double`` instead:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 149-151

The second ``assert()`` confirms that a reference to ``lambda_double`` (rather than a copy) has been
captured in ``c0_ref``.

Because our ``callable`` has been designed to be immutable, we cannot construct an instance
from a mutable lambda:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 153-157

Finally, an example with a plain old function:

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 133-136

.. literalinclude:: ../tutorial/std_function.cpp
   :language: c++
   :lines: 159-160

Thie example shows how :cpp:class:`wrap` supports construction directly from a function type,
without requiring conversion to a function pointer.

.. _std_func_code_listing:

Full code listing
-----------------

.. literalinclude:: ../tutorial/std_function.cpp
    :language: c++
