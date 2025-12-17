.. cpp:namespace-push:: tanuki

Utilities
=========

.. cpp:type:: template <typename T> unwrap_cvref_t = std::remove_cvref_t<std::unwrap_reference_t<T>>

   A shorthand type alias that removes ``std::reference_wrapper``, const/volatile and reference
   qualifiers (in that order) from :cpp:type:`T`.

.. cpp:concept:: template <typename IFace, typename T> iface_with_impl

   This concept is satisfied if the interface :cpp:type:`IFace` has an implementation
   for the value type :cpp:type:`T`.

.. cpp:function:: template <typename T>  [[nodiscard]] auto &getval(T *impl)

   Type-erased value getter.

   This function is intended to be used in an interface implementation in order to access the type-erased value.

   If the type-erased value stored in *impl* is a ``std::reference_wrapper``, this function will unwrap the reference
   (i.e., it will return a reference to the referenced-to value).

   :param impl: a pointer to an interface implementation.

   :return: a reference to the type-erased value stored in *impl*.

   :throws std\:\:runtime_error: if *impl* type-erases a const reference and *impl* is not a const pointer.

.. cpp:struct:: template <typename IFace0, typename IFace1, typename... IFaceN> composite_iface: public IFace0, public IFace1, public IFaceN...

   Composite interface.

   This class can be used to create a :ref:`composite interface <composite_interfaces>` by multiply inheriting
   from the interfaces :cpp:type:`IFace0`, :cpp:type:`IFace1` and :cpp:type:`IFaceN`.

.. cpp:concept:: template <typename T> valid_value_type = std::is_object_v<T> && (!std::is_const_v<T>) && (!std::is_volatile_v<T>) && std::destructible<T>

   This concept detects if :cpp:type:`T` is a type that can be type-erased by a :cpp:class:`wrap`.

   :cpp:type:`T` must be a non-cv qualified destructible object.

.. cpp:struct:: template <typename IFace, typename Base, typename Holder, typename T> iface_impl

   Non-intrusive interface implementation.

   This class can be partially specialised to specify a non-intrusive implementation for the interface :cpp:type:`IFace`.
   See the :ref:`tutorial <nonintrusive>` for an example.

   The unspecialised version of this class is an empty trivial structure which disables non-intrusive implementations
   for the interface :cpp:type:`IFace`.
