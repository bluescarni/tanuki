.. _config_settings:

.. cpp:namespace-push:: tanuki

Configuration options
=====================

.. cpp:struct:: template <typename DefaultValueType = void, typename RefIFace = no_ref_iface> requires (std::same_as<DefaultValueType, void> || valid_value_type<DefaultValueType>) && valid_ref_iface<RefIFace> config

   Configuration struct.

   This struct stores tunable config options for the :cpp:class:`wrap` class.
   
   :cpp:type:`DefaultValueType` represents the type used by the default constructor
   of :cpp:class:`wrap`. If set to ``void``, the default constructor of :cpp:class:`wrap`
   is disabled (unless the :cpp:var:`invalid_default_ctor` option is set to ``true``, see below).
   
   :cpp:type:`RefIFace` is the :ref:`reference interface <ref_interface>` selected for the
   :cpp:class:`wrap` class. The default type :cpp:struct:`no_ref_iface` selects an empty
   reference interface.

   .. cpp:var:: std::size_t static_size = 48

      Threshold size (in bytes) for the small-object optimisation in the :cpp:class:`wrap` class.
      If set to zero, the small-object optimisation is disabled and the :cpp:class:`wrap` class
      always uses dynamically-allocated memory.

   .. cpp:var:: std::size_t static_align = alignof(std::max_align_t)

      Alignment (in bytes) for the small-object optimisation in the :cpp:class:`wrap` class.
      This value must be a power of two.

   .. cpp:var:: bool invalid_default_ctor = false

      If this option is set to ``true``, then the default constructor of :cpp:class:`wrap`
      initialises to the :ref:`invalid state <invalid_state>`. Otherwise, the default constructor of :cpp:class:`wrap`
      is disabled, unless a non-``void`` :cpp:type:`DefaultValueType` is selected.

   .. cpp:var:: bool pointer_interface = true

      This option selects whether or not the :ref:`pointer interface <ref_interface>` is enabled.

   .. cpp:var:: wrap_ctor explicit_ctor = wrap_ctor::always_explicit

      This option selects when the generic constructor of the :cpp:class:`wrap` class
      is ``explicit``.

   .. cpp:var:: wrap_semantics semantics = wrap_semantics::value

   .. cpp:var:: bool copyable = true

      This option selects whether or not the :cpp:class:`wrap` class is copy constructible/assignable.

   .. cpp:var:: bool movable = true

      This option selects whether or not the :cpp:class:`wrap` class is move constructible/assignable.

   .. cpp:var:: bool swappable = true

      This option selects whether or not the :cpp:class:`wrap` class is swappable.


.. cpp:var:: inline constexpr auto default_config = config{}

   Default configuration for the :cpp:class:`wrap` class.

.. cpp:struct:: no_ref_iface

   A placeholder type that represents an empty :ref:`reference interface <ref_interface>`.
   This is a trivial empty struct.

.. cpp:enum-class:: wrap_ctor

   Enumerator representing the explicitness of the generic constructor of the :cpp:class:`wrap` class.

   .. cpp:enumerator:: always_explicit

      The constructor is always ``explicit``.

   .. cpp:enumerator:: ref_implicit

      The constructor is implicit when constructing from a ``std::reference_wrapper``,
      ``explicit`` otherwise.

   .. cpp:enumerator:: always_implicit

      The constructor is always implicit.

.. cpp:enum-class:: wrap_semantics

.. cpp:var:: template <typename T, typename IFace> requires iface_with_impl<IFace, T> inline constexpr std::size_t holder_size

   Helper to compute the total amount of memory (in bytes) needed to statically store in a :cpp:class:`wrap`
   a value of type :cpp:type:`T` wrapped by the interface :cpp:type:`IFace`.

.. cpp:var:: template <typename T, typename IFace> requires iface_with_impl<IFace, T> inline constexpr std::size_t holder_align

   Helper to compute the alignment (in bytes) required to statically store in a :cpp:class:`wrap`
   a value of type :cpp:type:`T` wrapped by the interface :cpp:type:`IFace`.

.. cpp:concept:: template <typename RefIFace> valid_ref_iface

   This concept is satisfied if :cpp:type:`RefIFace` is a valid :ref:`reference interface <ref_interface>`.

   When using C++23, this concept is satisfied by any non-cv qualified class type. In C++20, :cpp:type:`RefIFace`
   must also define in its scope an ``impl`` class template/template alias depending exactly on one parameter.

.. cpp:concept:: template <auto Cfg> valid_config

   Concept for checking that :cpp:var:`Cfg` is a valid instance of :cpp:class:`config`.

   Specifically, the concept is satisfied if:

   - :cpp:var:`Cfg` is an instance of the primary :cpp:class:`config` template,
   - :cpp:var:`config::static_align` is a power of two,
   - :cpp:var:`config::explicit_ctor` is one of the enumerators defined in :cpp:enum:`wrap_ctor`,
   - :cpp:var:`config::semantics` is one of the enumerators defined in :cpp:enum:`wrap_semantics`,
   - if :cpp:var:`config::copyable` is set to ``true``, so is :cpp:var:`config::movable` (that is,
     a copyable :cpp:class:`wrap` must also be movable),
   - if :cpp:var:`config::movable` is set to ``true``, so is :cpp:var:`config::swappable` (that is,
     a movable :cpp:class:`wrap` must also be swappable).
