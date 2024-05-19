.. _hello_world:

.. cpp:namespace-push:: tanuki

Hello world
===========

Consider the shortest possible interface that one can write in traditional
`C++ object-oriented <https://en.wikipedia.org/wiki/Object-oriented_programming>`__ (OO)
style:

.. literalinclude:: ../tutorial/hello_world.cpp
   :language: c++
   :lines: 11-12,16

Calling ``any_iface`` an *interface* is quite a stretch, as the only operation
it supports is destruction -- but for now this will suffice.

In OO programming, we would then proceed to define implementations
of the ``any_iface`` interface via inheritance:

.. literalinclude:: ../tutorial/hello_world.cpp
   :language: c++
   :lines: 18-27

We would then be able to construct instances of ``any1`` and ``any2`` and interact
with them polymorphically via pointers to ``any_iface``:

.. literalinclude:: ../tutorial/hello_world.cpp
   :language: c++
   :lines: 31-33

Although there is nothing intrinsically wrong with traditional C++ OO programming,
even this short and silly example shows potential drawbacks with this programming style:

- it enforces dynamic memory allocation,
- it introduces hierarchical coupling (i.e., in order to implement
  an interface a class has to derive from it),
- it enforces pointer semantics.

In recent years, an alternative approach which avoids these drawbacks -
and which is known under several monikers such
as `type-erasure <https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Type_Erasure>`__,
`runtime polymorphism <https://github.com/ldionne/dyno>`__,
`virtual concepts <https://github.com/andyprowl/virtual-concepts>`__,
`traits <https://doc.rust-lang.org/book/ch17-02-trait-objects.html>`__, etc. - has become
increasingly popular. Examples of type-erased classes in the C++ standard library include
`std::any <https://en.cppreference.com/w/cpp/utility/any>`__,
`std::function <https://en.cppreference.com/w/cpp/utility/functional/function>`__ and
`std::thread <https://en.cppreference.com/w/cpp/thread/thread>`__.

Let us see how tanuki can be used to implement a type-erased analogue to the ``any_iface``
interface. First, we define a generic implementation of the interface:

.. literalinclude:: ../tutorial/hello_world.cpp
   :language: c++
   :lines: 7-9

Let us ignore for the moment the ``Holder`` and ``T`` template parameters (their meaning will be
explained :ref:`later <simple_interface>`), and note how an implementation **must** always derive from its ``Base``.
Behind the scenes, tanuki will ensure that ``Base`` derives from ``any_iface``.

Aside from the additional funky template arguments and from the fact that it does not derive
*directly* from ``any_iface``, ``any_iface_impl`` looks very similar to a traditional
OO programming interface implementation.

Second, we add to the ``any_iface`` definition an ``impl`` template alias to indicate that
``any_iface_impl`` is the interface implementation:

.. literalinclude:: ../tutorial/hello_world.cpp
   :language: c++
   :emphasize-lines: 4-5
   :lines: 11-16

.. note::

   tanuki interfaces do not need to have a ``virtual`` destructor, because tanuki
   never deletes through pointers to the interfaces. In this initial example, we leave
   the ``virtual`` in order to highlight that it is possible to adapt existing OO interfaces
   to work with tanuki, but in the following tutorials we will never declare ``virtual``
   destructors.

Note that this is an *intrusive* way of specifying the implementation of an interface.
A :ref:`non-intrusive alternative <nonintrusive>` is also available, so that it is possible to provide
implementations for existing interfaces without modifying them.

And we are done! We can now use ``any_iface`` in the definition of a type-erased
:cpp:class:`wrap` that can store **any** destructible object:

.. literalinclude:: ../tutorial/hello_world.cpp
   :language: c++
   :lines: 35-47

Although this code looks superficially similar to the OO-style approach, there are a few
key differences:

- no dynamic memory allocation is enforced: the :cpp:class:`wrap` class employs
  an :ref:`optimisation <custom_storage>` that stores small values inline;
- there is no hierarchical coupling: objects of any destructible class can be
  stored in an ``any_wrap`` without the need to inherit from anything;
- ``any_wrap`` employs (by default) value semantics (that is, copy/move/swap operations
  on a :cpp:class:`wrap` will result in copying/moving/swapping the internal
  value).

And that's it for the most minimal example!

Full code listing
-----------------
   
.. literalinclude:: ../tutorial/hello_world.cpp
    :language: c++
