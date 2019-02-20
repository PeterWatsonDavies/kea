// Copyright (C) 2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef CONFIG_BACKEND_DHCP6_H
#define CONFIG_BACKEND_DHCP6_H

#include <cc/stamped_value.h>
#include <config_backend/base_config_backend.h>
#include <database/audit_entry.h>
#include <database/server_selector.h>
#include <dhcp/option.h>
#include <dhcp/option_definition.h>
#include <dhcpsrv/cfg_option.h>
#include <dhcpsrv/shared_network.h>
#include <dhcpsrv/subnet.h>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <string>

namespace isc {
namespace dhcp {

/// @brief Interface implemented by DHCPv6 configuration backends.
///
/// All POSIX times specified in the methods belonging to this
/// class must be local times.
class ConfigBackendDHCPv6 : public cb::BaseConfigBackend {
public:

    /// @brief Virtual destructor.
    virtual ~ConfigBackendDHCPv6() { }

    /// @brief Retrieves a single subnet by subnet_prefix.
    ///
    /// @param server_selector Server selector.
    /// @param subnet_prefix Prefix of the subnet to be retrieved.
    /// @return Pointer to the retrieved subnet or NULL if not found.
    virtual Subnet6Ptr
    getSubnet6(const db::ServerSelector& server_selector,
               const std::string& subnet_prefix) const = 0;

    /// @brief Retrieves a single subnet by subnet identifier.
    ///
    /// @param server_selector Server selector.
    /// @param subnet_id Identifier of a subnet to be retrieved.
    /// @return Pointer to the retrieved subnet or NULL if not found.
    virtual Subnet6Ptr
    getSubnet6(const db::ServerSelector& server_selector, const SubnetID& subnet_id) const = 0;

    /// @brief Retrieves all subnets.
    ///
    /// @param server_selector Server selector.
    /// @return Collection of subnets or empty collection if no subnet found.
    virtual Subnet6Collection
    getAllSubnets6(const db::ServerSelector& server_selector) const = 0;

    /// @brief Retrieves all subnets belonging to a specified shared network.
    ///
    /// @param server_selector Server selector.
    /// @param shared_network_name Name of the shared network for which the
    /// subnets should be retrieved.
    /// @return Collection of subnets or empty collection if no subnet found.
    virtual Subnet6Collection
    getSharedNetworkSubnets6(const db::ServerSelector& server_selector,
                             const std::string& shared_network_name) const = 0;

    /// @brief Retrieves subnets modified after specified time.
    ///
    /// @param server_selector Server selector.
    /// @param modification_time Lower bound subnet modification time.
    /// @return Collection of subnets or empty collection if no subnet found.
    virtual Subnet6Collection
    getModifiedSubnets6(const db::ServerSelector& server_selector,
                        const boost::posix_time::ptime& modification_time) const = 0;

    /// @brief Retrieves shared network by name.
    ///
    /// @param server_selector Server selector.
    /// @param name Name of the shared network to be retrieved.
    /// @return Pointer to the shared network or NULL if not found.
    virtual SharedNetwork6Ptr
    getSharedNetwork6(const db::ServerSelector& server_selector,
                      const std::string& name) const = 0;

    /// @brief Retrieves all shared networks.
    ///
    /// @param server_selector Server selector.
    /// @return Collection of shared network or empty collection if
    /// no shared network found.
    virtual SharedNetwork6Collection
    getAllSharedNetworks6(const db::ServerSelector& server_selector) const = 0;

    /// @brief Retrieves shared networks modified after specified time.
    ///
    /// @param server_selector Server selector.
    /// @param modification_time Lower bound shared network modification time.
    /// @return Collection of shared network or empty collection if
    /// no shared network found.
    virtual SharedNetwork6Collection
    getModifiedSharedNetworks6(const db::ServerSelector& server_selector,
                               const boost::posix_time::ptime& modification_time) const = 0;

