.. _custom_construct:

.. cpp:namespace-push:: tanuki

More on construction
====================

.. _def_ctor:

Default construction
--------------------

By default, the default constructor of :cpp:class:`wrap` is disabled.
There are two ways of enabling default construction for a :cpp:class:`wrap`.

First, if the :cpp:var:`config::invalid_default_ctor` option is activated,
then default construction will initialise a :cpp:class:`wrap` into the
:ref:`invalid state <invalid_state>`. This can be useful to model types such
as `std::function <https://en.cppreference.com/w/cpp/utility/functional/function>`__
for which an empty state is meaningful.

The other option is to specify a custom (i.e., non-``void``) ``DefaultValueType``
as first template argument in :cpp:struct:`config`, in which case the default constructor of
:cpp:class:`wrap` will value-initialise an interal value of type ``DefaultValueType``.
Note that a custom ``DefaultValueType`` must satisfy the requirements
of the :cpp:class:`wrap` interface.

If both the :cpp:var:`config::invalid_default_ctor` option is activated and
a custom ``DefaultValueType`` is specified, then the :cpp:var:`config::invalid_default_ctor` option
takes the precedence.

.. _invalid_state:

The invalid state
-----------------

A :cpp:class:`wrap` object is in the *invalid* state when it is empty, that is, it does not contain
any value. The validity of a :cpp:class:`wrap` can be checked via the :cpp:func:`~wrap::is_invalid()`
function.

A :cpp:class:`wrap` object can become invalid in a variety of circumstances:

- it is created (or assigned) from an instance of :cpp:struct:`invalid_wrap_t`,
- it has been moved-from,
- it has been swapped with an invalid :cpp:class:`wrap`,
- the generic assignment operator threw an exception,
- :ref:`emplacement <emplacement>` into an existing :cpp:class:`wrap` threw
  an exception,
- deserialisation threw an exception.

The only allowed operations on an invalid :cpp:class:`wrap` are:

- destruction,
- the invocation of :cpp:func:`~wrap::is_invalid()`,
- copy/move assignment from, and swapping with, a valid :cpp:class:`wrap`,
- :ref:`emplacement <emplacement>`,
- generic assignment.

Note that the invalid status is only about whether or not a :cpp:class:`wrap`
contains a value -- a valid wrap could be containing a moved-from value,
and it is up to the user to handle such an occurrence.

.. _emplacement:

Emplacement
-----------

In the examples we have seen so far, a :cpp:class:`wrap` was always constructed by copying/moving in
a value. Sometimes, however, it is necessary to type-erase values which are neither copyable nor movable.
A classic example is `std::mutex <https://en.cppreference.com/w/cpp/thread/mutex>`__.
In order to be able to wrap such types, :cpp:class:`wrap` supports *emplacement*, that is, a form
of direct initialisation that does not require to pass through a temporary object.

A :cpp:class:`wrap` can be emplace-constructed via a special constructor that, similarly
to `std::variant <https://en.cppreference.com/w/cpp/utility/variant/variant>`__, takes in input
an instance of `std::in_place_type_t <https://en.cppreference.com/w/cpp/utility/in_place>`__
(representing the desired value type) and a variadic pack. The variadic pack is used to directly
construct an object of the desired type.

Let us see a short example with ``std::mutex``. We have our usual minimal interface
and its implementation:

.. literalinclude:: ../tutorial/emplace.cpp
   :language: c++
   :lines: 6-15

We can emplace-construct a :cpp:class:`wrap` containing a ``std::mutex``:

.. literalinclude:: ../tutorial/emplace.cpp
   :language: c++
   :lines: 19-22

Note that in this specific case the variadic pack is empty (as the constructor of
``std::mutex`` takes no arguments) and thus only the ``std::in_place_type_t`` argument
is required.

In addition to the emplace constructor, the :cpp:func:`~wrap::emplace()` function is also
available to emplace-assign a value into an existing :cpp:class:`wrap` object. Here
is a simple example:

.. literalinclude:: ../tutorial/emplace.cpp
   :language: c++
   :lines: 24-28

Customising :cpp:class:`wrap`'s constructors
--------------------------------------------

Beside the :ref:`default constructor <def_ctor>`, it is also possible to customise
the behaviour of :cpp:class:`wrap`'s other constructors in a variety of ways.

:cpp:class:`wrap`'s generic constructors, for instance, are by default ``explicit``,
but they can be made implicit by altering the :cpp:var:`config::explicit_ctor`
configuration setting. If :cpp:var:`config::explicit_ctor` is set to
:cpp:enumerator:`wrap_ctor::ref_implicit`, then generic construction is implicit
when :ref:`wrapping a reference <wrap_reference>`, ``explicit`` otherwise.
If :cpp:var:`config::explicit_ctor` is set to
:cpp:enumerator:`wrap_ctor::always_implicit`, then generic construction is always implicit.

By default, :cpp:class:`wrap` is copy/move constructible and copy/move assignable.
The copy/move constructors and assignment operators can be disabled by switching off
the :cpp:var:`config::copyable` and :cpp:var:`config::movable` configuration settings.

The :cpp:class:`wrap` is also by default `swappable <https://en.cppreference.com/w/cpp/types/is_swappable>`__.
Swappability can be controlled via the :cpp:var:`config::swappable` configuration setting.
