.. _composite_interfaces:

.. cpp:namespace-push:: tanuki

Composing interfaces
====================

tanuki offers the possibility of combining multiple (potentially unrelated) interfaces
into a single one. This can be done in (at least) two ways:

- either via the easy-to-use :cpp:class:`composite_iface` class, or
- by manually composing interfaces via inheritance.

Automatic composition via :cpp:class:`composite_iface`
------------------------------------------------------

Consider the following two simple interfaces and their implementations:

.. literalinclude:: ../tutorial/compose1.cpp
   :language: c++
   :lines: 5-37

The two interfaces contain the ``foo()`` and ``bar()`` member functions respectively.
Their implementations will invoke the ``foo()`` and ``bar()`` member functions
on the type-erased values.

We can combine ``foo_iface`` and ``bar_iface`` into a single ``foobar_iface`` interface via
:cpp:class:`composite_iface`:

.. literalinclude:: ../tutorial/compose1.cpp
   :language: c++
   :lines: 52-53

Next, we define a type-erased wrapper for the composite interface:

.. literalinclude:: ../tutorial/compose1.cpp
   :language: c++
   :lines: 55-56

We can then introduce a type that satisfies both interfaces:

.. literalinclude:: ../tutorial/compose1.cpp
   :language: c++
   :lines: 39-48

And finally wrap it:

.. literalinclude:: ../tutorial/compose1.cpp
   :language: c++
   :lines: 58-63

.. code-block:: console

   Invoking foobar_model::foo()
   Invoking foobar_model::bar()

:cpp:class:`composite_iface` is convenient and it has the big advantage
of being able to compose interfaces which are unrelated to each other.
The main drawback of :cpp:class:`composite_iface` is that, because it uses
multiple inheritance behind the scenes, it has a memory footprint proportional
to the number of interfaces that it wraps. Roughly speaking, each interface
in a :cpp:class:`composite_iface` adds the size of a pointer to the memory
footprint of :cpp:class:`wrap`.

Full code listing
^^^^^^^^^^^^^^^^^

.. literalinclude:: ../tutorial/compose1.cpp
    :language: c++

Manual composition via inheritance
----------------------------------

If you want to avoid the memory overhead of :cpp:class:`composite_iface`, it is possible
to create composite interfaces via inheritance. This approach requires a bit more work
and it will introduce coupling between the interfaces. On the other hand, an inheritance-based
approach can be appropriate when modelling types which naturally fit in a hierarchy (e.g.,
standard iterators).

We begin with a ``foo_iface`` interface (and its implementation) identical to the previous example:

.. literalinclude:: ../tutorial/compose2.cpp
   :language: c++
   :lines: 5-20

The ``bar_iface`` interface and its implementation, however, are now different:

.. literalinclude:: ../tutorial/compose2.cpp
   :language: c++
   :lines: 22-36

``bar_iface`` and its implementation now inherit from ``foo_iface`` and its implementation respectively.
In other words, ``bar_iface`` now plays the role of a composite interface. Let us wrap it and show
its usage with the ``foobar_model`` class from the previous example:

.. literalinclude:: ../tutorial/compose2.cpp
   :language: c++
   :lines: 38-60

.. code-block:: console

   Invoking foobar_model::foo()
   Invoking foobar_model::bar()

By manually defining a hierarchy of single inheritances, we have now avoided the size overhead
of multiple inheritance.

Full code listing
^^^^^^^^^^^^^^^^^

.. literalinclude:: ../tutorial/compose2.cpp
    :language: c++
