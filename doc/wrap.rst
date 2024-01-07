.. cpp:namespace-push:: tanuki

The ``wrap`` class
==================

.. cpp:class:: template <typename IFace, auto Cfg = default_config> wrap

   .. cpp:function:: [[nodiscard]] friend bool is_invalid(const wrap &w) noexcept

      This function will return ``true`` if *w* is in the :ref:`invalid state <invalid_state>`,
      ``false`` otherwise.

      :param w: the input argument.

      :return: the validity status for *w*.

.. cpp:struct:: invalid_wrap_t

   A tag structure used to set a :cpp:class:`wrap` to the :ref:`invalid state <invalid_state>`.

.. cpp:var:: inline constexpr invalid_wrap_t invalid_wrap{}

   A global instance of :cpp:struct:`invalid_wrap_t`.
