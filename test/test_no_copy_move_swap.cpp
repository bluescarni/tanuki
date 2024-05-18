#include <concepts>
#include <utility>

#include <tanuki/tanuki.hpp>

#include <catch2/catch_test_macros.hpp>
#include <utility>

#if defined(__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

template <typename Base, typename, typename>
struct any_iface_impl : Base {
};

// NOLINTNEXTLINE
struct any_iface {
    template <typename Base, typename Holder, typename T>
    using impl = any_iface_impl<Base, Holder, T>;
};

template <typename W, typename T, typename... Args>
concept emplaceable = requires(W &w, Args &&...args) { emplace<T>(w, std::forward<Args>(args)...); };

struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable &) = delete;
    noncopyable(noncopyable &&) noexcept = default;
    noncopyable &operator=(const noncopyable &) = delete;
    noncopyable &operator=(noncopyable &&) noexcept = default;
    ~noncopyable() = default;

    int m_value = {};
};

TEST_CASE("noncopyable")
{
    using wrap_t = tanuki::wrap<any_iface>;

    REQUIRE(!std::constructible_from<wrap_t, noncopyable>);
    REQUIRE(!std::constructible_from<wrap_t, std::in_place_type_t<noncopyable>>);
    REQUIRE(!emplaceable<wrap_t, noncopyable>);

    REQUIRE(!tanuki::valid_config<tanuki::config<>{.copyable = true, .movable = false}>);
    REQUIRE(!tanuki::valid_config<tanuki::config<>{.copyable = true, .movable = true, .swappable = false}>);

    using wrap2_t = tanuki::wrap<any_iface, tanuki::config<>{.copyable = false}>;

    REQUIRE(std::constructible_from<wrap2_t, noncopyable>);
    REQUIRE(std::constructible_from<wrap2_t, std::in_place_type_t<noncopyable>>);
    REQUIRE(emplaceable<wrap2_t, noncopyable>);
    REQUIRE(!std::copyable<wrap2_t>);
    REQUIRE(std::movable<wrap2_t>);
    REQUIRE(std::swappable<wrap2_t>);

    using wrap3_t = tanuki::wrap<any_iface, tanuki::config<>{.semantics = tanuki::wrap_semantics::reference}>;

    REQUIRE(std::constructible_from<wrap3_t, noncopyable>);
    REQUIRE(std::constructible_from<wrap3_t, std::in_place_type_t<noncopyable>>);
    REQUIRE(emplaceable<wrap3_t, noncopyable>);
    REQUIRE(std::copyable<wrap3_t>);
    REQUIRE(std::movable<wrap3_t>);
    REQUIRE(std::swappable<wrap3_t>);
}

struct nonmovable {
    nonmovable() = default;
    nonmovable(const nonmovable &) = delete;
    nonmovable(nonmovable &&) noexcept = delete;
    nonmovable &operator=(const nonmovable &) = delete;
    nonmovable &operator=(nonmovable &&) noexcept = delete;
    ~nonmovable() = default;

    int m_value = {};
};

TEST_CASE("nonmovable")
{
    using wrap_t = tanuki::wrap<any_iface>;

    REQUIRE(!std::constructible_from<wrap_t, nonmovable>);
    REQUIRE(!std::constructible_from<wrap_t, std::in_place_type_t<nonmovable>>);
    REQUIRE(!emplaceable<wrap_t, nonmovable>);

    using wrap2_t = tanuki::wrap<any_iface, tanuki::config<>{.copyable = false, .movable = false, .swappable = false}>;

    REQUIRE(!std::constructible_from<wrap2_t, nonmovable>);
    REQUIRE(std::constructible_from<wrap2_t, std::in_place_type_t<nonmovable>>);
    REQUIRE(emplaceable<wrap2_t, nonmovable>);

    using wrap3_t = tanuki::wrap<any_iface, tanuki::config<>{.semantics = tanuki::wrap_semantics::reference}>;

    REQUIRE(!std::constructible_from<wrap3_t, nonmovable>);
    REQUIRE(std::constructible_from<wrap3_t, std::in_place_type_t<nonmovable>>);
    REQUIRE(emplaceable<wrap3_t, nonmovable>);
    REQUIRE(std::copyable<wrap3_t>);
    REQUIRE(std::movable<wrap3_t>);
    REQUIRE(std::swappable<wrap3_t>);
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif
