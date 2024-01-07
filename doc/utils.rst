.. cpp:namespace-push:: tanuki

Utilities
=========

.. cpp:type:: template <typename T> unwrap_cvref_t = std::remove_cvref_t<std::unwrap_reference_t<T>>

   A shorthand type alias that removes ``std::reference_wrapper``, const/volatile and reference
   qualifiers (in that order) from :cpp:type:`T`.

.. cpp:struct:: template <typename Base, typename Holder> iface_impl_helper 

   Helper class for interface implementations.

   Interface implementations that inherit from this class can use the
   :cpp:func:`value()` functions to access the type-erased value stored
   in a :cpp:class:`wrap`. If the :cpp:class:`wrap` contains a
   `std::reference_wrapper <https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper>`__,
   the :cpp:func:`value()` functions will automatically unwrap the reference.

   .. cpp:function:: auto &value()

      :return: a mutable reference to the value stored in the :cpp:type:`Holder`.

      :exception std\:\:runtime_error: if the :cpp:type:`Holder` stores a const reference.

   .. cpp:function:: const auto &value() const noexcept

      :return: a const reference to the value stored in the :cpp:type:`Holder`.
