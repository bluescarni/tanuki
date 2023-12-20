// Copyright 2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the tanuki library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <tanuki/tanuki.hpp>

#include "external_inst.hpp"

namespace fooable
{

// LCOV_EXCL_START

void foo_model::foo() const {}

// LCOV_EXCL_STOP

} // namespace fooable

namespace tanuki
{

template class wrap<fooable::foo_iface<int>, fooable::foo_wrap_config<int>>;

} // namespace tanuki
