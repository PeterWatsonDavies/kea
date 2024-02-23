// Copyright (C) 2024 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <dhcp/pkt4.h>
#include <dhcp/pkt6.h>
#include <dhcp/dhcp6.h>
#include <exceptions/exceptions.h>
#include <monitored_duration.h>

using namespace isc::dhcp;
using namespace boost::posix_time;

namespace isc {
namespace perfmon {

// DurationDataInterval methods

DurationDataInterval::DurationDataInterval(const Timestamp& start_time /* = PktEvent::now()*/)
    : start_time_(start_time), occurrences_(0),
      min_duration_(pos_infin), max_duration_(neg_infin),
      total_duration_(microseconds(0)) {
}

void
DurationDataInterval::addDuration(const Duration& duration) {
    ++occurrences_;
    if (duration < min_duration_) {
        min_duration_ = duration;
    }

    if (duration > max_duration_) {
        max_duration_ = duration;
    }

    total_duration_ += duration;
}

Duration
DurationDataInterval::getAverageDuration() const {
    if (!occurrences_) {
        return (ZERO_DURATION());
    }

    return (total_duration_ / occurrences_);
}

// DurationKey methods

DurationKey::DurationKey(uint16_t family,
                         uint8_t query_type,
                         uint8_t response_type,
                         const std::string& start_event_label,
                         const std::string& end_event_label,
                         dhcp::SubnetID subnet_id)
    : family_(family),
      query_type_(query_type),
      response_type_(response_type),
      start_event_label_(start_event_label),
      end_event_label_(end_event_label),
      subnet_id_(subnet_id) {
    if (family != AF_INET && family != AF_INET6) {
        isc_throw (BadValue, "DurationKey: family must be AF_INET or AF_INET6");
    }

    validateMessagePair(family, query_type, response_type);
}

void
DurationKey::validateMessagePair(uint16_t family, uint8_t query_type, uint8_t response_type) {
    if (family == AF_INET) {
        switch(query_type) {
            case DHCP_NOTYPE:
                if (response_type == DHCP_NOTYPE ||
                    response_type == DHCPOFFER ||
                    response_type == DHCPACK ||
                    response_type == DHCPNAK) {
                        return;
                    }
                break;

            case DHCPDISCOVER:
                if (response_type == DHCP_NOTYPE ||
                    response_type == DHCPOFFER ||
                    response_type == DHCPNAK) {
                    return;
                }
                break;

            case DHCPREQUEST:
                if (response_type == DHCP_NOTYPE ||
                    response_type == DHCPACK ||
                    response_type == DHCPNAK) {
                    return;
                }
                break;

            case DHCPINFORM:
                if (response_type == DHCP_NOTYPE ||
                    response_type == DHCPACK) {
                    return;
                }
                break;

            default:
                isc_throw(BadValue, "Query type not supported by monitoring: "
                                    << Pkt4::getName(query_type));
                break;
        }

        isc_throw(BadValue, "Response type: " << Pkt4::getName(response_type)
                             << " not valid for query type: " << Pkt4::getName(query_type));

    } else {
        switch(query_type) {
            case DHCPV6_NOTYPE:
            case DHCPV6_SOLICIT:
                if (response_type == DHCPV6_NOTYPE ||
                    response_type == DHCPV6_ADVERTISE ||
                    response_type == DHCPV6_REPLY) {
                    return;
                }
                break;

            case DHCPV6_REQUEST:
            case DHCPV6_RENEW:
            case DHCPV6_REBIND:
            case DHCPV6_CONFIRM:
                if (response_type == DHCPV6_NOTYPE ||
                    response_type == DHCPV6_REPLY) {
                    return;
                }
                break;

            default:
                isc_throw(BadValue, "Query type not supported by monitoring: "
                                     << Pkt6::getName(query_type));
                break;
        }

        isc_throw(BadValue, "Response type: " << Pkt6::getName(response_type)
                             << " not valid for query type: " << Pkt6::getName(query_type));
    }
}

std::string
DurationKey::getLabel() const {
    std::ostringstream oss;
    if (family_ == AF_INET) {
        oss << (query_type_ == DHCP_NOTYPE ? "NONE" : Pkt4::getName(query_type_)) << "-"
            << (response_type_ == DHCP_NOTYPE ? "NONE" : Pkt4::getName(response_type_));
    } else {
        oss << (query_type_ == DHCPV6_NOTYPE ? "NONE" : Pkt6::getName(query_type_)) << "-"
            << (response_type_ == DHCPV6_NOTYPE ? "NONE" : Pkt6::getName(response_type_));
    }

    oss << "." << start_event_label_ << "-" << end_event_label_
        << "." << subnet_id_;

    return (oss.str());
};

bool
DurationKey::operator==(const DurationKey& other) const {
    return (
        (family_ == other.family_) &&
        (query_type_ == other.query_type_) &&
        (response_type_ == other.response_type_) &&
        (start_event_label_ == other.start_event_label_) &&
        (end_event_label_ == other.end_event_label_) &&
        (subnet_id_ == other.subnet_id_)
    );
}

bool
DurationKey::operator!=(const DurationKey& other) const {
    return (!(*this == other));
}

bool
DurationKey::operator<(const DurationKey& other) const {
    return (
        (family_ < other.family_) ||
        (query_type_ < other.query_type_) ||
        (response_type_ < other.response_type_) ||
        (start_event_label_ < other.start_event_label_) ||
        (end_event_label_ < other.end_event_label_) ||
        (subnet_id_ < other.subnet_id_)
    );
}


// MonitoredDuration methods

MonitoredDuration::MonitoredDuration(uint16_t family,
                                     uint8_t query_type,
                                     uint8_t response_type,
                                     const std::string& start_event_label,
                                     const std::string& end_event_label,
                                     dhcp::SubnetID subnet_id,
                                     const Duration& interval_duration)
    : DurationKey(family, query_type, response_type, start_event_label, end_event_label, subnet_id),
      interval_duration_(interval_duration),
      current_interval_(0),
      previous_interval_(0) {
    if (interval_duration_ <= DurationDataInterval::ZERO_DURATION()) {
        isc_throw(BadValue, "MonitoredDuration - interval_duration " << interval_duration_
                            << ", is invalid, it must be greater than 0");
    }
}

MonitoredDuration::MonitoredDuration(const DurationKey& key,
                                     const Duration& interval_duration)
    : DurationKey(key),
      interval_duration_(interval_duration),
      current_interval_(0),
      previous_interval_(0) {
    if (interval_duration_ <= DurationDataInterval::ZERO_DURATION()) {
        isc_throw(BadValue, "MonitoredDuration - interval_duration " << interval_duration_
                            << ", is invalid, it must be greater than 0");
    }
}

bool
MonitoredDuration::addSample(const Duration& sample) {
    auto now = PktEvent::now();
    bool do_report = false;
    if (!current_interval_) {
        current_interval_.reset(new DurationDataInterval(now));
    } else if ((now - current_interval_->getStartTime()) > interval_duration_) {
        previous_interval_ = current_interval_;
        do_report = true;
        current_interval_.reset(new DurationDataInterval(now));
    }

    current_interval_->addDuration(sample);
    return (do_report);
}

void
MonitoredDuration::clear() {
    current_interval_.reset();
    previous_interval_.reset();
}

} // end of namespace perfmon
} // end of namespace isc