.. _wrap_reference:

.. cpp:namespace-push:: tanuki

Type-erasure for references
===========================

In all the examples we have seen so far, :cpp:class:`wrap` objects were constructed
by copying/moving objects into the :cpp:class:`wrap`. It is however also possible
to construct :cpp:class:`wrap` objects that contain references to existing values,
rather than copies of the values. With a small additional effort, it is also possible
to write interface implementations which work seamlessly with both values and references.