    /// @brief Retrieves single option definition by code and space.
    ///
    /// @param server_selector Server selector.
    /// @param code Code of the option to be retrieved.
    /// @param space Option space of the option to be retrieved.
    /// @return Pointer to the option definition or NULL if not found.
    virtual OptionDefinitionPtr
    getOptionDef6(const db::ServerSelector& server_selector, const uint16_t code,
                  const std::string& space) const = 0;

    /// @brief Retrieves all option definitions.
    ///
    /// @param server_selector Server selector.
    /// @return Collection of option definitions or empty collection if
    /// no option definition found.
    virtual OptionDefContainer
    getAllOptionDefs6(const db::ServerSelector& server_selector) const = 0;

    /// @brief Retrieves option definitions modified after specified time.
    ///
    /// @param server_selector Server selector.
    /// @param modification_time Lower bound option definition modification
    /// time.
    /// @return Collection of option definitions or empty collection if
    /// no option definition found.
    virtual OptionDefContainer
    getModifiedOptionDefs6(const db::ServerSelector& server_selector,
                           const boost::posix_time::ptime& modification_time) const = 0;

    /// @brief Retrieves single option by code and space.
    ///
    /// @param server_selector Server selector.
    /// @param code Option code.
    /// @param space Option space.
    /// @return Pointer to the retrieved option descriptor or null if
    /// no option was found.
    virtual OptionDescriptorPtr
    getOption6(const db::ServerSelector& server_selector, const uint16_t code,
               const std::string& space) const = 0;

    /// @brief Retrieves all global options.
    ///
    /// @param server_selector Server selector.
    /// @return Collection of global options or empty collection if no
    /// option found.
    virtual OptionContainer
    getAllOptions6(const db::ServerSelector& server_selector) const = 0;

    /// @brief Retrieves options modified after specified time.
    ///
    /// @param selector Server selector.
    /// @param modification_time Lower bound option modification time.
    /// @return Collection of global options or empty collection if no
    /// option found.
    virtual OptionContainer
    getModifiedOptions6(const db::ServerSelector& selector,
                        const boost::posix_time::ptime& modification_time) const = 0;

    /// @brief Retrieves global parameter value.
    ///
    /// @param selector Server selector.
    /// @param name Name of the global parameter to be retrieved.
    /// @return Value of the global parameter or null if parameter doesn't
    /// exist.
    virtual data::StampedValuePtr
    getGlobalParameter6(const db::ServerSelector& selector,
                        const std::string& name) const = 0;

    /// @brief Retrieves all global parameters.
    ///
    /// @param selector Server selector.
    /// @return Collection of global parameters.
    virtual data::StampedValueCollection
    getAllGlobalParameters6(const db::ServerSelector& selector) const = 0;

    /// @brief Retrieves global parameters modified after specified time.
    ///
    /// @param selector Server selector.
    /// @param modification_time Modification time.
    /// @return Collection of modified global parameters.
    virtual data::StampedValueCollection
    getModifiedGlobalParameters6(const db::ServerSelector& selector,
                                 const boost::posix_time::ptime& modification_time) const = 0;

    /// @brief Retrieves the most recent audit entries.
    ///
    /// @param server_selector Server selector.
    /// @param modification_time Timestamp being a lower limit for the returned
    /// result set, i.e. entries later than specified time are returned.
    /// @return Collection of audit entries.
    virtual db::AuditEntryCollection
    getRecentAuditEntries6(const db::ServerSelector& server_selector,
                           const boost::posix_time::ptime& modification_time) const = 0;

    /// @brief Creates or updates a subnet.
    ///
    /// @param server_selector Server selector.
    /// @param subnet Subnet to be added or updated.
    virtual void
    createUpdateSubnet6(const db::ServerSelector& server_selector,
                        const Subnet6Ptr& subnet) = 0;

    /// @brief Creates or updates a shared network.
    ///
    /// @param server_selector Server selector.
    /// @param shared_network Shared network to be added or updated.
    virtual void
    createUpdateSharedNetwork6(const db::ServerSelector& server_selector,
                               const SharedNetwork6Ptr& shared_network) = 0;

    /// @brief Creates or updates an option definition.
    ///
    /// @param server_selector Server selector.
    /// @param option_def Option definition to be added or updated.
    virtual void
    createUpdateOptionDef6(const db::ServerSelector& server_selector,
                           const OptionDefinitionPtr& option_def) = 0;

