// Copyright (C) 2012 Internet Systems Consortium, Inc. ("ISC")
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

#ifndef __TEST_CONTROL_H
#define __TEST_CONTROL_H

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <dhcp/pkt4.h>
#include <dhcp/pkt6.h>

namespace isc {
namespace perfdhcp {

/// \brief Test Control class.
///
/// This class is responsible for executing DHCP performance
/// test end to end.
///
class TestControl : public boost::noncopyable {
public:

    /// \brief Socket wrapper class.
    ///
    /// This is wrapper class that holds descriptor of the socket
    /// used to run DHCP test. All sockets created with \ref IfaceMgr
    /// are closed in the destructor. This ensures that socket is
    /// closed when the function that created the socket ends
    /// (normally or when exception occurs).
    class TestControlSocket {
    public:

        /// \brief Constructor of socket wrapper class.
        ///
        /// This constructor uses provided socket descriptor to
        /// find the name of the interface where socket has been
        /// bound to.
        ///
        /// \param socket socket descriptor.
        /// \throw isc::BadValue if interface for specified
        /// socket descriptor does not exist.
        TestControlSocket(const int socket);

        /// \brief Destriuctor of the socket wrapper class.
        ///
        /// Destructor closes all open sockets on all interfaces.
        /// TODO: close only the socket being wrapped by this class.
        ~TestControlSocket();

        /// \brief Return name of the interface where socket is bound to.
        ///
        /// \return name of the interface where socket is bound to.
        const std::string& getIface() const { return(iface_); }

    private:
        /// \brief Private default constructor.
        ///
        /// Default constructor is private to make sure that valid
        /// socket descriptor is passed to create object.
        TestControlSocket();

        /// \brief Initialize name of the interface.
        ///
        /// This method intializes the name of the interface where
        /// socket is bound to. This name can be later retreived by
        /// client class to set interface name in DHCP packet objects.
        ///
        /// \throw isc::BadValue if interface for specified socket
        /// descriptor does not exist.
        void initInterface();

        int socket_;          ///< Socket descirptor.
        std::string iface_;   ///< Name of the interface.
    };

    /// \brief Length of the Ethernet HW address (MAC) in bytes.
    static const uint8_t HW_ETHER_LEN = 6;

    /// TestControl is a singleton class. This method returns reference
    /// to its sole instance.
    ///
    /// \return the only existing instance of test control
    static TestControl& instance();

    /// Run performance test.
    ///
    /// Method runs whole performance test. Command line options must
    /// be parsed prior to running this function. Othewise function will
    /// throw exception.
    ///
    /// \throw isc::InvalidOperation if command line options are not parsed.
    /// \throw isc::Unexpected if internal Test Controler error occured.
    void run();

private:

    /// \brief Private default constructor.
    ///
    /// Default constructor is private as the object can be created
    /// only via \ref instance method.
    TestControl();

    /// \brief Check if test exit condtitions fulfiled.
    ///
    /// Method checks if test exit conditions are fulfiled.
    /// Exit conditions are checked periodically from the
    /// main loop. Program should break the main loop when
    /// this method returns true. It is calling function
    /// responsibility to break main loop gracefully and
    /// cleanup after test execution.
    ///
    /// \return true if any of the exit conditions is fulfiled.
    bool checkExitConditions() const;

    /// \brief Create DHCPv4 DISCOVER packet.
    ///
    /// Create instance of DHCPv4 DISCOVER packet with ethernet
    /// HW type and MAC address specified as parameter. The following
    /// DHCP options are added to the packet:
    /// - DHO_DHCP_MESSAGE_TYPE with DHCPDISCOVER message type value
    /// - DHO_DHCP_PARAMETER_REQUEST_LIST with the following options
    /// being requested from the server:
    ///    - DHO_SUBNET_MASK,
    ///    - DHO_BROADCAST_ADDRESS,
    ///    - DHO_TIME_OFFSET,
    ///    - DHO_ROUTERS,
    ///    - DHO_DOMAIN_NAME,
    ///    - DHO_DOMAIN_NAME_SERVERS,
    ///    - DHO_HOST_NAME.
    ///
    /// \param mac_addr MAC address to be set for the packet. MAC address
    /// has to be exactly 6 octets long.
    /// \throw isc::Unexpected if failed to create new packet instance.
    /// \throw isc::BadValue if MAC address has invalid length.
    /// \return insance of the DHCPv4 DISCOVER packet.
    boost::shared_ptr<dhcp::Pkt4>
    createDiscoverPkt4(const std::vector<uint8_t>& mac_addr) const;

