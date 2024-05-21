.. cpp:namespace-push:: tanuki

The ``wrap`` class
==================

.. cpp:class:: template <typename IFace, auto Cfg = default_config> requires std::is_class_v<IFace> && std::same_as<IFace, std::remove_cv_t<IFace>> && valid_config<Cfg> wrap

   .. cpp:function:: wrap()

      Default constructor.

      The default constructor is enabled in the following circumstances:

      - the configuration option :cpp:var:`~config::invalid_default_ctor` in :cpp:var:`Cfg` is set to
        ``true``. In this case, the default constructor initialises into the
        :ref:`invalid state <invalid_state>`. Otherwise,
      - a non-``void`` default-initialisable :cpp:type:`~config::DefaultValueType` with a valid, default-initialisable
        interface implementation for :cpp:type:`IFace` has been specified as first template argument in :cpp:var:`Cfg`.
        In this case, the default constructor value-initialises an instance of :cpp:type:`~config::DefaultValueType` as
        the internal type-erased value. When employing value semantics, the copyability, movability and swappability of
        :cpp:type:`~config::DefaultValueType` must be consistent with the corresponding settings
        in :cpp:var:`Cfg` in order for this constructor to be enabled.

      In both cases, the :ref:`reference interface <ref_interface>` must
      be default-initialisable in order for this constructor to be available.

      :throws: any exception thrown by the default constructor of the interface implementation or of
        the :ref:`reference interface <ref_interface>`, by the value-initialisation of a
        non-``void`` :cpp:type:`~config::DefaultValueType` or by memory allocation errors
        if the non-``void`` :cpp:type:`~config::DefaultValueType` does not fit in static storage
        or if reference semantics is being used. If it can be determined at compile time that none
        of these conditions can occurr, then this constructor is marked ``noexcept``.

   .. cpp:function:: explicit wrap(invalid_wrap_t)

      Explicit initialisation into the :ref:`invalid state <invalid_state>`.

      This constructor is enabled only if the :ref:`reference interface <ref_interface>` is
      default-initialisable.

      :throws: any exception thrown by the default constructor of the :ref:`reference interface <ref_interface>`.
        If the default constructor of the reference interface does not throw, then this constructor
        is marked ``noexcept``.

   .. cpp:function:: template <typename T> wrap(T &&x)

      Generic constructor.

      This constructor will create a :cpp:class:`wrap` from the input value :cpp:var:`x`.

      This constructor is enabled only if *all* the following conditions are satisfied:

      - the :ref:`reference interface <ref_interface>` is default-initialisable;
      - after the removal of reference and cv-qualifiers, :cpp:type:`T` is not

        - the same as :cpp:class:`wrap` (so that this constructor does not interfere with
          the copy/move constructors),
        - an instance of ``std::in_place_type_t`` (so that this constructor does not interfere
          with the in-place constructor),
        - the same as :cpp:struct:`invalid_wrap_t` (so that this constructor does not interfere
          with the constructor into the :ref:`invalid state <invalid_state>`);

      - the interface :cpp:type:`IFace` has a valid, default-initialisable implementation for the value type :cpp:type:`T`
        (see the :cpp:concept:`iface_with_impl` concept);
      - :cpp:var:`x` can be perfectly-forwarded to construct an instance of the value type;
      - when employing value semantics, the copyability, movability and swappability of the value type
        are consistent with the corresponding settings in :cpp:var:`Cfg`.

      This constructor is marked ``explicit`` if either:

      - :cpp:type:`T` is a ``std::reference_wrapper`` and the value of :cpp:var:`~config::explicit_ctor` in :cpp:var:`Cfg`
        if :cpp:enumerator:`wrap_ctor::always_explicit`, or
      - :cpp:type:`T` is *not* a ``std::reference_wrapper`` and the value of :cpp:var:`~config::explicit_ctor` in :cpp:var:`Cfg`
        is less than :cpp:enumerator:`wrap_ctor::always_implicit`.

      Otherwise, the constructor is implicit.

      :param x: the input value.

      :throws: any exception thrown by the default constructor of the interface implementation or of
        the :ref:`reference interface <ref_interface>`, by the construction of the value type or by memory allocation errors
        if the value type does not fit in static storage or if reference semantics is being used. If it can be
        determined at compile time that none of these conditions can occurr, then this constructor
        is marked ``noexcept``.

   .. cpp:function:: template <typename T, typename... U> explicit wrap(std::in_place_type_t<T>, U &&...args)

      Generic in-place constructor.

      This constructor will create a :cpp:class:`wrap` containing a type-erased value of type :cpp:type:`T`
      constructed from the input argument(s) :cpp:var:`args`. If no input arguments are provided, the internal
      value will be value-initialised.

      This constructor is enabled only if *all* the following conditions are satisfied:

      - :cpp:type:`T` is an object type without cv qualifications;
      - the :ref:`reference interface <ref_interface>` is default-initialisable;
      - the interface :cpp:type:`IFace` has a valid, default-initialisable implementation for the value type :cpp:type:`T`
        (see the :cpp:concept:`iface_with_impl` concept);
      - :cpp:var:`args` can be perfectly-forwarded to construct an instance of the value type :cpp:type:`T`;
      - when employing value semantics, the copyability, movability and swappability of the value type :cpp:type:`T`
        are consistent with the corresponding settings in :cpp:var:`Cfg`.

      :param args: the input construction arguments.

      :throws: any exception thrown by the default constructor of the interface implementation or of
        the :ref:`reference interface <ref_interface>`, by the construction of the value type or by memory allocation errors
        if the value type does not fit in static storage or if reference semantics is being used. If it can be
        determined at compile time that none of these conditions can occurr, then this constructor
        is marked ``noexcept``.

   .. cpp:function:: wrap(const wrap &other)

      Copy constructor.

      When employing value semantics, the copy constructor will copy-construct the type-erased value from :cpp:var:`other`.
      Otherwise, a :cpp:class:`wrap` sharing ownership of the type-erased value with :cpp:var:`other` will be constructed.

      This constructor is enabled only if the following conditions are satisfied:

      - the :ref:`reference interface <ref_interface>` is default-initialisable;
      - when employing value semantics, the :cpp:var:`~config::copyable` option in :cpp:var:`Cfg`
        is activated.

      :param other: the :cpp:class:`wrap` to be copied.

      :throws: any exception thrown by the default constructor of the interface implementation or of
        the :ref:`reference interface <ref_interface>`, or by the copy-construction of the value type or by memory allocation errors
        when value semantics is being used. This constructor
        is marked ``noexcept`` when using reference semantics and if the :ref:`reference interface <ref_interface>`'s
        default constructor is marked ``noexcept``.

   .. cpp:function:: [[nodiscard]] friend bool is_invalid(const wrap &w) noexcept

      This function will return ``true`` if :cpp:var:`w` is in the :ref:`invalid state <invalid_state>`,
      ``false`` otherwise.

      :param w: the input argument.

      :return: the validity status for :cpp:var:`w`.

   .. cpp:function:: [[nodiscard]] friend const IFace *iface_ptr(const wrap &w) noexcept
                     [[nodiscard]] friend const IFace *iface_ptr(const wrap &&w) noexcept
                     [[nodiscard]] friend IFace *iface_ptr(wrap &w) noexcept
                     [[nodiscard]] friend IFace *iface_ptr(wrap &&w) noexcept

      Fetch a pointer to the interface.

      These functions will return a pointer to the instance of the interface :cpp:type:`IFace` stored
      within a :cpp:class:`wrap`.
      If :cpp:var:`w` is in the :ref:`invalid state <invalid_state>`, then ``nullptr`` will be returned.

      :param w: the input argument.

      :return: a pointer to the interface.

   .. cpp:function:: template <typename T, typename... Args> friend void emplace(wrap &w, Args &&...args)

      Emplace a value into a :cpp:class:`wrap`.

      This function will first destroy the value in :cpp:var:`w` (if :cpp:var:`w` is not already in the :ref:`invalid state <invalid_state>`).
      It will then construct in :cpp:var:`w` a value of type :cpp:type:`T` using the construction arguments :cpp:type:`Args`.

      This function is enabled only if the following conditions are satisfied:

      - :cpp:type:`T` is an object type without cv qualifications;
      - an instance of :cpp:type:`T` can be constructed from :cpp:type:`Args`;
      - the interface :cpp:type:`IFace` has a valid, default-initialisable implementation for the value type :cpp:type:`T`
        (see the :cpp:concept:`iface_with_impl` concept);
      - when employing value semantics, the copyability, movability and swappability of the value type :cpp:type:`T`
        are consistent with the corresponding settings in :cpp:var:`Cfg`.

      Passing :cpp:var:`w` as an argument in :cpp:var:`args` (e.g., attempting to emplace :cpp:var:`w` into itself) will lead to
      undefined behaviour.

      This function is ``noexcept`` if all these conditions are satisfied:

      - :cpp:var:`w` is using value semantics,
      - the static size and alignment of :cpp:var:`w` are :ref:`large enough <custom_storage>` to store an instance of :cpp:type:`T`,
      - the invoked constructor of :cpp:type:`T` does not throw.

      If an exception is thrown, :cpp:var:`w` may be left in the :ref:`invalid state <invalid_state>`.

      :param w: the target :cpp:class:`wrap`.
      :param args: the construction arguments.

      :throws: any exception thrown by memory allocation primitives or by the
         invoked constructor of :cpp:type:`T`.

   .. cpp:function:: [[nodiscard]] friend bool has_static_storage(const wrap &w) noexcept

      Query the storage type of a :cpp:class:`wrap`.

      :param w: the input :cpp:class:`wrap`.

      :return: ``true`` if :cpp:var:`w` is currently employing static storage, ``false`` otherwise.

   .. cpp:function:: [[nodiscard]] friend wrap copy(const wrap &w) requires(Cfg.semantics == wrap_semantics::reference)

      Make a deep-copy of a :cpp:class:`wrap` employing :ref:`reference semantics <ref_semantics>`.

      This function will return a new :cpp:class:`wrap` containing a copy of the value stored in :cpp:var:`w`.

      :param w: the input :cpp:class:`wrap`.

      :return: a deep copy of :cpp:var:`w`.

      :throws std\:\:invalid_argument: if the value stored in :cpp:var:`w` is not copy-constructible.
      :throws: any exception thrown by memory allocation primitives or by the
         copy constructor of the value stored in :cpp:var:`w`.

   .. cpp:function:: [[nodiscard]] friend bool same_value(const wrap &w1, const wrap &w2) noexcept requires(Cfg.semantics == wrap_semantics::reference)

      Check if two :cpp:class:`wrap` objects employing :ref:`reference semantics <ref_semantics>` share
      ownership of the internal value.

      :param w1: the first :cpp:class:`wrap`.
      :param w2: the second :cpp:class:`wrap`.

      :return: ``true`` if :cpp:var:`w1` and :cpp:var:`w2` share the internal value, ``false`` otherwise.

