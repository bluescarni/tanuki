.. _ref_interface:

.. cpp:namespace-push:: tanuki

Reference vs pointer interface
==============================

In the :ref:`previous tutorial <simple_interface>` we showed several ways
in which type-erased interfaces can be implemented in tanuki. We also showed
how the interface member functions can be invoked directly from the
:cpp:class:`wrap` class via the arrow operator ``->``:

.. code-block:: c++

   using foo1_wrap = tanuki::wrap<foo1_iface>;

   foo1_wrap w1(foo_model{});

   // Invoke the interface's foo() member function
   // directly from the wrap class.
   w1->foo();

Relying on the arrow operator ``->`` to access the member functions can however
be problematic.

To begin with, the arrow operator ``->`` is usually associated with pointers, but, as
we mentioned :ref:`previously <hello_world>`, the :cpp:class:`wrap` class employs
*value* semantics and not pointer semantics. Although the usage of the arrow operator
with value semantics is not unprecedented (e.g., see
`std::optional <https://en.cppreference.com/w/cpp/utility/optional>`__), it may be a source
of confusion.

Secondly, and more importantly, in generic algorithms and data structures it is often required
to invoke member functions via the dot operator ``.``, rather than via the arrow operator.
For instance, if we wanted to write the type-erased version of a standard
`range <https://en.cppreference.com/w/cpp/utility/optional>`__, we would need to be able
to access the begin/end functions as ``.begin()`` and ``.end()``, rather than
``->begin()`` and ``->end()``.

The not-so-good news is that at this time the C++ language offers no clean solution to automatically
enable dot-style access to the member functions of the interface from the :cpp:class:`wrap` class. This would require
`dot operator overloading <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4477.pdf>`__
or perhaps `reflection/metaclasses <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2996r0.html>`__,
neither of which are currently available.

The better news is that tanuki provides a mechanism to deactivate the pointer interface (i.e., operator ``->``)
and activate a *reference interface* instead (i.e., dot-style access to the member functions). Implementing
a reference interface will require a certain amount of
`repetition <https://en.wikipedia.org/wiki/Don%27t_repeat_yourself>`__ that can be partially alleviated
via the use of macros.

The following sections explain how to implement reference interfaces. Two APIs can be used: the first one
is always available, the second one is a cleaner and less verbose alternative that however requires
C++23.

Reference interfaces in C++20
-----------------------------

Consider the simple interface from the :ref:`previous tutorial <simple_interface>`, its implementation
and a ``foo_model`` class:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 5-23

Let us see first a macro-based implementation of a reference interface:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 25-30

A reference interface must be a class which defines in its scope an ``impl`` class
template depending exactly on one parameter, conventionally named ``Wrap``.
The ``impl`` class is the actual reference interface, and the ``Wrap`` parameter
represents the :cpp:class:`wrap` class to which the reference interface is applied.
``impl`` will be used as a static `mixin <https://en.wikipedia.org/wiki/Mixin>`__
to augment the :cpp:class:`wrap` class.

In the body of the ``impl`` class, we should declare and implement the list of member
functions of ``foo_iface`` that we want to be able to access via the dot operator in the
:cpp:class:`wrap` class. This is most easily accomplished with the help of the
:c:macro:`TANUKI_REF_IFACE_MEMFUN` macro, which requires only the name of the member function
(``foo``, in this specific case).

In order to understand what is going on behind the scenes, let us also see an example of
an equivalent reference interface which does **not** use the :c:macro:`TANUKI_REF_IFACE_MEMFUN` macro:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 32-40

Here is what is going on: tanuki makes the :cpp:class:`wrap` class inherit from ``foo_ref_iface2::impl``,
so that, via the `curiously recurring template pattern (CRTP) <https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`__,
we can invoke the :cpp:func:`~wrap::iface_ptr()` function on the :cpp:class:`wrap` object
to access a pointer to the ``foo_iface`` interface, via which we finally invoke the ``foo()``
member function. Phew!

