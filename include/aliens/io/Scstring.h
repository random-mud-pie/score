#pragma once

#include "aliens/vendored/seastar/core/sstring.hh"

namespace aliens { namespace io {

using Scstring = vendored::seastar::basic_sstring<char, uint32_t, 31>;

}} // aliens::io