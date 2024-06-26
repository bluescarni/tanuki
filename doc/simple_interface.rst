.. _simple_interface:

.. cpp:namespace-push:: tanuki

A simple interface
==================

The minimal example shown in the :ref:`previous tutorial <hello_world>` was not very
interesting, as the interface we implemented did not define any behaviour other than
destruction. In this tutorial, we are going to take the next step and implement
a slightly more useful interface consisting of a single member function called ``foo()``.

Take 1: The basics
------------------

Here is the first version of our interface:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 14-19

Here is the declaration of a tanuki implementation for this interface:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 5-6

As explained in the :ref:`previous tutorial <hello_world>`, an interface implementation
must always derive from its ``Base`` template parameter. Additionally, we must provide
an implementation for the ``void foo() const`` function:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 5-12

The ``Holder`` template parameter is a class defined in the tanuki library which stores the
value we are type-erasing as the ``_tanuki_value`` data member. ``Holder`` derives from ``foo1_iface_impl``
and thus we can reach the ``_tanuki_value`` data member via the cast
``static_cast<const Holder *>(this)`` leveraging the
`curiously recurring template pattern (CRTP) <https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`__.
In other words, this implementation of the ``void foo() const`` function will invoke
the ``foo()`` member function of the type-erased value.

We can now proceed to define a class providing such a ``foo()`` member function:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 21-26

We are now ready to define and use a type-erased wrapper:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 95-98

This code will produce the following output:

.. code-block:: console

   foo1_iface_impl calling foo()
   foo_model calling foo()

That is, the ``foo1_wrap`` wrapper provides access, via the arrow operator ``->``,
to the ``foo1_iface_impl::foo()`` member function, which in turn invokes
``foo_model::foo()``.

.. _getval_intro:

Take 2: Introducing ``getval()``
--------------------------------

The use of the CRTP in the interface implementation can be a bit verbose and ugly.
tanuki provides a couple of helpers, called :cpp:func:`getval()`,
which can help reducing typing in an interface implementation. Let us see them
in action with a second version of the interface implementation:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 28-35

Here we have replaced the explicit ``static_cast`` with an invocation of :cpp:func:`getval()`,
which, behind the scenes, performs the CRTP downcast and returns a (const) reference to the
type-erased value.

Note that :cpp:func:`getval()` does a bit more than providing access to the type-erased value, as it will be explained
in a :ref:`later tutorial <wrap_reference>`. In any case, from now on, all interface implementations
shown in these tutorials will make use of :cpp:func:`getval()` in order to reduce typing.

Let us see the new interface implementation in action:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 100-103

.. code-block:: console

   foo2_iface_impl calling foo()
   foo_model calling foo()

Take 3: Concept checking
------------------------

What happens though if we try to construct a ``foo2_wrap`` from an object which does **not** provide a ``foo()`` member
function (such as, e.g., an ``int``)? The compiler will loudly complain:

.. code-block:: console

   simple_interface.cpp:34:23: error: request for member ‘foo’ in [...] which is of non-class type ‘const int’
   34 |         _tanuki_value.foo();

This is of course not ideal, for at least a couple of reasons:

- the error is generated deep in the bowels of the interface implementation, it would be better if we
  could catch the problem earlier and provide a more meaningful error message;
- we might want to provide an alternative implementation of the interface for value types that do **not**
  have a ``foo()`` member function.

`C++20 concepts <https://en.cppreference.com/w/cpp/language/constraints>`__ are a powerful tool
that allows us to address both of these issues in more than one way.

As a first approach, we are going to constrain our interface implementation to types that
provide a ``foo()`` member function. In order to do this, we first introduce a ``fooable`` concept:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 44-45

This concept will be satisfied by all types which provide a const ``foo()`` member function.
We then constrain the interface implementation to ``fooable`` types via the ``requires``
keyword:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 47-55

That is, the third template parameter ``T`` in a tanuki interface implementation represents the type of the
value stored in the ``Holder`` class, and we can thus use it to specialise and/or constrain interface implementations.
Here is the new interface implementation in action:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 105-108

.. code-block:: console

   foo3_iface_impl calling foo()
   foo_model calling foo()

``foo3_iface_impl`` is similar to what Rust calls a
`blanket implementation <https://doc.rust-lang.org/book/ch10-02-traits.html#using-trait-bounds-to-conditionally-implement-methods>`__.

What happens now if we try to construct a ``foo3_wrap`` from an ``int``? The error message given by the compiler changes:

.. code-block:: console

   simple_interface.cpp:84:21: error: no matching function for call to ‘tanuki::v1::wrap<foo3_iface>::wrap(int)’
   [...]
   1073 |         wrap(T &&x) noexcept(noexcept(this->ctor_impl<detail::value_t_from_arg<T &&>>(std::forward<T>(x)))
        |         ^~~~
   tanuki.hpp:1073:9: note:   template argument deduction/substitution failed:
   tanuki.hpp:1073:9: note: constraints not satisfied

That is, the constructor of the :cpp:class:`wrap` class now detects that ``int`` does not have an
interface implementation, and as a consequence the compiler detects an error *before* trying to
invoke the (non-existing) ``foo()`` member function on the ``int``. We can confirm
that the non-constructibility of ``foo3_wrap`` from ``int`` is detected at compile time
by checking the ``std::is_constructible`` type trait:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 110-111

.. code-block:: console

   Is foo3_wrap constructible from an int? false

This is all fine and dandy, but what if we wanted to provide an implementation of our interface also for ``int``? We can do this
with yet another (and final) variation on the theme.

Take 4: Empty implementations
-----------------------------

First off, we begin with an **empty** interface implementation:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 64-66

This is an **invalid** implementation because it does not derive from ``Base``.
Second, we add a constrained specialisation of the interface implementation
for ``fooable`` types (that is, types providing a const ``foo()`` member function):

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 68-76

Finally, we add a specialisation of ``foo4_iface_impl`` for ``int``:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 78-84

This implementation will just print to screen the value of the integer
when the ``foo()`` function is invoked.

Let use see the new interface implementation in action:

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 113-118

.. code-block:: console

   foo4_iface_impl calling foo()
   foo_model calling foo()
   foo4_iface_impl implementing foo() for the integer 42

It works! Bu what happens if we try to construct a ``foo4_wrap`` from an object that is neither
``fooable`` nor an ``int``? The :cpp:class:`wrap` class will detect
that the interface implementation corresponding to an object of such type is empty (i.e., invalid),
and it will thus disable the constructor. We can confirm that this is the case by checking
the constructibility of ``foo4_wrap`` from a ``float`` (which is neither
``fooable`` nor an ``int``):

.. literalinclude:: ../tutorial/simple_interface.cpp
   :language: c++
   :lines: 120

.. code-block:: console

   Is foo4_wrap constructible from a float? false

And that is enough for now!

Full code listing
-----------------

.. literalinclude:: ../tutorial/simple_interface.cpp
    :language: c++
