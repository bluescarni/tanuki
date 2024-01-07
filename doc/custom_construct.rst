.. _custom_construct:

.. cpp:namespace-push:: tanuki

More on construction
====================

.. _invalid_state:

The invalid state
-----------------

A :cpp:class:`wrap` object is in the *invalid* state when it is empty, that is, it does not contain
any value. The validity status of a :cpp:class:`wrap` can be checked via the :cpp:func:`~wrap::is_invalid()`
function.

A :cpp:class:`wrap` object can become invalid in a variety of circumstances:

- it is created (or assigned) from an instance of :cpp:struct:`invalid_wrap_t`,
- it has been moved-from,
- it has been swapped with an invalid :cpp:class:`wrap`,
- the generic assignment operator threw an exception,
- deserialisation threw an exception.

The only allowed operations on an invalid :cpp:class:`wrap` are:

- destruction,
- the invocation of :cpp:func:`~wrap::is_invalid()`,
- copy/move assignment from, and swapping with, a valid :cpp:class:`wrap`,
- generic assignment.

Note that the invalid status is only about whether or not a :cpp:class:`wrap`
contains a value -- a valid wrap could be containing a moved-from value,
and it is up to the user to handle such an occurrence.

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
