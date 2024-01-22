.. cpp:namespace-push:: tanuki

The ``wrap`` class
==================

.. cpp:class:: template <typename IFace, auto Cfg = default_config> wrap

   .. cpp:function:: [[nodiscard]] friend bool is_invalid(const wrap &w) noexcept

      This function will return ``true`` if *w* is in the :ref:`invalid state <invalid_state>`,
      ``false`` otherwise.

      :param w: the input argument.

      :return: the validity status for *w*.

   .. cpp:function:: template <typename T, typename... Args> friend void emplace(wrap &w, Args &&...args)

      Emplace a value into a :cpp:class:`wrap`.

      This function will first destroy the value in *w* (if *w* is not already in the :ref:`invalid state <invalid_state>`).
      It will then construct in *w* a value of type :cpp:type:`T` using the construction arguments :cpp:type:`Args`.

      This function is enabled only if :cpp:type:`T` is not :cpp:class:`wrap` and if an instance of :cpp:type:`T`
      can be constructed from :cpp:type:`Args`.

      This function is ``noexcept`` if all these conditions are satisfied:

      - *w* is using value semantics,
      - the static size and alignment of *w* are :ref:`large enough <custom_storage>` to store an instance of :cpp:type:`T`,
      - the invoked constructor of :cpp:type:`T` does not throw.

      If an exception is thrown, *w* may be left in the :ref:`invalid state <invalid_state>`.

      :param w: the target :cpp:class:`wrap`.
      :param args: the construction arguments.

      :throws: any exception thrown by memory allocation primitives or by the
         invoked constructor of :cpp:type:`T`.

   .. cpp:function:: [[nodiscard]] friend bool has_static_storage(const wrap &w) noexcept

      Query the storage type of a :cpp:class:`wrap`.

      :param w: the input :cpp:class:`wrap`.

      :return: ``true`` if *w* is currently employing static storage, ``false`` otherwise.

.. cpp:function:: template <typename IFace, auto Cfg> bool has_dynamic_storage(const wrap<IFace, Cfg> &w) noexcept

   Query the storage type of a :cpp:class:`wrap`.

   :param w: the input :cpp:class:`wrap`.

   :return: ``true`` if *w* is currently employing dynamic storage, ``false`` otherwise.

.. cpp:struct:: invalid_wrap_t

   A tag structure used to set a :cpp:class:`wrap` to the :ref:`invalid state <invalid_state>`.

.. cpp:var:: inline constexpr auto invalid_wrap = invalid_wrap_t{}

   A global instance of :cpp:struct:`invalid_wrap_t`.
