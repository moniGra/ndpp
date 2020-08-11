/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007-2009 Strasbourg University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

#ifndef IPV6_INTERFACE_H
#define IPV6_INTERFACE_H

#include <list>
#include <map>

#include "ns3/ipv6-address.h"
#include "ns3/ipv6-interface-address.h"
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/timer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-callback.h"

#include "mpr-lists.h"

namespace ns3
{

class NetDevice;
class Packet;
class Node;
class NdiscCache;

/**
 * \class Ipv6Interface
 * \brief The IPv6 representation of a network interface
 *
 * By default IPv6 interfaces are created in the "down" state
 * with IP "fe80::1" and a /64 prefix. Before becoming useable,
 * the user must invoke SetUp on them once the final IPv6 address
 * and mask has been set.
 */
class Ipv6Interface : public Object
{
public:
  /**
   * \brief Get the type ID
   * \return type ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Constructs an Ipv6Interface.
   */
  Ipv6Interface ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6Interface ();

  /**
   * \brief Set node associated with interface.
   * \param node node
   */
  void SetNode (Ptr<Node> node);

  /**
   * \brief Set the NetDevice.
   * \param device NetDevice
   */
  void SetDevice (Ptr<NetDevice> device);

  /**
   * \brief Get the NetDevice.
   * \return the NetDevice associated with this interface
   */
  virtual Ptr<NetDevice> GetDevice () const;

  /**
   * \brief Set the metric.
   * \param metric configured routing metric (cost) of this interface
   */
  void SetMetric (uint16_t metric);

  /**
   * \brief Get the metric
   * \return the metric
   */
  uint16_t GetMetric () const;

  /**
   * \brief Is the interface UP ?
   * \return true if interface is enabled, false otherwise.
   */
  bool IsUp () const;

  /**
   * \brief Is the interface DOWN ?
   * \return true if interface is disabled, false otherwise.
   */
  bool IsDown () const;

  /**
   * \brief Enable this interface.
   */
  void SetUp ();

  /**
   * \brief Disable this interface.
   */
  virtual void SetDown ();

  /**
   * \brief If the interface allows forwarding packets.
   * \return true if forwarding is enabled, false otherwise
   */
  bool IsForwarding () const;

  /**
   * \brief Set forwarding enabled or not.
   * \param forward forwarding state
   */
  void SetForwarding (bool forward);

  /**
   * \brief Set the current hop limit.
   * \param curHopLimit the value to set
   */
  void SetCurHopLimit (uint8_t curHopLimit);

  /**
   * \brief Get the current hop limit value.
   * \return current hop limit
   */
  uint8_t GetCurHopLimit () const;

  /**
   * \brief Set the base reachable time.
   * \param baseReachableTime the value to set
   */
  void SetBaseReachableTime (uint16_t baseReachableTime);

  /**
   * \brief Get the base reachable time.
   * \return base reachable time
   */
  uint16_t GetBaseReachableTime () const;

  /**
   * \brief Set the reachable time.
   * \param reachableTime value to set
   */
  void SetReachableTime (uint16_t reachableTime);

  /**
   * \brief Get the reachable time.
   * \return reachable time
   */
  uint16_t GetReachableTime () const;

  /**
   * \brief Set the retransmission timer.
   * \param retransTimer value to set
   */
  void SetRetransTimer (uint16_t retransTimer);

  /**
   * \brief Get the retransmission timer.
   * \return retransmission timer
   */
  uint16_t GetRetransTimer () const;

  /**
   * \brief Send a packet through this interface.
   * \param p packet to send
   * \param dest next hop address of packet.
   *
   * \note This method will eventually call the private SendTo
   * method which must be implemented by subclasses.
   */
  void Send (Ptr<Packet> p, Ipv6Address dest);

  /**
   * \brief Add an IPv6 address.
   * \param iface address to add
   * \return true if address was added, false otherwise
   */
  virtual bool AddAddress (Ipv6InterfaceAddress iface);

  /**
   * \brief Get link-local address from IPv6 interface.
   * \return link-local Ipv6InterfaceAddress, assert if not found
   */
  Ipv6InterfaceAddress GetLinkLocalAddress () const;

  /**
   * \brief Get an address from IPv6 interface.
   * \param index index
   * \return Ipv6InterfaceAddress address whose index is i
   */
  Ipv6InterfaceAddress GetAddress (uint32_t index) const;

  /**
   * \brief Get an address which is in the same network prefix as destination.
   * \param dst destination address
   * \return Corresponding Ipv6InterfaceAddress or assert if not found
   */
  Ipv6InterfaceAddress GetAddressMatchingDestination (Ipv6Address dst);

  /**
   * \brief Get number of addresses on this IPv6 interface.
   * \return number of address
   */
  uint32_t GetNAddresses (void) const;

  /**
   * \brief Remove an address from interface.
   * \param index index to remove
   * \return Ipv6InterfaceAddress address whose index is index
   */
  Ipv6InterfaceAddress RemoveAddress (uint32_t index);

  /**
   * \brief Remove the given Ipv6 address from the interface.
   * \param address The Ipv6 address to remove
   * \returns The removed Ipv6 interface address 
   * \returns The null interface address if the interface did not contain the 
   * address or if loopback address was passed as argument
   */
  Ipv6InterfaceAddress RemoveAddress (Ipv6Address address);

  /**
   * \brief Update state of an interface address.
   * \param address IPv6 address
   * \param state new state
   */
  void SetState (Ipv6Address address, Ipv6InterfaceAddress::State_e state);

