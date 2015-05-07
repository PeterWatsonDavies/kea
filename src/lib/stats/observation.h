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

#ifndef OBSERVATION_H
#define OBSERVATION_H

#include <boost/shared_ptr.hpp>
#include <exceptions/exceptions.h>
#include <cc/data.h>
#include <boost/date_time/time_duration.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <list>

namespace isc {
namespace stats {

/// @brief Exception thrown if invalid statistic type is used
///
/// For example statistic is of type duration, but methods using
/// it as integer are called.
class InvalidStatType : public Exception {
public:
    InvalidStatType(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) {}
};

/// @brief Defines duration resolution
///
/// Boost offers a base boost::posix_time::time_duration class, that has specific
/// implementations: boost::posix_time::{hours,minutes,seconds,millisec,nanosec}.
/// For statistics purposes, the most appropriate choice seems to be milliseconds
/// precision, so we'll stick with that.
typedef boost::posix_time::microsec::time_duration StatsDuration;

/// @defgroup stat_samples Specifies supported observation types.
///
/// @brief The list covers all supported types of observations.
///
/// @{

/// @brief Integer (implemented as unsigned 64-bit integer)
typedef std::pair<uint64_t, boost::posix_time::ptime> IntegerSample;

/// @brief Float (implemented as double precision)
typedef std::pair<double, boost::posix_time::ptime> FloatSample;

/// @brief Time Duration
typedef std::pair<StatsDuration, boost::posix_time::ptime> DurationSample;

/// @brief String
typedef std::pair<std::string, boost::posix_time::ptime> StringSample;

/// @}

/// @brief Represents a single observable characteristic (a 'statistic')
///
/// Currently it supports one of four types: integer (implemented as unsigned 64
/// bit integer), float (implemented as double), time duration (implemented with
/// millisecond precision) and string. Absolute (setValue) and
/// incremental (addValue) modes are supported. Statistic type is determined
/// during its first use. Once type is set, any additional observations recorded
/// must be of the same type. Attempting to set or extract information about
/// other types will result in InvalidStateType exception.
///
/// Observation can be retrieved in one of @ref getInteger, @ref getFloat,
/// @ref getDuration, @ref getString (appropriate type must be used) or
/// @ref getJSON, which is generic and can be used for all types.
///
/// @todo: Eventually it will be possible to retain multiple samples for the same
/// observation, but that is outside of scope for 0.9.2.
class Observation {
 public:

    /// @brief type of available statistics
    ///
    /// Note that those will later be exposed using control socket. Therefore
    /// an easy to understand names were chosen (integer instead of uint64).
    /// To avoid confusion, we will support only one type of integer and only
    /// one type of floating points. Initially, these are represented by
    /// uint64_t and double. If convincing use cases appear to change them
    /// to something else, we may change the underlying type.
    enum Type {
        STAT_INTEGER, ///< this statistic is unsinged 64-bit integer value
        STAT_FLOAT,   ///< this statistic is a floating point value
        STAT_DURATION,///< this statistic represents time duration
        STAT_STRING   ///< this statistic represents a string
    };

    /// @brief Constructor for integer observations
    ///
    /// @param name observation name
    /// @param value integer value observed.
    Observation(const std::string& name, uint64_t value);

    /// @brief Constructor for floating point observations
    ///
    /// @param name observation name
    /// @param value floating point value observed.
    Observation(const std::string& name, double value);

    /// @brief Constructor for duration observations
    ///
    /// @param name observation name
    /// @param value duration observed.
    Observation(const std::string& name, StatsDuration value);

    /// @brief Constructor for string observations
    ///
    /// @param name observation name
    /// @param value string observed.
    Observation(const std::string& name, const std::string& value);

    /// @brief Records absolute integer observation
    ///
    /// @param value integer value observed
    /// @throw InvalidStatType if statistic is not integer
    void setValue(uint64_t value);

    /// @brief Records absolute floating point observation
    ///
    /// @param value floating point value observed
    /// @throw InvalidStatType if statistic is not fp
    void setValue(double value);

    /// @brief Records absolute duration observation
    ///
    /// @param value duration value observed
    /// @throw InvalidStatType if statistic is not time duration
    void setValue(StatsDuration duration);

