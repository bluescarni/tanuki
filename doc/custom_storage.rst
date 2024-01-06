.. _custom_storage:

.. cpp:namespace-push:: tanuki

Storage customisation
=====================

In a :ref:`previous tutorial <hello_world>`, we mentioned briefly how the :cpp:class:`wrap` class
implements a small storage optimisation, but we left the specifics out. In this tutorial,
we will see how it is possible to customise the storage options for the :cpp:class:`wrap` class.

How small is small?
-------------------

The threshold size under which the :cpp:class:`wrap` class avoids dynamic memory allocation
is represented by the configuration parameter :cpp:var:`config::static_size`. The unit of measure,
as usual, is bytes.

The default value for :cpp:var:`config::static_size` is somewhat arbitrary, but it should ensure that
on most platforms it is possible to store in static storage pointers and structs with a few members.
If :cpp:var:`config::static_size` is set to zero, then the small-object optimisation is disabled and the
:cpp:class:`wrap` class always uses dynamically-allocated memory.

One very important thing to understand about :cpp:var:`config::static_size` is that it accounts
also for the type-erasure storage overhead. That is, if you specify a :cpp:var:`config::static_size`
of 24 bytes, some of those bytes will be taken up by the type-erasure machinery, and thus the
storage available for the type-erased value will be less than 24 bytes. How much less exactly
is dependent on a variety of factors and difficult to compute in advance.

For this reason, tanuki provides a :cpp:var:`holder_size` helper which can be used to compute
how much **total** storage is needed to store a value of a type ``T`` in a :cpp:class:`wrap`.
Let us a see a simple example.

We have our usual super-basic interface and its implementation:

.. literalinclude:: ../tutorial/custom_storage.cpp
   :language: c++
   :lines: 5-14

Let us say that we want to ensure that we can store in static storage objects which are up
to the size of a pointer. We can define a custom :cpp:class:`config` instance in which we
set the :cpp:var:`config::static_size` parameter to ``holder_size<void *, any_iface>``:

.. literalinclude:: ../tutorial/custom_storage.cpp
   :language: c++
   :lines: 16

This will ensure that value types whose size is up to ``sizeof(void *)`` can be stored
in static storage.

We can then define a :cpp:class:`wrap` class using the custom :cpp:class:`config` instance and
verify that indeed static storage is employed when the :cpp:class:`wrap` contains a pointer:

.. literalinclude:: ../tutorial/custom_storage.cpp
   :language: c++
   :lines: 20-25

.. code-block:: console

   Is w1 using static storage? true

On the other hand, if we try to store 2 pointers instead of 1, we can verify that the
:cpp:class:`wrap` class switches to dynamic storage:

.. literalinclude:: ../tutorial/custom_storage.cpp
   :language: c++
   :lines: 27-34

.. code-block:: console

   Is w2 using static storage? false

One limitation of the :cpp:var:`holder_size` helper is that it requires the interface
(i.e., the second parameter - ``any_iface`` in the example) to have an implementation
for the value type (i.e., the first parameter - ``void *`` in the example).

What about alignment?
---------------------

The alignment of the static storage within :cpp:class:`wrap` can be configured
via the :cpp:var:`config::static_align` config option. By default, the maximum
alignment supported on the platform (that is, ``alignof(std::max_align_t)``) is used.
This ensures that objects of any type can be stored in static storage (provided of
course that there is enough space).

In some cases, it may be convenient to specify a smaller :cpp:var:`config::static_align`
value in order to reduce the memory footprint of a :cpp:class:`wrap`. Note that
:cpp:var:`config::static_align` is subject to the `usual constraints <https://en.cppreference.com/w/cpp/language/object>`__
for an alignment value - specifically, the value must be a power of two.

If a :cpp:var:`config::static_align` value smaller than the default one is specified,
and the alignment required for a specific value type ``T`` exceeds it,
then the :cpp:class:`wrap` class will automatically switch to dynamic storage.

Analogously to :cpp:var:`config::static_size`, :cpp:var:`config::static_align` accounts
for the alignment constraints imposed not only by the value type but also by the type-erasure
machinery. That is, if you specify a :cpp:var:`config::static_align` of 8, that does not necessarily
mean that it is possible to store in static storage values with an alignment of 8 or less, as the
type-erasure machinery might impose additional alignment constraints.

Similarly to :cpp:var:`holder_size`, the :cpp:var:`holder_align` helper can be used to
compute the alignment requirement of :cpp:class:`wrap`'s static storage for a specific value type ``T``.
Let us see a simple example.

First, we define a new configuration instance in which we specify
both a custom static size **and** a custom alignment:

.. literalinclude:: ../tutorial/custom_storage.cpp
   :language: c++
   :lines: 36-37

We can then define a new wrap type using the ``custom_config2``
settings, and verify that it can store pointers in static storage:

.. literalinclude:: ../tutorial/custom_storage.cpp
   :language: c++
   :lines: 39-43

.. code-block:: console

   Is w3 using static storage? true

Finally, we can verify how specifying a smaller alignment can (at least on some platforms) reduce the
memory footprint of the :cpp:class:`wrap` class:

.. literalinclude:: ../tutorial/custom_storage.cpp
   :language: c++
   :lines: 45

.. code-block:: console

   sizeof(wrap1_t) is 32, sizeof(wrap2_t) is 24

Full code listing
-----------------

.. literalinclude:: ../tutorial/custom_storage.cpp
    :language: c++