    /// \brief Factory function to create generic option.
    ///
    /// Factory function is registered using \ref LibDHCP::OptionFactoryRegister.
    /// Registered factory functions provide a way to create options of the
    /// same type in the same way. When new option instance is needed the
    /// corresponding factory function is called to create it. This is done
    /// by calling \ref Option::factory with DHCP message type specified as
    /// one of parameters. Some of the parameters passed to factory function
    /// may be ignored (e.g. option buffer). For generic option however, factory
    /// function creates option using contents of the buffer.
    ///
    /// \param u universe (V6 or V4).
    /// \param type option-type.
    /// \param buf option-buffer.
    /// \return instance o the generic option.
    static dhcp::OptionPtr factoryGeneric4(dhcp::Option::Universe u,
                                           uint16_t type,
                                           const dhcp::OptionBuffer& buf);

    /// \brief Factory function to create DHCPv4 Request List option.
    ///
    /// Factory function is registered using \ref LibDHCP::OptionFactoryRegister.
    /// Registered factory functions provide a way to create options of the
    /// same type in the same way. When new option instance is needed the
    /// corresponding factory function is called to create it. This is done
    /// by calling \ref Option::factory with DHCP message type specified as
    /// one of parameters. This factory function ignores contents of the
    /// buffer provided and creates option buffer internally with the following
    /// list of requested options:
    /// - DHO_SUBNET_MASK,
    /// - DHO_BROADCAST_ADDRESS,
    /// - DHO_TIME_OFFSET,
    /// - DHO_ROUTERS,
    /// - DHO_DOMAIN_NAME,
    /// - DHO_DOMAIN_NAME_SERVERS,
    /// - DHO_HOST_NAME.
    ///
    /// \param u universe (V6 or V4).
    /// \param type option-type.
    /// \param buf option-buffer.
    /// \return instance o the generic option.
     static dhcp::OptionPtr factoryRequestList4(dhcp::Option::Universe u,
                                               uint16_t type,
                                               const dhcp::OptionBuffer& buf);

    /// \brief Generate MAC address.
    ///
    /// This method generates MAC address. The number of unique
    /// MAC addresses it can generate is determined by the number
    /// simulated DHCP clients specified from command line. It uses
    /// \ref CommandOptions object to retrieve number of clients.
    /// Based on this the random value is generated and added to
    /// the MAC address prefix (default MAC address).
    ///
    /// \return generated MAC address.
    std::vector<uint8_t> generateMacAddress() const;

    /// \brief Returns number of exchanges to be started.
    ///
    /// Method returns number of new exchanges to be started as soon
    /// as possible to satisfy expected rate. Calculation used here
    /// is based on current time, due time calculated with
    /// \ref updateSendTime function and expected rate.
    ///
    /// \return number of exchanges to be started immediatelly.
    uint64_t getNextExchangesNum() const;

    /// \brief Open socket to communicate with DHCP server.
    ///
    /// Method opens socket and binds it to local address. Function will
    /// can use either interface name, local address or server address
    /// to create a socket, depending on what is available (specified
    /// from the command line). If socket can't be created for any
    /// reason, exception is thrown.
    ///
    /// \throw isc::BadValue if socket can't be created.
    /// \return socket descriptor.
    int openSocket() const;

    void receivePackets();

    /// \brief Register option factory functions for DHCPv4
    ///
    /// Method registers option factory functions for DHCPv4.
    /// These functions are called to create instances of DHCPv4
    /// options. Call \ref Option::factory to invoke factory
    /// function for particular option. Don't use this function directly.
    /// Use \ref registerOptionFactories instead.
    void registerOptionFactories4() const;

    /// \brief Register option factory functions for DHCPv6
    ///
    /// Method registers option factory functions for DHCPv6.
    /// These functions are called to create instances of DHCPv6
    /// options. Call \ref Option::factory to invoke factory
    /// function for particular option. Don't use this function directly.
    /// Use \ref registerOptionFactories instead.
    void registerOptionFactories6() const;

    /// \brief Register option factory functions for DHCPv4 or DHCPv6.
    ///
    /// Method registers option factory functions for DHCPv4 or DHCPv6,
    /// depending in whch mode test is currently running.
     void registerOptionFactories() const;

    /// \brief Start new exchange of DHCP messages.
    ///
    /// Method starts new DHCP exchange by sending new DHCPv4 DISCOVER
    /// or DHCPv6 SOLICIT packet to the server.
    ///
    /// \param socket socket used to send DHCP message.
    /// \throw isc::Unexpected if failed to create or send packet
    void startExchange(const TestControlSocket& socket);

    /// \brief Update due time to initiate next chunk of exchanges.
    ///
    /// Method updates due time to initiate next chunk of exchanges.
    /// Function takes current time, last sent packet's time and
    /// expected rate in its calculations.
    void updateSendDue();

    boost::posix_time::ptime send_due_;    ///< Due time to initiate next chunk
                                           ///< of exchanges.
    boost::posix_time::ptime last_sent_;   ///< Indicates when the last exchange
                                           /// was initiated.

    uint64_t sent_packets_0_;
    uint64_t sent_packets_1_;
};

} // namespace perfdhcp
} // namespace isc

#endif // __COMMAND_OPTIONS_H
