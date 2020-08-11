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
 *         David Gross <gdavid.devel@gmail.com>
 *         Mehdi Benamor <benamor.mehdi@ensi.rnu.tn>
 */

#ifndef ICMPV6_L4_PROTOCOL_H
#define ICMPV6_L4_PROTOCOL_H

#include <list>

#include "ns3/ipv6-address.h"
#include "ns3/random-variable-stream.h"

#include "icmpv6-header.h"
#include "ip-l4-protocol.h"

#include "ns3/traced-callback.h"
#include <map>

namespace ns3
{

class NetDevice;
class Node;
class Packet;
class TraceContext;
class NdiscCache;

/**
 * \class Icmpv6L4Protocol
 * \brief An implementation of the ICMPv6 protocol.
 */
class Icmpv6L4Protocol : public IpL4Protocol
{
public:
  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief ICMPv6 protocol number (58).
   */
  static const uint8_t PROT_NUMBER;

  /**
   * \brief Neighbor Discovery router constants : max initial RA initial interval.
   */
  static const uint8_t MAX_INITIAL_RTR_ADVERT_INTERVAL;

  /**
   * \brief Neighbor Discovery router constants : max initial RA transmission.
   */
  static const uint8_t MAX_INITIAL_RTR_ADVERTISEMENTS;

  /**
   * \brief Neighbor Discovery router constants : max final RA transmission.
   */
  static const uint8_t MAX_FINAL_RTR_ADVERTISEMENTS;

  /**
   * \brief Neighbor Discovery router constants : min delay between RA.
   */
  static const uint8_t MIN_DELAY_BETWEEN_RAS;

  /**
   * \brief Neighbor Discovery router constants : max delay between RA.
   */
  static const uint32_t MAX_RA_DELAY_TIME;

  /**
   * \brief Neighbor Discovery host constants : max RS delay.
   */
  static const uint8_t MAX_RTR_SOLICITATION_DELAY;

  /**
   * \brief Neighbor Discovery host constants : RS interval.
   */
  static const uint8_t RTR_SOLICITATION_INTERVAL;

  /**
   * \brief Neighbor Discovery host constants : max RS transmission.
   */
  static const uint8_t MAX_RTR_SOLICITATIONS;

  /**
   * \brief Neighbor Discovery node constants : max multicast solicitations.
   */
  static const uint8_t MAX_MULTICAST_SOLICIT;

  /**
   * \brief Neighbor Discovery node constants : max unicast solicitations.
   */
  static const uint8_t MAX_UNICAST_SOLICIT;

  /**
   * \brief Neighbor Discovery node constants : max anycast delay.
   */
  static const uint8_t MAX_ANYCAST_DELAY_TIME;

  /**
   * \brief Neighbor Discovery node constants : max NA transmission.
   */
  static const uint8_t MAX_NEIGHBOR_ADVERTISEMENT;

  /**
   * \brief Neighbor Discovery node constants : reachable time.
   */
  static const uint32_t REACHABLE_TIME;

  /**
   * \brief Neighbor Discovery node constants : retransmission timer.
   */
  static const uint32_t RETRANS_TIMER;

  /**
   * \brief Neighbor Discovery node constants : delay for the first probe.
   */
  static const uint8_t DELAY_FIRST_PROBE_TIME;

  /**
   * \brief Neighbor Discovery node constants : min random factor.
   */
  static const double MIN_RANDOM_FACTOR;

  /**
   * \brief Neighbor Discovery node constants : max random factor.
   */
  static const double MAX_RANDOM_FACTOR;

  /**
   * \brief Get ICMPv6 protocol number.
   * \return protocol number
   */
  static uint16_t GetStaticProtocolNumber ();

  /**
   * \brief Constructor.
   */
  Icmpv6L4Protocol ();

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6L4Protocol ();

  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);

  /**
   * \brief This method is called by AddAgregate and completes the aggregation
   * by setting the node in the ICMPv6 stack and adding ICMPv6 factory to
   * IPv6 stack connected to the node.
   */
  void NotifyNewAggregate ();