    /// @brief Records absolute string observation
    ///
    /// @param value string value observed
    /// @throw InvalidStatType if statistic is not a string
    void setValue(const std::string& value = "");

    /// @brief Records incremental integer observation
    ///
    /// @param value integer value observed
    /// @throw InvalidStatType if statistic is not integer
    void addValue(uint64_t value = 1);

    /// @brief Records inremental floating point observation
    ///
    /// @param value floating point value observed
    /// @throw InvalidStatType if statistic is not fp
    void addValue(double value = 1.0f);

    /// @brief Records incremental duration observation
    ///
    /// @param value duration value observed
    /// @throw InvalidStatType if statistic is not time duration
    void addValue(StatsDuration value = StatsDuration(0,0,0,0));

    /// @brief Records incremental string observation.
    ///
    /// @param value string value observed
    /// @throw InvalidStatType if statistic is not a string
    void addValue(const std::string& value = "");

    /// @brief Resets statistic.
    ///
    /// Sets statistic to a neutral (0, 0.0 or "") value.
    void reset();

    /// @brief Returns statistic type
    /// @return statistic type
    Type getType() const {
        return (type_);
    }

    /// @brief Returns observed integer sample
    /// @return observed sample (value + timestamp)
    /// @throw InvalidStatType if statistic is not integer
    IntegerSample getInteger();

    /// @brief Returns observed float sample
    /// @return observed sample (value + timestamp)
    /// @throw InvalidStatType if statistic is not fp
    FloatSample getFloat();

    /// @brief Returns observed duration sample
    /// @return observed sample (value + timestamp)
    /// @throw InvalidStatType if statistic is not time duration
    DurationSample getDuration();

    /// @brief Returns observed string sample
    /// @return observed sample (value + timestamp)
    /// @throw InvalidStatType if statistic is not a string
    StringSample getString();

    /// @brief Returns as a JSON structure
    /// @return JSON structures representing all observations
    isc::data::ConstElementPtr getJSON();

    /// @brief Converts statistic type to string
    /// @return textual name of statistic type
    static std::string typeToText(Type type);

    /// @brief Converts ptime structure to text
    /// @return a string representing time
    static std::string ptimeToText(boost::posix_time::ptime time);

    /// @brief Converts StatsDuration to text
    /// @return a string representing time
    static std::string durationToText(StatsDuration dur);

    /// @brief Returns observation name
    std::string getName() {
        return (name_);
    }

 protected:
    /// @brief Records absolute sample (internal version)
    ///
    /// This method records an absolute value of an observation.
    /// It is used by public methods to add sample to one of
    /// available storages.
    ///
    /// @tparam SampleType type of sample (e.g. IntegerSample)
    /// @tparam StorageType type of storage (e.g. list<IntegerSample>)
    /// @param value observation to be recorded
    /// @param storage observation will be stored here
    /// @param exp_type expected observation type (used for sanity checking)
    /// @throw InvalidStatType if observation type mismatches
    template<typename SampleType, typename StorageType>
    void setValueInternal(SampleType value, StorageType& storage,
                          Type exp_type);

    /// @brief Returns a sample
    ///
    /// @tparam SampleType type of sample (e.g. IntegerSample)
    /// @tparam StorageType type of storage (e.g. list<IntegerSample>)
    /// @param observation storage
    /// @param exp_type expected observation type (used for sanity checking)
    /// @throw InvalidStatType if observation type mismatches
    /// @return Observed sample
    template<typename SampleType, typename Storage>
    SampleType getValueInternal(Storage& storage, Type exp_type);

    /// @brief Observation (statistic) name
    std::string name_;

    /// @brief Observation (statistic) type)
    Type type_;

    /// @defgroup samples_storage Storage for supported observations
    ///
    /// @brief The following containers serve as a storage for all supported
    /// observation types.
    ///
    /// @{

    /// @brief Storage for integer samples
    std::list<IntegerSample> integer_samples_;

    /// @brief Storage for floating point samples
    std::list<FloatSample> float_samples_;

    /// @brief Storage for time duration samples
    std::list<DurationSample> duration_samples_;

    /// @brief Storage for string samples
    std::list<StringSample> string_samples_;
    /// @}
};

/// @brief Observation pointer
typedef boost::shared_ptr<Observation> ObservationPtr;

};
};

#endif // OBSERVATION_H
