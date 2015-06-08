// Copyright (C) 2015 Internet Systems Consortium, Inc. ("ISC")
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

#include <exceptions/exceptions.h>
#include <stats/stats_mgr.h>
#include <cc/data.h>
#include <config/command_interpreter.h>

using namespace std;
using namespace isc::data;
using namespace isc::config;

namespace isc {
namespace stats {

StatsMgr& StatsMgr::instance() {
    static StatsMgr stats_mgr;
    return (stats_mgr);
}

StatsMgr::StatsMgr()
    :global_(new StatContext()) {

}

void StatsMgr::setValue(const std::string& name, const uint64_t value) {
    setValueInternal(name, value);
}

void StatsMgr::setValue(const std::string& name, const double value) {
    setValueInternal(name, value);
}

void StatsMgr::setValue(const std::string& name, const StatsDuration& value) {
    setValueInternal(name, value);
}
void StatsMgr::setValue(const std::string& name, const std::string& value) {
    setValueInternal(name, value);
}

void StatsMgr::addValue(const std::string& name, const uint64_t value) {
    addValueInternal(name, value);
}

void StatsMgr::addValue(const std::string& name, const double value) {
    addValueInternal(name, value);
}

void StatsMgr::addValue(const std::string& name, const StatsDuration& value) {
    addValueInternal(name, value);
}

void StatsMgr::addValue(const std::string& name, const std::string& value) {
    addValueInternal(name, value);
}

ObservationPtr StatsMgr::getObservation(const std::string& name) const {
    /// @todo: Implement contexts.
    // Currently we keep everyting in a global context.
    return (global_->get(name));
}

void StatsMgr::addObservation(const ObservationPtr& stat) {
    /// @todo: Implement contexts.
    // Currently we keep everyting in a global context.
    return (global_->add(stat));
}

bool StatsMgr::deleteObservation(const std::string& name) {
    /// @todo: Implement contexts.
    // Currently we keep everyting in a global context.
    return (global_->del(name));
}

void StatsMgr::setMaxSampleAge(const std::string& ,
                               const StatsDuration&) {
    isc_throw(NotImplemented, "setMaxSampleAge not implemented");
}

void StatsMgr::setMaxSampleCount(const std::string& , uint32_t){
    isc_throw(NotImplemented, "setMaxSampleCount not implemented");
}

bool StatsMgr::reset(const std::string& name) {
    ObservationPtr obs = getObservation(name);
    if (obs) {
        obs->reset();
        return (true);
    } else {
        return (false);
    }
}

bool StatsMgr::del(const std::string& name) {
    return (global_->del(name));
}

void StatsMgr::removeAll() {
    global_->stats_.clear();
}

isc::data::ConstElementPtr StatsMgr::get(const std::string& name) const {
    isc::data::ElementPtr response = isc::data::Element::createMap(); // a map
    ObservationPtr obs = getObservation(name);
    if (obs) {
        response->set(name, obs->getJSON()); // that contains the observation
    }
    return (response);
}

isc::data::ConstElementPtr StatsMgr::getAll() const {
    isc::data::ElementPtr map = isc::data::Element::createMap(); // a map

    // Let's iterate over all stored statistics...
    for (std::map<std::string, ObservationPtr>::iterator s = global_->stats_.begin();
         s != global_->stats_.end(); ++s) {

        // ... and add each of them to the map.
        map->set(s->first, s->second->getJSON());
    }
    return (map);
}

void StatsMgr::resetAll() {
    // Let's iterate over all stored statistics...
    for (std::map<std::string, ObservationPtr>::iterator s = global_->stats_.begin();
         s != global_->stats_.end(); ++s) {

        // ... and reset each statistic.
        s->second->reset();
    }
}

size_t StatsMgr::count() const {
    return (global_->stats_.size());
}

isc::data::ConstElementPtr
StatsMgr::statisticGetHandler(const std::string& /*name*/,
                              const isc::data::ConstElementPtr& params) {
    if (!params) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "Missing mandatory 'name' parameter."));
    }
    ConstElementPtr stat_name = params->get("name");
    if (!stat_name) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "Missing mandatory 'name' parameter."));
    }
    if (stat_name->getType() != Element::string) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "'name' parameter expected to be a string."));
    }
    return (createAnswer(CONTROL_RESULT_SUCCESS, get(stat_name->stringValue())));
}

isc::data::ConstElementPtr
StatsMgr::statisticResetHandler(const std::string& /*name*/,
                                const isc::data::ConstElementPtr& params) {
    if (!params) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "Missing mandatory 'name' parameter."));
    }
    ConstElementPtr stat_name = params->get("name");
    if (!stat_name) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "Missing mandatory 'name' parameter."));
    }
    if (stat_name->getType() != Element::string) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "'name' parameter expected to be a string."));
    }

    if (reset(stat_name->stringValue())) {
        return (createAnswer(CONTROL_RESULT_SUCCESS,
                             "Statistic '" + stat_name->stringValue() + "' reset."));
    } else {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "No '" + stat_name->stringValue() + "' statistic found"));
    }
}

isc::data::ConstElementPtr
StatsMgr::statisticRemoveHandler(const std::string& /*name*/,
                                 const isc::data::ConstElementPtr& params) {
    if (!params) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "Missing mandatory 'name' parameter."));
    }
    ConstElementPtr stat_name = params->get("name");
    if (!stat_name) {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "Missing mandatory 'name' parameter."));
    }
    if (del(stat_name->stringValue())) {
        return (createAnswer(CONTROL_RESULT_SUCCESS,
                             "Statistic '" + stat_name->stringValue() + "' removed."));
    } else {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "No '" + stat_name->stringValue() + "' statistic found"));
    }

}

isc::data::ConstElementPtr
StatsMgr::statisticRemoveAllHandler(const std::string& /*name*/,
                                    const isc::data::ConstElementPtr& /*params*/) {
    removeAll();
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         "All statistics removed."));
}

isc::data::ConstElementPtr
StatsMgr::statisticGetAllHandler(const std::string& /*name*/,
                                 const isc::data::ConstElementPtr& /*params*/) {
    ConstElementPtr all_stats = getAll();
    return (createAnswer(CONTROL_RESULT_SUCCESS, all_stats));
}

isc::data::ConstElementPtr
StatsMgr::statisticResetAllHandler(const std::string& /*name*/,
                                   const isc::data::ConstElementPtr& /*params*/) {
    resetAll();
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         "All statistics reset to neutral values."));
}


};
};
