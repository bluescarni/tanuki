#include <utility>
#include <ranges>
#include <cstddef>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../ranges.hpp"
#include "../time_series.hpp"

namespace py = pybind11;

struct ra_iter_adaptor
{
    py::object obj;
    std::ptrdiff_t idx = 0;

    py::object operator*() const
    {
        return obj[py::cast(idx)];
    }
    void operator++()
    {
        ++idx;
    }
    void operator--()
    {
        --idx;
    }
    void operator+=(std::ptrdiff_t n)
    {
        idx += n;
    }
    void operator-=(std::ptrdiff_t n)
    {
        idx -= n;
    }
    [[nodiscard]] std::ptrdiff_t distance_from(const ra_iter_adaptor &other) const
    {
        return idx - other.idx;
    }
    friend bool operator==(const ra_iter_adaptor &a, const ra_iter_adaptor &b)
    {
        return a.idx == b.idx;
    }
    friend bool operator<(const ra_iter_adaptor &a, const ra_iter_adaptor &b)
    {
        return a.idx < b.idx;
    }
};

struct ra_range_adaptor
{
    py::object obj;

    [[nodiscard]] auto begin() const
    {
        return ra_iter_adaptor{obj, 0};
    }
    [[nodiscard]] auto end() const
    {
        return ra_iter_adaptor{obj, static_cast<std::ptrdiff_t>(py::len(obj))};
    }
};

template <typename TS, typename Key, std::indirect_strict_weak_order<const Key *, const Key *> Comp = std::ranges::less>
    requires facade::any_random_access_ts<TS> && std::same_as<Key, facade::ts_key_t<TS>>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
auto lagrange_interpolation(TS &&ts, const Key &key, std::size_t order, Comp comp = {})
{
    assert(order >= 2u);
    const auto order_half = static_cast<std::ranges::range_difference_t<TS>>(order / 2u);

    // Projection to extract a reference to the key from
    // a time series record.
    const auto proj = [](const auto &p) -> const auto & { return std::get<0>(p); };

    // Locate the first key in ts which is greater than the input key.
    const auto center_it = std::ranges::upper_bound(ts, key, comp, proj);

    // How much can we move left and right of center_it without exiting ts?
    const auto max_distance_right = std::ranges::distance(center_it, std::ranges::end(ts));
    const auto max_distance_left = std::ranges::distance(std::ranges::begin(ts), center_it);

    // Establish the interpolation interval.
    const auto lag_begin
        = std::ranges::prev(center_it, (order_half > max_distance_left) ? max_distance_left : order_half);
    const auto lag_end
        = std::ranges::next(center_it, (order_half > max_distance_right) ? max_distance_right : order_half);

    // The value type used for interpolation.
    using value_t = facade::ts_value_t<TS>;

    // Run the interpolation.
    value_t result{};

    for (auto outer = lag_begin; outer != lag_end; ++outer) {
        auto term = std::get<1>(*outer);

        for (auto inner = lag_begin; inner != lag_end; ++inner) {
            if (outer != inner) {
                term *= (key - std::get<0>(*inner)) / (std::get<0>(*outer) - std::get<0>(*inner));
            }
        }

        if (outer == lag_begin) {
            result = term;
        } else {
            result += term;
        }
    }

    return result;
}

PYBIND11_MODULE(basic, m)
{
    m.def("print_iterable", [](py::iterable it){
        auto irange = facade::make_input_range(std::move(it));
        std::ranges::for_each(irange, [](const auto &x){py::print(x);});
    });

    m.def("lagrange_interpolation", [](py::object rng, py::object key, std::size_t order){
        auto ra_rng = facade::make_random_access_range(ra_range_adaptor{std::move(rng)});

        auto tview = ra_rng | std::ranges::views::transform([](const auto &o) {
            auto tup = py::cast<py::tuple>(o);
            return std::make_pair(py::object(tup[0]), py::object(tup[1]));
        });
    
        return lagrange_interpolation(tview, key, order);
    });
}
