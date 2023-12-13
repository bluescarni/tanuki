// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <tanuki/tanuki.hpp>

#include "fooable.hpp"

namespace fooable
{

void foo_model::foo() const {}

} // namespace fooable

// NOLINTNEXTLINE(cert-err58-cpp)
TANUKI_S11N_WRAP_EXPORT_IMPLEMENT(fooable::foo_model, fooable::foo_iface<int>)
