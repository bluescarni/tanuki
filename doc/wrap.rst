.. cpp:namespace-push:: tanuki

The ``wrap`` class
==================

.. cpp:class:: template <typename IFace, auto Cfg = default_config> wrap

   .. cpp:function:: wrap()

      Default constructor.

      The default constructor is enabled in the following circumstances:

      - the configuration option :cpp:var:`config::invalid_default_ctor` is set to
        ``true``. In this case, the default constructor initialises into the
        :ref:`invalid state <invalid_state>`. Otherwise,
      - a non-``void`` default-initialisable ``DefaultValueType`` with a valid :cpp:type:`IFace` implementation
        has been specified as first template argument in the in :cpp:struct:`config` structure. In this case,
        the default constructor value-initialises an instance of ``DefaultValueType`` as
        the internal type-erased type.

      In both cases, :cpp:type:`IFace`, its implementation and the :ref:`reference interface <ref_interface>` must
      all be default-initialisable in order for this constructor to be available.

      :throws: any exception thrown by the default constructor of :cpp:type:`IFace`, its implementation, or
        the :ref:`reference interface <ref_interface>`, by the value-initialisation of a
        non-``void`` ``DefaultValueType`` or by memory allocation errors
        if the non-``void`` ``DefaultValueType`` does not fit in static storage. If it can be
        determined at compile time that none of these conditions can occurr, then this constructor
        is marked ``noexcept``.

   .. cpp:function:: explicit wrap(invalid_wrap_t)

      Explicit initialisation into the :ref:`invalid state <invalid_state>`.

      This constructor is enabled only if the :ref:`reference interface <ref_interface>` is
      default-initialisable.

      :throws: any exception thrown by the default constructor of the :ref:`reference interface <ref_interface>`.
        If the default constructor of the reference interface does not throw, then this constructor
        is marked ``noexcept``.

   .. cpp:function:: template <typename T> explicit wrap(T &&x)

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
