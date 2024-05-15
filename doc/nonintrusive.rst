.. _nonintrusive:

.. cpp:namespace-push:: tanuki

Non-intrusive interface implementations
=======================================

In all the examples seen so far, implementations for tanuki interfaces have always
been specified by adding an ``impl`` template alias in the body of the interface class.
This is an *intrusive* approach, in the sense that it requires modifying the definition
of the interface.

In order to be able to adapt existing object-oriented interfaces without having to modify them
by adding the ``impl`` alias, tanuki also supports a non-intrusive way of specifying
interface implementations. Let us see it in action.

Consider the following object-oriented interface ``my_iface`` defined in some namespace ``ns``:

.. literalinclude:: ../tutorial/nonintrusive.cpp
   :language: c++
   :lines: 6-15

In order to provide a non-intrusive tanuki implementation for ``my_iface``, we need to implement
a partial specialisation of the :cpp:struct:`iface_impl` struct for ``my_iface``:

.. literalinclude:: ../tutorial/nonintrusive.cpp
   :language: c++
   :lines: 17-29

Here we are specifying an implementation for all value types ``T``, but, as explained
in previous tutorials, we could also easily provide a partially-specialised implementation,
constrain the implementation only for value types modelling
certain requirements, provide an empty default implementation, etc.

We are now able to wrap ``my_iface`` in the usual way:

.. literalinclude:: ../tutorial/nonintrusive.cpp
   :language: c++
   :lines: 31-40

.. code-block:: console

   The final answer is 42

Full code listing
-----------------

.. literalinclude:: ../tutorial/nonintrusive.cpp
    :language: c++