    /// @brief Creates or updates global option.
    ///
    /// @param server_selector Server selector.
    /// @param option Option to be added or updated.
    virtual void
    createUpdateOption6(const db::ServerSelector& server_selector,
                        const OptionDescriptorPtr& option) = 0;

    /// @brief Creates or updates shared network level option.
    ///
    /// @param selector Server selector.
    /// @param shared_network_name Name of a shared network to which option
    /// belongs.
    /// @param option Option to be added or updated.
    virtual void
    createUpdateOption6(const db::ServerSelector& selector,
                        const std::string& shared_network_name,
                        const OptionDescriptorPtr& option) = 0;

    /// @brief Creates or updates subnet level option.
    ///
    /// @param server_selector Server selector.
    /// @param subnet_id Identifier of a subnet to which option belongs.
    /// @param option Option to be added or updated.
    virtual void
    createUpdateOption6(const db::ServerSelector& server_selector,
                        const SubnetID& subnet_id,
                        const OptionDescriptorPtr& option) = 0;

    /// @brief Creates or updates pool level option.
    ///
    /// @param server_selector Server selector.
    /// @param pool_start_address Lower bound address of the pool to which
    /// the option belongs.
    /// @param pool_end_address Upper bound address of the pool to which the
    /// option belongs.
    /// @param option Option to be added or updated.
    virtual void
    createUpdateOption6(const db::ServerSelector& server_selector,
                        const asiolink::IOAddress& pool_start_address,
                        const asiolink::IOAddress& pool_end_address,
                        const OptionDescriptorPtr& option) = 0;

    /// @brief Creates or updates pd pool level option.
    ///
    /// @param server_selector Server selector.
    /// @param pd_pool_prefix Address part of the prefix of the pd pool
    /// to which the option belongs.
    /// @param pd_pool_prefix_length Prefix length of the pd pool to which
    /// the option belongs.
    /// @param option Option to be added or updated.
    virtual void
    createUpdateOption6(const db::ServerSelector& server_selector,
                        const asiolink::IOAddress& pd_pool_prefix,
                        const uint8_t pd_pool_prefix_length,
                        const OptionDescriptorPtr& option) = 0;

    /// @brief Creates or updates global parameter.
    ///
    /// @param server_selector Server selector.
    /// @param value Value of the global parameter.
    virtual void
    createUpdateGlobalParameter6(const db::ServerSelector& server_selector,
                                 const data::StampedValuePtr& value) = 0;

    /// @brief Deletes subnet by prefix.
    ///
    /// @param server_selector Server selector.
    /// @param subnet_prefix Prefix of the subnet to be deleted.
    /// @return Number of deleted subnets.
    virtual uint64_t
    deleteSubnet6(const db::ServerSelector& server_selector,
                  const std::string& subnet_prefix) = 0;

    /// @brief Deletes subnet by identifier.
    ///
    /// @param server_selector Server selector.
    /// @param subnet_id Identifier of the subnet to be deleted.
    /// @return Number of deleted subnets.
    virtual uint64_t
    deleteSubnet6(const db::ServerSelector& server_selector, const SubnetID& subnet_id) = 0;

    /// @brief Deletes all subnets.
    ///
    /// @param server_selector Server selector.
    /// @return Number of deleted subnets.
    virtual uint64_t
    deleteAllSubnets6(const db::ServerSelector& server_selector) = 0;

    /// @brief Deletes shared network by name.
    ///
    /// @param server_selector Server selector.
    /// @param name Name of the shared network to be deleted.
    /// @return Number of deleted shared networks..
    virtual uint64_t
    deleteSharedNetwork6(const db::ServerSelector& server_selector,
                         const std::string& name) = 0;

    /// @brief Deletes all shared networks.
    ///
    /// @param server_selector Server selector.
    /// @return Number of deleted shared networks.
    virtual uint64_t
    deleteAllSharedNetworks6(const db::ServerSelector& server_selector) = 0;

