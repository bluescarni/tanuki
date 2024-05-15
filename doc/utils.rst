.. cpp:namespace-push:: tanuki

Utilities
=========

.. cpp:type:: template <typename T> unwrap_cvref_t = std::remove_cvref_t<std::unwrap_reference_t<T>>

   A shorthand type alias that removes ``std::reference_wrapper``, const/volatile and reference
   qualifiers (in that order) from :cpp:type:`T`.

.. cpp:concept:: template <typename IFace, typename T> iface_with_impl

   This concept is satisfied if the interface :cpp:type:`IFace` has an implementation
   for the value type :cpp:type:`T`.

.. cpp:function:: template <typename Holder, typename T> requires any_holder<Holder> && std::derived_from<Holder, T> [[nodiscard]] const auto &getval(const T *h) noexcept
                  template <typename Holder, typename T> requires any_holder<Holder> && std::derived_from<Holder, T> [[nodiscard]] auto &getval(T *h)
                  template <typename Holder, typename T> requires any_holder<Holder> && std::derived_from<Holder, T> [[nodiscard]] auto &getval(T &h)

   Type-erased value getters.

   These getters will return a reference to the type-erased value stored in a :cpp:class:`holder` of type
   :cpp:type:`Holder` deriving from :cpp:type:`T`. They are meant to be used within the implementation of
   an :ref:`interface <getval_intro>`. Internally, they will employ the
   `curiously recurring template pattern (CRTP) <https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`__
   to cast *h* to :cpp:type:`Holder` and fetch the type-erased value stored within.

   If the type-erased value stored in *h* is a ``std::reference_wrapper``, these getters will unwrap the reference
   (i.e., they will return a reference to the referenced-to value).

   :param h: a pointer or reference to a :cpp:class:`holder`, cast to its base type :cpp:type:`T`.

   :return: a reference to the type-erased value stored in *h*.

   :throws std\:\:runtime_error: if *h* type-erases a const reference and *h* is not a const pointer/reference.

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