.. cpp:function:: [[nodiscard]] bool is_valid(const wrap &w) noexcept

   This function will return ``false`` if :cpp:var:`w` is in the :ref:`invalid state <invalid_state>`,
   ``true`` otherwise.

   :param w: the input argument.

   :return: the validity status for :cpp:var:`w`.

.. cpp:function:: template <typename IFace, auto Cfg> bool has_dynamic_storage(const wrap<IFace, Cfg> &w) noexcept

   Query the storage type of a :cpp:class:`wrap`.

   :param w: the input :cpp:class:`wrap`.

   :return: ``true`` if :cpp:var:`w` is currently employing dynamic storage, ``false`` otherwise.

.. cpp:struct:: invalid_wrap_t

   A tag structure used to set a :cpp:class:`wrap` to the :ref:`invalid state <invalid_state>`.
   This is a trivial empty struct.

.. cpp:var:: inline constexpr auto invalid_wrap = invalid_wrap_t{}

   A global instance of :cpp:struct:`invalid_wrap_t`.

.. cpp:concept:: template <typename T> any_wrap

   This concept is satisfied if :cpp:type:`T` is any instance of :cpp:class:`wrap`.

.. cpp:struct:: template <typename T, typename IFace, wrap_semantics Sem> holder

   Holder class for type-erased values.

   .. note::

      This class is to be regarded as an implementation detail, and as such it is left
      undocumented on purpose.

.. cpp:concept:: template <typename T> any_holder

   This concept is satisfied if :cpp:type:`T` is any instance of :cpp:class:`holder`.
