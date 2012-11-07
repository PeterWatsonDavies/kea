// Copyright (C) 2010  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <stdint.h>

#include <sstream>
#include <ostream>

#include <util/buffer.h>
#include <dns/messagerenderer.h>
#include <dns/rrttl.h>

#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <cctype>

using namespace std;
using namespace isc::dns;
using namespace isc::util;

namespace {

// We wrap the C isalpha, because it seems to be overloaded with something.
// Then the find_if doesn't work.
bool
myIsalpha(char c) {
    return (isalpha(c));
}

// The conversion of units to their size
struct Unit {
    char unit;
    uint32_t multiply;
};

Unit units[] = {
    { 'S', 1 },
    { 'M', 60 },
    { 'H', 60 * 60 },
    { 'D', 24 * 60 * 60 },
    { 'W', 7 * 24 * 60 * 60 }
};

}

namespace isc {
namespace dns {

RRTTL::RRTTL(const std::string& ttlstr) {
    if (ttlstr.empty()) {
        isc_throw(InvalidRRTTL, "Empty TTL string");
    }
    // We use a larger data type during the computation. This is because
    // some compilers don't fail when out of range, so we check the range
    // ourselves later.
    int64_t val = 0;

    const string::const_iterator end = ttlstr.end();
    string::const_iterator pos = ttlstr.begin();

    try {
        while (pos != end) {
            // Find the first unit, if there's any.
            const string::const_iterator unit = find_if(pos, end, myIsalpha);
            // Default multiplication if no unit.
            uint32_t multiply = 1;
            if (unit != end) {
                // Find the unit and get the size.
                bool found = false;
                for (size_t i = 0; i < sizeof(units) / sizeof(*units); ++i) {
                    if (toupper(*unit) == units[i].unit) {
                        found = true;
                        multiply = units[i].multiply;
                        break;
                    }
                }
                if (!found) {
                    isc_throw(InvalidRRTTL, "Unknown unit used: " << *unit <<
                              "in: " << ttlstr);
                }
            }
            // Now extract the number, defaut to 1 if there's no digit
            if (unit == pos) {
                isc_throw(InvalidRRTTL, "Missing number in TTL: " << ttlstr);
            }
            const int64_t value = boost::lexical_cast<int64_t>(string(pos,
                                                                      unit));
            // Add what we found
            val += multiply * value;
            // The partial value is still in range (the value can only grow,
            // so if we get out now, it won't get better).
            //
            // Any valid uint32_t number must have at most 10 digits. If it
            // has more, it could wrap around the int64_t silently (at least
            // in theory, some compilers seem to throw from lexical_cast).
            if (unit - pos > 10 || value <= 0 || val <= 0 ||
                val >= 0xffffffff) {
                isc_throw(InvalidRRTTL, "Part of TTL out of range: " <<
                          ttlstr);
            }
            // Move to after the unit (if any). But make sure not to increment
            // past end, which is, strictly speaking, illegal.
            pos = unit;
            if (pos != end) {
                ++pos;
            }
        }
    } catch (const boost::bad_lexical_cast&) {
        isc_throw(InvalidRRTTL, "invalid TTL: " << ttlstr);
    }

    if (val >= 0 && val <= 0xffffffff) {
        ttlval_ = val;
    } else {
        isc_throw(InvalidRRTTL, "TTL out of range: " << ttlstr);
    }
}

RRTTL::RRTTL(InputBuffer& buffer) {
    if (buffer.getLength() - buffer.getPosition() < sizeof(uint32_t)) {
        isc_throw(IncompleteRRTTL, "incomplete wire-format TTL value");
    }
    ttlval_ = buffer.readUint32();
}

const string
RRTTL::toText() const {
    ostringstream oss;
    oss << ttlval_;
    return (oss.str());
}

void
RRTTL::toWire(OutputBuffer& buffer) const {
    buffer.writeUint32(ttlval_);
}

void
RRTTL::toWire(AbstractMessageRenderer& renderer) const {
    renderer.writeUint32(ttlval_);
}

ostream&
operator<<(ostream& os, const RRTTL& rrttl) {
    os << rrttl.toText();
    return (os);
}
}
}