  /**
   * \brief Get the protocol number.
   * \return protocol number
   */
  virtual int GetProtocolNumber () const;

  /**
   * \brief Get the version of the protocol.
   * \return version
   */
  virtual int GetVersion () const;

  /**
   * \brief Send a packet via ICMPv6, note that packet already contains ICMPv6 header.
   * \param packet the packet to send which contains ICMPv6 header
   * \param src source address
   * \param dst destination address
   * \param ttl next hop limit
   */
  void SendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl);

  /**
   * \brief Helper function used during delayed solicitation. Calls SendMessage internally
   * \param packet the packet to send which contains ICMPv6 header
   * \param src source address
   * \param dst destination address
   * \param ttl next hop limit
   */
  void DelayedSendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl);

  /**
   * \brief Send a packet via ICMPv6.
   * \param packet the packet to send
   * \param dst destination address
   * \param icmpv6Hdr ICMPv6 header (needed to calculate checksum
   * after source address is determined by routing stuff
   * \param ttl next hop limit
   */
  void SendMessage (Ptr<Packet> packet, Ipv6Address dst, Icmpv6Header& icmpv6Hdr, uint8_t ttl);

  /**
   * \brief Do the Duplication Address Detection (DAD).
   * It consists in sending a NS with our IPv6 as target. If
   * we received a NA with matched target address, we could not use
   * the address, else the address pass from TENTATIVE to PERMANENT.
   *
   * \param target target address
   * \param interface interface
   */
  virtual void DoDAD (Ipv6Address target, Ptr<Ipv6Interface> interface);

  /**
   * \brief Send a Neighbor Adverstisement.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param hardwareAddress our MAC address
   * \param flags to set (4 = flag R, 2 = flag S, 3 = flag O)
   */
  void SendNA (Ipv6Address src, Ipv6Address dst, Address* hardwareAddress, uint8_t flags);

  /**
   * \brief Send a Echo Reply.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param id id of the packet
   * \param seq sequence number
   * \param data auxiliary data
   */
  void SendEchoReply (Ipv6Address src, Ipv6Address dst, uint16_t id, uint16_t seq, Ptr<Packet> data);

  /**
   * \brief Send a Neighbor Solicitation.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param target target IPv6 address
   * \param hardwareAddress our mac address
   */
  void SendNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress);

  /**
   * \brief Send an error Destination Unreachable.
   * \param malformedPacket the malformed packet
   * \param dst destination IPv6 address
   * \param code code of the error
   */
  void SendErrorDestinationUnreachable (Ptr<Packet> malformedPacket, Ipv6Address dst, uint8_t code);

  /**
   * \brief Send an error Too Big.
   * \param malformedPacket the malformed packet
   * \param dst destination IPv6 address
   * \param mtu the mtu
   */
  void SendErrorTooBig (Ptr<Packet> malformedPacket, Ipv6Address dst, uint32_t mtu);

  /**
   * \brief Send an error Time Exceeded.
   * \param malformedPacket the malformed packet
   * \param dst destination IPv6 address
   * \param code code of the error
   */
  void SendErrorTimeExceeded (Ptr<Packet> malformedPacket, Ipv6Address dst, uint8_t code);

  /**
   * \brief Send an error Parameter Error.
   * \param malformedPacket the malformed packet
   * \param dst destination IPv6 address
   * \param code code of the error
   * \param ptr byte of p where the error is located
   */
  void SendErrorParameterError (Ptr<Packet> malformedPacket, Ipv6Address dst, uint8_t code, uint32_t ptr);

  /**
   * \brief Send an ICMPv6 Redirection.
   * \param redirectedPacket the redirected packet
   * \param src IPv6 address to send the redirect from
   * \param dst IPv6 address to send the redirect to
   * \param redirTarget IPv6 target address for Icmpv6Redirection
   * \param redirDestination IPv6 destination address for Icmpv6Redirection
   * \param redirHardwareTarget L2 target address for Icmpv6OptionRdirected
   */
  void SendRedirection (Ptr<Packet> redirectedPacket, Ipv6Address src, Ipv6Address dst, Ipv6Address redirTarget, Ipv6Address redirDestination, Address redirHardwareTarget);

  /**
   * \brief Forge a Neighbor Solicitation.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param target target IPv6 address
   * \param hardwareAddress our mac address
   * \return NS packet (with IPv6 header)
   */
  virtual Ptr<Packet> ForgeNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress);

  /**
   * \brief Forge a Neighbor Advertisement.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param hardwareAddress our mac address
   * \param flags flags (bitfield => R (4), S (2), O (1))
   * \param mpr_opts used in ndpp subclass for sending MPR Parameters (=1) and MPR Announcement (=2) options attached
   * \return NA packet (with IPv6 header)
   */
  virtual Ptr<Packet> ForgeNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags,
		  uint8_t mpr_opts=0,  MprData* mpr_data = NULL);


  /**
   * \brief Forge a Router Solicitation.
   * \param src source IPv6 address
   * \param dst destination IPv6 address
   * \param hardwareAddress our mac address
   * \return RS packet (with IPv6 header)
   */
  Ptr<Packet> ForgeRS (Ipv6Address src, Ipv6Address dst, Address hardwareAddress);

  /**
   * \brief Forge an Echo Request.
   * \param src source address
   * \param dst destination address
   * \param id ID of the packet
   * \param seq sequence number
   * \param data the data
   * \return Echo Request packet (without IPv6 header)
   */
  Ptr<Packet> ForgeEchoRequest (Ipv6Address src, Ipv6Address dst, uint16_t id, uint16_t seq, Ptr<Packet> data);

  /**
   * \brief Receive method.
   * \param p the packet
   * \param header the IPv4 header
   * \param interface the interface from which the packet is coming
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                               Ipv4Header const &header,
                                               Ptr<Ipv4Interface> interface);

  /**
   * \brief Receive method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                               Ipv6Header const &header,
                                               Ptr<Ipv6Interface> interface);

  //MG:added to allow passing ipv6 header as argument
 // virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);
  /**
   * \brief Function called when DAD timeout.
   * \param icmpv6 Icmpv6L4Protocol instance
   * \param interface the interface
   * \param addr the IPv6 address
   */
  //static void FunctionDadTimeout (Ptr<Icmpv6L4Protocol> icmpv6, Ipv6Interface* interface, Ipv6Address addr);
  virtual void FunctionDadTimeout (Ptr<Ipv6Interface> interface, Ipv6Address addr);


  /**
   * \brief Lookup in the ND cache for the IPv6 address
   * \param dst destination address
   * \param device device
   * \param cache the neighbor cache
   * \param hardwareDestination hardware address
   * \note Unlike other Lookup method, it does not send NS request!
   */

  bool Lookup (Ipv6Address dst, Ptr<NetDevice> device, Ptr<NdiscCache> cache, Address* hardwareDestination);

  /**
   * \brief Lookup in the ND cache for the IPv6 address (similar as ARP protocol).
   *
   * It also send NS request to target and store the waiting packet.
   * \param p the packet
   * \param dst destination address
   * \param device device
   * \param cache the neighbor cache
   * \param hardwareDestination hardware address
   * \return true if the address is in the ND cache, the hardwareDestination is updated.
   */
  bool Lookup (Ptr<Packet> p, Ipv6Address dst, Ptr<NetDevice> device, Ptr<NdiscCache> cache, Address* hardwareDestination);

  /**
   * \brief Send a Router Solicitation.
   * \param src link-local source address
   * \param dst destination address (usealy ff02::2 i.e all-routers)
   * \param hardwareAddress link-layer address (SHOULD be included if src is not ::
   */
  void SendRS (Ipv6Address src, Ipv6Address dst,  Address hardwareAddress);

  /**
   * \brief Create a neighbor cache.
   * \param device thet NetDevice
   * \param interface the IPv6 interface
   * \return a smart pointer of NdCache or 0 if problem
   */
  Ptr<NdiscCache> CreateCache (Ptr<NetDevice> device, Ptr<Ipv6Interface> interface);

  /**
   * \brief Is the node must do DAD.
   * \return true if node has to do DAD.
   */
  bool IsAlwaysDad () const;

  //set HopLimit value
  virtual void SetHopLimit(uint32_t hopLimit) {};

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  virtual int64_t AssignStreams (int64_t stream);

  double GetDadDelay();
  double GetInitDadDelay();