The :c:macro:`TANUKI_REF_IFACE_MEMFUN` macro works by defining several variadic overloads for the
``foo()`` member function, automatically disabling the invalid ones via
`expression SFINAE <https://en.cppreference.com/w/cpp/language/sfinae>`__.
Although :c:macro:`TANUKI_REF_IFACE_MEMFUN` is not perfect (e.g., it cannot
be used with certain overloadable operators), you should consider using it if you can
stomach macros.

After the definition of the reference interfaces, we need to configure the :cpp:class:`wrap` class
to make use of them. This is accomplished by defining custom :cpp:class:`config` instances and using
them in the :cpp:class:`wrap` class. For instance, for the macro-based reference interface we
first define a custom configuration called ``foo_config1``:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 42

The :cpp:class:`config` class is templated over two types. Ignoring the first one for the time
being (its meaning will be explained :ref:`later <def_ctor>`), the second parameter is the reference interface, which
we set to ``foo_ref_iface1`` to select the macro-based reference interface. We also switch off
the pointer interface in :cpp:class:`wrap` by setting to ``false`` the :cpp:var:`~config::pointer_interface`
:ref:`configuration setting <config_settings>` -- this will ensure that access to the interface
functions via the arrow operator is disabled.

We can now use the custom configuration instance in the definition of the wrap class:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 68

And we can confirm that indeed we can now invoke the ``foo()`` member function via the dot operator:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 70-71

.. code-block:: console

   foo_iface_impl calling foo()

We can use the non-macro-based ``foo_ref_iface2`` reference interface in exactly the same way.
First, we define a custom configuration instance:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 44

Second, we can use the custom configuration instance to define another wrap type:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 73-76

.. code-block:: console

   foo_iface_impl calling foo()

.. _ref_interface23:

Reference interfaces in C++23
-----------------------------

An alternative API for the definition of reference interfaces is available if your compiler
supports C++23 (more specifically, the so-called
"`deducing this <https://en.cppreference.com/w/cpp/language/member_functions#Explicit_object_member_functions>`__"
feature). With the alternative API, it is not necessary any more to define a nested ``impl``
template, and we can define the member function wrappers directly in the body of the
class instead, like this:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 48-50

Note that, in the C++23 API, we must use the :c:macro:`TANUKI_REF_IFACE_MEMFUN2` macro, rather
than :c:macro:`TANUKI_REF_IFACE_MEMFUN`.

If you cannot or do not want to use the :c:macro:`TANUKI_REF_IFACE_MEMFUN2` macro, here is the implementation
of a C++23 reference interface without macros:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 52-58

In other words, the "deducing this" C++23 feature allows us to avoid having to perform the CRTP cast
manually, and we can invoke the :cpp:func:`~wrap::iface_ptr()` function directly on the ``self``/``this`` argument
instead.

We can then proceed as usual with the definition of the custom configurations using the
new reference interfaces:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 60-62

The definition and usage of the :cpp:class:`wrap` instances is identical to the C++20 API:

.. literalinclude:: ../tutorial/reference_interface.cpp
   :language: c++
   :lines: 80-88

.. code-block:: console

   foo_iface_impl calling foo()
   foo_iface_impl calling foo()

Note that the C++20 reference interface API is still available when using C++23 -- tanuki will detect whether
or not the nested template ``impl`` is present and will auto-select the API accordingly.

Tips & tricks
-------------

It should be noted that reference interfaces are useful beyond just enabling dot-style
access to the member functions. For instance, they also allow to define nested types and inline friend functions and operators
that will be accessible via `ADL <https://en.cppreference.com/w/cpp/language/adl>`__.

On the other hand, it is also possible to design an API around the :cpp:class:`wrap` class which does not
employ member functions and which thus needs neither a pointer nor a reference interface. In this usage
mode, the member functions defined in a type-erased interface are used as an implementation detail for free functions,
rather than being invoked directly.

Full code listing
-----------------

.. literalinclude:: ../tutorial/reference_interface.cpp
    :language: c++
