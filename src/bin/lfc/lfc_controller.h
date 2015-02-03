// Copyright (C) 2015  Internet Systems Consortium, Inc. ("ISC")
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

#ifndef LFC_CONTROLLER_H
#define LFC_CONTROLLER_H

#include <exceptions/exceptions.h>
#include <string>

namespace isc {
namespace lfc {

/// @brief Exception thrown when the command line is invalid.
class InvalidUsage : public isc::Exception {
public:
    InvalidUsage(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) { };
};

/// @brief Exceptions thrown when the processing fails
class RunTimeFail : public isc::Exception {
public:
    RunTimeFail(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) { };
};

/// @brief Process controller for LFC process
///
/// This class provides the LFC process functions. These are used to:
/// manage the command line, check for already running instances,
/// invoke the code to process the lease files and finally to rename
/// the lease files as necessary.
///
/// @todo The current code simply processes the command line we still need to
/// -# invoke the code to read, process and write the lease files
/// -# rename and delete the shell files as required
class LFCController {
public:
    /// @brief Defines the application name, it may be used to locate
    /// configuration data and appears in log statements.
    static const char* lfc_app_name_;

    /// @brief Defines the executable name, by convention this should match
    /// the executable name.
    static const char* lfc_bin_name_;

    /// @brief Constructor
    LFCController();

    /// @brief Destructor
    ~LFCController();

    /// @brief Acts as the primary entry point to start execution
    /// of the process.  Provides the control logic:
    ///
    /// -# parse command line arguments
    /// -# verify that it is the only instance
    /// -# create pid file
    /// -# read leases files TBD
    /// -# write lease file TBD
    /// -# move leases files TBD
    /// -# cleanup artifacts TBD
    /// -# remove pid file
    /// -# exit to the caller
    ///
    /// @param argc Number of strings in the @c argv array.
    /// @param argv Array of arguments passed in via the program's main function.
    ///
    /// @throw InvalidUsage if the command line parameters are invalid.
    void launch(int argc, char* argv[]);

    /// @brief Process the command line arguments.  It is the first
    /// step taken after the process has been launched.
    ///
    /// @param argc Number of strings in the @c argv array.
    /// @param argv Array of arguments passed in via the program's main function.
    ///
    /// @throw InvalidUsage if the command line parameters are invalid.
    void parseArgs(int argc, char* argv[]);

    /// @brief Prints the program usage text to std error.
    ///
    /// @param text is a string message which will preceded the usage text.
    /// This is intended to be used for specific usage violation messages.
    void usage(const std::string& text);

    /// @brief Gets the Kea version number for printing
    ///
    /// @param extended is a boolean indicating if the version string
    /// should be short (false) or extended (true)
    std::string getVersion(const bool extended) const;

    /// @name Accessor methods mainly used for testing purposes
    //@{

    /// @brief Gets the protocol version of the leaes files
    ///
    /// @return Returns the value of the DHCP protocol version.
    /// This can be 4 or 6 while in use and 0 before parsing
    /// any arguments.
    int getProtocolVersion() const {
      return (protocol_version_);
    }

    /// @brief Gets the config file name
    ///
    /// @return Returns the path to the config file
    std::string getConfigFile() const {
        return (config_file_);
    }

    /// @brief Gets the previous file name
    ///
    /// @return Returns the path to the previous file
    std::string getPreviousFile() const {
        return (previous_file_);
    }

    /// @brief Gets the copy file name
    ///
    /// @return Returns the path to the copy file
    std::string getCopyFile() const {
        return (copy_file_);
    }

    /// @brief Gets the output file name
    ///
    /// @return Returns the path to the output file
    std::string getOutputFile() const {
        return (output_file_);
    }

    /// @brief Gets the finish file name
    ///
    /// @return Returns the path to the finish file
    std::string getFinishFile() const {
        return (finish_file_);
    }

    /// @brief Gets the pid file name
    ///
    /// @return Returns the path to the pid file
    std::string getPidFile() const {
        return (pid_file_);
    }

    /// @brief Process files.  Read in the leases from any previous & copy
    /// files we have and write the results out to the output file.  Upon
    /// completion of the write move the file to the finish file.
    template<typename LeaseObjectType, typename LeaseFileType, typename StorageType>
    void processLeases() const;

    /// @brief Cleanup files.  After we have a finish file, either from
    /// doing the cleanup or because a previous instance was interrupted,
    /// delete the work files (previous & copy) and move the finish file
    /// to be the new previous file.
    ///
    /// @throw RunTimeFail if the command line parameters are invalid.
    void fileCleanup() const;
    //@}

private:
    /// Version of the DHCP protocol used, i.e. 4 or 6.
    int protocol_version_;
    /// When true output the result of parsing the comamnd line
    bool verbose_;
    std::string config_file_;   ///< The path to the config file
    std::string previous_file_; ///< The path to the previous LFC file (if any)
    std::string copy_file_;     ///< The path to the copy of the lease file
    std::string output_file_;   ///< The path to the output file
    std::string finish_file_;   ///< The path to the finished output file
    std::string pid_file_;      ///< The path to the pid file
};

}; // namespace isc::lfc
}; // namespace isc

#endif // LFC_CONTROLLER_H