protected:
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();

  typedef std::list<Ptr<NdiscCache> > CacheList;

  /**
   * \brief The node.
   */
  Ptr<Node> m_node;

  /**
   * \brief A list of cache by device.
   */
  CacheList m_cacheList;

  /**
   * \brief Always do DAD ?
   */
  bool m_alwaysDad;

  /**
   * \brief Random jitter before sending solicitations
   */
  Ptr<RandomVariableStream> m_solicitationJitter;

  //daley before DAD start - according to RFC4862
  Ptr<UniformRandomVariable> m_initDadDelay;

  /**
   * \brief Notify an ICMPv6 reception to upper layers (if requested).
   * \param source the ICMP source
   * \param icmp the ICMP header
   * \param info information about the ICMP
   * \param ipHeader the IP header carried by the ICMP
   * \param payload the data carried by the ICMP
   */
  void Forward (Ipv6Address source, Icmpv6Header icmp,
                uint32_t info, Ipv6Header ipHeader,
                const uint8_t payload[8]);

  /**
   * \brief Receive Neighbor Solicitation method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  virtual void HandleNS (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Router Solicitation method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleRS (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Router Advertisement method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleRA (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Echo Request method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleEchoRequest (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Neighbor Advertisement method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  virtual void HandleNA (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Redirection method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleRedirection (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Destination Unreachable method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleDestinationUnreachable (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Time Exceeded method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleTimeExceeded (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Packet Too Big method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandlePacketTooBig (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Parameter Error method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void HandleParameterError (Ptr<Packet> p, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Link layer address option processing.
   * \param lla LLA option
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  void ReceiveLLA (Icmpv6OptionLinkLayerAddress lla, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Get the cache corresponding to the device.
   * \param device the device
   */
  Ptr<NdiscCache> FindCache (Ptr<NetDevice> device);

  // From IpL4Protocol
  virtual void SetDownTarget (IpL4Protocol::DownTargetCallback cb);
  virtual void SetDownTarget6 (IpL4Protocol::DownTargetCallback6 cb);
  // From IpL4Protocol
  virtual IpL4Protocol::DownTargetCallback GetDownTarget (void) const;
  virtual IpL4Protocol::DownTargetCallback6 GetDownTarget6 (void) const;

  IpL4Protocol::DownTargetCallback6 m_downTarget;

};