    /// @brief Deletes option definition.
    ///
    /// @param server_selector Server selector.
    /// @param code Code of the option to be deleted.
    /// @param space Option space of the option to be deleted.
    /// @return Number of deleted option definitions.
    virtual uint64_t
    deleteOptionDef6(const db::ServerSelector& server_selector, const uint16_t code,
                     const std::string& space) = 0;

    /// @brief Deletes all option definitions.
    ///
    /// @param server_selector Server selector.
    /// @return Number of deleted option definitions.
    virtual uint64_t
    deleteAllOptionDefs6(const db::ServerSelector& server_selector) = 0;

    /// @brief Deletes global option.
    ///
    /// @param server_selector Server selector.
    /// @param code Code of the option to be deleted.
    /// @param space Option space of the option to be deleted.
    /// @return Number of deleted options.
    virtual uint64_t
    deleteOption6(const db::ServerSelector& server_selector, const uint16_t code,
                  const std::string& space) = 0;

    /// @brief Deletes shared network level option.
    ///
    /// @param selector Server selector.
    /// @param shared_network_name Name of the shared network which option
    /// belongs to.
    /// @param code Code of the option to be deleted.
    /// @param space Option space of the option to be deleted.
    virtual uint64_t
    deleteOption6(const db::ServerSelector& selector,
                  const std::string& shared_network_name,
                  const uint16_t code,
                  const std::string& space) = 0;

    /// @brief Deletes subnet level option.
    ///
    /// @param server_selector Server selector.
    /// @param subnet_id Identifier of the subnet to which deleted option
    /// belongs.
    /// @param code Code of the deleted option.
    /// @param space Option space of the deleted option.
    /// @return Number of deleted options.
    virtual uint64_t
    deleteOption6(const db::ServerSelector& server_selector, const SubnetID& subnet_id,
                  const uint16_t code, const std::string& space) = 0;

    /// @brief Deletes pool level option.
    ///
    /// @param server_selector Server selector.
    /// @param pool_start_address Lower bound address of the pool to which
    /// deleted option belongs.
    /// @param pool_end_address Upper bound address of the pool to which the
    /// deleted option belongs.
    /// @param code Code of the deleted option.
    /// @param space Option space of the deleted option.
    /// @return Number of deleted options.
    virtual uint64_t
    deleteOption6(const db::ServerSelector& server_selector,
                  const asiolink::IOAddress& pool_start_address,
                  const asiolink::IOAddress& pool_end_address,
                  const uint16_t code,
                  const std::string& space) = 0;

    /// @brief Deletes pd pool level option.
    ///
    /// @param server_selector Server selector.
    /// @param pd_pool_prefix Address part of the prefix of the pd pool
    /// to which the deleted option belongs.
    /// @param pd_pool_prefix_length Prefix length of the pd pool to which
    /// the deleted option belongs.
    /// @param code Code of the deleted option.
    /// @param space Option space of the deleted option.
    /// @return Number of deleted options.
    virtual uint64_t
    deleteOption6(const db::ServerSelector& server_selector,
                  const asiolink::IOAddress& pd_pool_prefix,
                  const uint8_t pd_pool_prefix_length,
                  const uint16_t code,
                  const std::string& space) = 0;

    /// @brief Deletes global parameter.
    ///
    /// @param server_selector Server selector.
    /// @param name Name of the global parameter to be deleted.
    /// @return Number of deleted global parameters.
    virtual uint64_t
    deleteGlobalParameter6(const db::ServerSelector& server_selector,
                           const std::string& name) = 0;

    /// @brief Deletes all global parameters.
    ///
    /// @param server_selector Server selector.
    /// @return Number of deleted global parameters.
    virtual uint64_t
    deleteAllGlobalParameters6(const db::ServerSelector& server_selector) = 0;
};

/// @brief Shared pointer to the @c ConfigBackendDHCPv6 instance.
typedef boost::shared_ptr<ConfigBackendDHCPv6> ConfigBackendDHCPv6Ptr;

} // end of namespace isc::dhcp
} // end of namespace isc

#endif // CONFIG_BACKEND_DHCP6_H