  /**
   * \brief Update NS DAD packet UID of an interface address.
   * \param address IPv6 address
   * \param uid packet UID 
   */
  void SetNsDadUid (Ipv6Address address, uint32_t uid);


protected:
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();

//private:
  typedef std::list<Ipv6InterfaceAddress> Ipv6InterfaceAddressList;
  typedef std::list<Ipv6InterfaceAddress>::iterator Ipv6InterfaceAddressListI;
  typedef std::list<Ipv6InterfaceAddress>::const_iterator Ipv6InterfaceAddressListCI;

  /**
   * \brief Initialize interface.
   */
  void DoSetup ();

  /**
   * \brief The addresses assigned to this interface.
   */
  Ipv6InterfaceAddressList m_addresses;

  /**
   * \brief The state of this interface.
   */
  bool m_ifup;

  /**
   * \brief Forwarding state.
   */
  bool m_forwarding;

  /**
   * \brief The metric.
   */
  uint16_t m_metric;

  /**
   * \brief Node associated with this interface.
   */
  Ptr<Node> m_node;

  /**
   * \brief NetDevice associated with this interface.
   */
  Ptr<NetDevice> m_device;

  /**
   * \brief Neighbor cache.
   */
  Ptr<NdiscCache> m_ndCache;

  /**
   * \brief Current hop limit.
   */
  uint8_t m_curHopLimit;

  /**
   * \brief Base value used for computing the random reachable time value (in millisecond).
   */
  uint16_t m_baseReachableTime;

  /**
   * \brief Reachable time (in millisecond).
   * The time a neighbor is considered reachable after receiving a reachability confirmation.
   */
  uint16_t m_reachableTime;

  /**
   * \brief Retransmission timer (in millisecond).
   * Time between retransmission of NS.
   */
  uint16_t m_retransTimer;
};

class Ipv6Interface_Ndpp : public Ipv6Interface
{
public:

  /**
   * \brief ND++ constants : delay between messages with MPR Params opt
   */
 double m_MPRP_DELAY;

  MprData m_mprData;

  typedef std::map<Ipv6Address, int> NdadCountMap;	//key - ipv6Address for which n-DAD is ongoing, value - n-dad count
  typedef std::map<Ipv6Address, int>::iterator NdadCountMapI;
  typedef std::map<Ipv6Address, int>::const_iterator NdadCountMapCI;

  NdadCountMap m_nDADregistry;	//registry to store information about nDAD cont for each address on a given interface

  /**
   * \brief Get the type ID
   * \return type ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Constructs an Ipv6Interface.
   */
  Ipv6Interface_Ndpp ();

  /**
     * \brief copy Constructor
     */
    Ipv6Interface_Ndpp (const Ipv6Interface_Ndpp &o);

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6Interface_Ndpp ();

  /**
   * \brief Add an IPv6 address.
   * \param iface address to add
   * \return true if address was added, false otherwise
   */
  virtual bool AddAddress (Ipv6InterfaceAddress iface);

  //same as above, but used only after auto-assigned new address - no delay before DAD
  bool AddAddressNoDelay (Ipv6InterfaceAddress iface);

  /**
	* \brief Disable this interface.
 */
  virtual void SetDown ();

  void DoSetupAutoAddress ();

  /* function MPRLookUp
     * function finds a MPR set
     */
  void MprLookUp();

  /* function MprInit
       * function initiates MPR processes
       */
  void MprsInit();

private:
  bool m_mprs_init;

  void MprTimeout();


  /*function ListCopyStat
   * @param tN - a header to N list
   * @param tN2 - a header to N2 list
   *
   * @return size of N2 list
   *
   * Function performs a copy of the list structure given by m_mprData.m_m_neighbours. The copy is kept by tN header. While copying the structure, N2 list of 2 hop neighbors (N2) is created and
   * statistics for each node  are calculated. Statistics includes:
   * D(node)- the number of symmetric neighbours of  1 hop neighbor node (in field N), excluding all the members of N and excluding the node preforming computation. It is kept in field : other
   * D(node2) - the number of symmetric neighbors (only members of list N1) of node2 (the member of  N2 list). It is kept in field : other
   *
   */

  int ListCopyStat(OneHopSet *tN, TwoHopSet *tN2);

  /* function IsOn1hopList
   * @param taddr - address of the node
   * @return int (values 1 or 0)
   *
   * Function checks if a node with given address taddr is on the N list.  Returns 1 if element was found; 0 otherwise.
   *
   */
  int IsOn1hopList(Ipv6AddrRandomIDPair taddr);

  /*function AddToMPR
   * @param elem - node to be added
   * @param tN - header to N list N
   * @param tN2 - header to N2 list N2
   * @param tMPR - header to MPR_Set list
   * @tsizeN2 - size of N2 list
   * return new size of N2 list
   *
   * Adds node to MPR_SET list, and removes this node from N list. It also removes nodes from N2 which are now covered by a node in MPR Set
   */

  int AddToMPR(OneHopSetI elem_it,OneHopSet *tN,TwoHopSet *tN2, int tsizeN2);

  /*function CleanList
   * @param tN -  a header to N list
   * @param tN2 - a header to N2 list
   * @param stats - a table that keeps reachability for each member of N list
   *
   * Function removes nodes from 2 hop neighbor list kept by each member of N. Only nodes that are not in N2 list are removed. Reachability for each node in N is calculated and
   * kept in stats table
   */
  void CleanList(OneHopSet *tN, TwoHopSet *tN2);

  /* function IsOn2hopList
   * @param taddr - address of the node
   * @param head - header to N2 list
   * @return int (values 1 or 0)
   *
   * Function checks if a node with given address taddr is on the N2 list. Returns 1 if element was found; 0 otherwise
   *
   */
  int  IsOn2hopList(Ipv6AddrRandomIDPair, TwoHopSet *thead);

};

} /* namespace ns3 */

#endif /* IPV6_INTERFACE_H */