class Icmpv6L4Protocol_NdSimple : public Icmpv6L4Protocol
{
public:

	enum NdppMessageType_e
	{
		UNKNOWN = 0,
		NDPP_NA_1HOP,	//for DAD
		NDPP_NA_MHOP,
		NDPP_NS_1HOP,
		NDPP_NS_MHOP,
		NDPP_NA_1HOP_MPRs,
		NdppMessageTypeSize	//just to identify how many elements are defined here - for statistics
	};


	enum MprOptType_e
	{
		NO_MPR_OPT = 0,
		MPR_PARAMS = 0x01,
		MPR_ANNOUNCEMENT = 0x02,
		BOTH = 0x03
	};

	enum ExitCode_e	//for tracing: to specify, what kind of event took place for a packet
	{
	   NO_EXIT_CODE = 0,
	   NOT_NS_FOR_US,
	   OWN_DAD_NS_PROBE,
	   OTHERS_WITH_MY_ADDR,
	   DAD_QUERY,
	   LLA_NEIGH_CACHE_UPDATE,
	   OWN_DAD_NA_PROBE,
	   DAD_DUPLICATION,
	   WARNING_DUPLICATION_POSSIBLE,
	   DISCARD_NA,	//no communication with target initiated earlier
	   DISCARD_NA_INVALID_ADDR,
	   DISCARD_NS_INVALID_ADDR,
	   MPR_INFO,
	   DUPLICATED_MNS_DROPPED,
	   DUPLICATED_MNA_DROPPED,
	   ExitCodeTypeSize	//just to identify how many elements are defined here - for statistics
	};

