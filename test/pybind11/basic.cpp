#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../ranges.hpp"
#include "../time_series.hpp"

namespace py = pybind11;

struct ra_iter_adaptor {
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

struct ra_range_adaptor {
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

PYBIND11_MODULE(basic, m)
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace py::literals;

    m.def(
        "print_iterable",
        [](py::iterable it) {
            auto irange = facade::make_input_range(std::move(it));
            std::ranges::for_each(irange, [](const auto &x) { py::print(x); });
        },
        "it"_a);

    using proj_t = std::function<py::object(const py::object &)>;

    struct key_proj {
        py::object operator()(const py::object &t) const
        {
            return t[py::cast(0)];
        }
    };

    struct value_proj {
        py::object operator()(const py::object &t) const
        {
            return t[py::cast(1)];
        }
    };

    m.def(
        "lagrange_interpolation",
        // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
        [](py::object rng, const py::object &key, std::size_t order, proj_t kproj, proj_t vproj) {
            auto ra_rng = facade::make_random_access_range(ra_range_adaptor{std::move(rng)});

            return facade::lagrange_interpolation(ra_rng, key, order, {}, std::move(kproj), std::move(vproj));
        },
        "rng"_a, "key"_a, "order"_a, "kproj"_a = proj_t{key_proj{}}, "vproj"_a = proj_t{value_proj{}});
}