	enum EntryCode_e	//for tracing: to specify, what kind of event took place for a packet
	{
	   NO_ENTRY_CODE,
	   FORWARDED,
	   DAD_REPLY,
	   DAD_INIT,
	   NDAD_INIT,
	   MPR_PROCESSES
	};

	/**
   * \brief ICMPv6 code number for enhanced ND
   */
	static const uint8_t NDAD_CODE;
//	static const double FRW_DELAY;	//in milliseconds!!!

  /**
   * \brief Constructor.
   */
  Icmpv6L4Protocol_NdSimple ();

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6L4Protocol_NdSimple ();

  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  /**
    * \brief Do the Duplication Address Detection (DAD).
    * It consists in sending a NS with our IPv6 as target. If
    * we received a NA with matched target address, we could not use
    * the address, else the address pass from TENTATIVE to PERMANENT.
    *
    * \param target target address
    * \param interface interface
    */
   virtual void DoDAD (Ipv6Address target, Ptr<Ipv6Interface> interface);

   /**
   	* \brief Do Duplication Address Detection with ehanced scope (n-DAD).
   	* It consists in sending a NS with our IPv6 as target with HopLimit set to n.
   	*
   	* \param target target address
   	* \param interface interface
   	*/
   virtual void Do_nDAD (Ipv6Address target, Ptr<Ipv6Interface> interface);


	/**
	   * \brief Function called when DAD timeout.
	   * \param icmpv6 Icmpv6L4Protocol instance
	   * \param interface the interface
	   * \param addr the IPv6 address
	   */

   virtual void FunctionDadTimeout (Ptr<Ipv6Interface> interface, Ipv6Address addr);

   //set HopLimit value
   virtual void SetHopLimit(uint32_t hopLimit);


protected:

   uint32_t m_HopLimit;

   //specify random delay before sending first MPR Parameters message
   Ptr<UniformRandomVariable> m_frwDelay;

   //time to wait for a reply during n-dad (in ms!!!)
   uint32_t m_retransTimerNDad;

   /**
	* \brief Forge a Neighbor Solicitation.
	* \param src source IPv6 address
	* \param dst destination IPv6 address
	* \param target target IPv6 address
	* \param hardwareAddress our mac address
	* \return NS packet (with IPv6 header)
	*/

   Ptr<Packet> Forge_mNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress);

   /**
	* \brief Forge a Neighbor Advertisement.
	* \param src source IPv6 address
	* \param dst destination IPv6 address
	* \param hardwareAddress our mac address
	* \param flags flags (bitfield => R (4), S (2), O (1))
	* \return NA packet (with IPv6 header)
	*/
  Ptr<Packet> Forge_mNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags);


   /**
	* \brief Receive Neighbor Solicitation method.
	* \param p the packet
	* \param src source address
	* \param dst destination address
	* \param interface the interface from which the packet is coming
	*/
  virtual void HandleNS (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  virtual void HandleNS_local (Ptr<Packet> p, Ipv6Header ip, Ipv6Address &src, Ipv6Address &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Neighbor Advertisement method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  virtual void HandleNA (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  //tracing:
  /**
	* \brief Callback to trace TX (transmission) packets.
	*/
  TracedCallback<Ptr<const Packet>, NdppMessageType_e, MprOptType_e, EntryCode_e, uint32_t, Time> m_txNdTrace;

  /**
   * \brief Callback to trace RX (reception) packets.
   */
  TracedCallback<Ptr<const Packet>, const Icmpv6Header*, NdppMessageType_e, MprOptType_e, ExitCode_e, uint32_t> m_rxNdTrace;


  friend class Ipv6Interface_Ndpp;
};


class Icmpv6L4Protocol_Ndpp : public Icmpv6L4Protocol_NdSimple
{

public:

	/**
	* \brief ND++ constants : delay before starting n-DAD
	*/
	static const double NDAD_DELAY;

	enum InterfaceState_e
	{
		INIT = 0,	//query for an address has been sent
		INVALID,	//teh address is invalid
		PREFERRED	//the address is preferred (NOTE that this may refer to one of next addresses auto-assigned to this interface)
	};

	typedef std::list<Icmpv6OptionMprParams> MprOptsList;	//list of all mprp_opts
	typedef std::list<Icmpv6OptionMprParams>::iterator MprOptsI;

  /**
   * \brief Constructor.
   */
  Icmpv6L4Protocol_Ndpp ();

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6L4Protocol_Ndpp ();

  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

   /**
   	* \brief Do Duplication Address Detection with ehanced scope (n-DAD).
   	* It consists in sending a NS with our IPv6 as target with HopLimit set to n.
   	*
   	* \param target target address
   	* \param interface interface
   	*/
   virtual void Do_nDAD (Ipv6Address target, Ptr<Ipv6Interface> interface);


	/**
	   * \brief Function called when DAD timeout.
	   * \param icmpv6 Icmpv6L4Protocol instance
	   * \param interface the interface
	   * \param addr the IPv6 address
	   */

   virtual void FunctionDadTimeout (Ptr<Ipv6Interface> interface, Ipv6Address addr);

   /**
    * \brief Forge a Neighbor Advertisement.
    * \param src source IPv6 address
    * \param dst destination IPv6 address
    * \param hardwareAddress our mac address
    * \param flags flags (bitfield => R (4), S (2), O (1))
    * \param mpr_opts used in ndpp subclass for sending MPR Parameters (=1) and MPR Announcement (=2) options attached
    * \return NA packet (with IPv6 header)
	*/
   virtual Ptr<Packet> ForgeNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags,
		   uint8_t mpr_opts=0, MprData* mpr_data = NULL);

   int64_t AssignStreams (int64_t stream);

   double GetMprStartDelay();

   virtual void SendAsyncMPRann( Ptr<Ipv6Interface> interface);


protected:

   /**
      * \brief node's willingness to become MPR for the others
      */
   	uint8_t m_willingness;

   	//specify random delay before sending first MPR Parameters message
   	Ptr<UniformRandomVariable> m_mprStartDelay;

   	//number of consecutive n-DAD trials
   	int m_nDADcount;

	/**
	* \brief Callback to trace address events and track latency to obtain an address
	*/
	TracedCallback<Ptr<Ipv6Interface>, Ipv6Address, InterfaceState_e> m_latencyTrace;



   /**
	* \brief Forge a Neighbor Solicitation.
	* \param src source IPv6 address
	* \param dst destination IPv6 address
	* \param target target IPv6 address
	* \param hardwareAddress our mac address
	* \return NS packet (with IPv6 header)
	*/

   //Ptr<Packet> Forge_mNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress);

   /**
	* \brief Forge a Neighbor Advertisement.
	* \param src source IPv6 address
	* \param dst destination IPv6 address
	* \param hardwareAddress our mac address
	* \param flags flags (bitfield => R (4), S (2), O (1))
	* \return NA packet (with IPv6 header)
	*/
 // Ptr<Packet> Forge_mNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags);


   /**
	* \brief Receive Neighbor Solicitation method.
	* \param p the packet
	* \param src source address
	* \param dst destination address
	* \param interface the interface from which the packet is coming
	*/
 // virtual void HandleNS (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  virtual void HandleNS_local (Ptr<Packet> p, Ipv6Header ip, Ipv6Address &src, Ipv6Address &dst, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive Neighbor Advertisement method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  virtual void HandleNA (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);


  friend class Ipv6Interface_Ndpp;
};

class ForwardingTuple{
public:
	ForwardingTuple(Ipv6Address targetAddr,/* Ipv6Address dstAddr,*/
			int randomID, int sequenceNo, Ipv6Address srcAddr = "::");

	Ipv6Address m_targetAddr;
	//Ipv6Address m_dstAddr;
	int m_sequenceNo;
	int m_randomID;
	Ipv6Address m_srcAddr;

	//necessary for map comparison function
	friend bool operator < (ForwardingTuple const &a, ForwardingTuple const &b);
	friend bool operator == (ForwardingTuple const &a, ForwardingTuple const &b);
	friend bool operator != (ForwardingTuple const &a, ForwardingTuple const &b);

};


class Icmpv6L4Protocol_NdppFrw : public Icmpv6L4Protocol_Ndpp
{

public:
  /**
   * \brief Constructor.
   */
  Icmpv6L4Protocol_NdppFrw ();

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6L4Protocol_NdppFrw ();

  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  /**
	* \brief Do Duplication Address Detection with ehanced scope (n-DAD).
	* It consists in sending a NS with our IPv6 as target with HopLimit set to n.
	*
	* \param target target address
	* \param interface interface
	*/
  virtual void Do_nDAD (Ipv6Address target, Ptr<Ipv6Interface> interface);

  int GetSequenceNo();	//returns SN and increases it by 1
  void ClearFrwSet();
  void ClearDADReplySet();

protected:

  int m_sequenceNo;
  bool m_frwUseSrcAddr;

  typedef std::map<ForwardingTuple, int> FrwMap;	//key - forwarding tuple (target, SN, RandomID), value - occurence counter
  typedef std::map<ForwardingTuple, int>::iterator FrwMapI;
  typedef std::map<ForwardingTuple, int>::const_iterator FrwMapCI;

  FrwMap m_frwSet;	//forwarding set to store info about forwarded messages
  FrwMap m_DADReplySet;

  int m_frwCountLimit;
  int m_DADReplyCountLimit;

  //whether to auto-assign a new address in case there is no valid address on the interface
  //check if there is any address possible to be used, if not - try to assign new one based on MAC address
  //WARNING! if the above MAC-based address is already there marked as INVALID - leave it
  //this approach makes sense in my simulations, in general probbaly a random selection shoudl be made
  bool m_autoAssignNewAddr;

  virtual void HandleNS_local (Ptr<Packet> p, Ipv6Header ip, Ipv6Address &src, Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  /**
   * \brief Receive Neighbor Advertisement method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  virtual void HandleNA (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

  virtual void AutoAssignNewAddr(Ptr<Ipv6Interface> interface);

};

//this version provides changes to MPR selection heuristic - taking into consideration random IDs
class Icmpv6L4Protocol_NdppMpr : public Icmpv6L4Protocol_NdppFrw
{

public:

	typedef std::list<Icmpv6OptionMprParamsWithRandomIds> MprOptsListWithIds;	//list of all mprp_opts
	typedef std::list<Icmpv6OptionMprParamsWithRandomIds>::iterator MprOptsWithIdsI;
  /**
   * \brief Constructor.
   */
  Icmpv6L4Protocol_NdppMpr (){};

  /**
   * \brief Destructor.
   */
  virtual ~Icmpv6L4Protocol_NdppMpr (){};

  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  virtual Ptr<Packet> ForgeNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags,
  		   uint8_t mpr_opts=0, MprData* mpr_data = NULL);

protected:
  virtual void HandleNA (Ptr<Packet> p, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface);

};
//adding to Attribute system:

//std::ostream &operator << (std::ostream &os, const Icmpv6L4Protocol_NdSimple &icmpv6L4Protocol_NdSimple);
//std::istream &operator >> (std::istream &is, Icmpv6L4Protocol_NdSimple &icmpv6L4Protocol_NdSimple);

/**
 * \brief hold objects of type ns3::Address
 */
//ATTRIBUTE_HELPER_HEADER (Icmpv6L4Protocol_NdSimple);

} /* namespace ns3 */

#endif /* ICMPV6_L4_PROTOCOL_H */

