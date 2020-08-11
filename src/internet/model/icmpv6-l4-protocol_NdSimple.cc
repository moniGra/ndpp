/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 ND modification 1 (NdSimple)
 author: Monika Grajzer
 */

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"

#include "ipv6-raw-socket-factory-impl.h"
#include "ipv6-l3-protocol.h"
#include "ipv6-interface.h"
#include "icmpv6-l4-protocol.h"
#include "ndisc-cache.h"


//MG:
#include "ns3/flow-id-tag.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"



namespace ns3
{
//ATTRIBUTE_HELPER_H (Icmpv6L4Protocol_NdSimple);

NS_OBJECT_ENSURE_REGISTERED (Icmpv6L4Protocol_NdSimple);

NS_LOG_COMPONENT_DEFINE ("Icmpv6L4Protocol_NdSimple");


const uint8_t Icmpv6L4Protocol_NdSimple::NDAD_CODE = 1;
//const double Icmpv6L4Protocol_NdSimple::FRW_DELAY = 100;	//in milliseconds!!!

Icmpv6L4Protocol_NdSimple::Icmpv6L4Protocol_NdSimple ()
	: m_HopLimit(3)
{
	/*m_frwDelay = CreateObject<UniformRandomVariable> ();
	m_frwDelay->SetAttribute ("Min", DoubleValue (0.0));
	m_frwDelay->SetAttribute ("Max", DoubleValue (FRW_DELAY));*/
}

Icmpv6L4Protocol_NdSimple::~Icmpv6L4Protocol_NdSimple()
{
	NS_LOG_FUNCTION_NOARGS ();
}

TypeId Icmpv6L4Protocol_NdSimple::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Icmpv6L4Protocol_NdSimple")
    .SetParent<Icmpv6L4Protocol> ()
    .AddConstructor<Icmpv6L4Protocol_NdSimple> ()
    .AddAttribute ("HopLimit",
    		"Set the ND++ range - Hop Limit value for multicast NS and NA messages",
    		UintegerValue (3),
    		MakeUintegerAccessor (&Icmpv6L4Protocol_NdSimple::m_HopLimit),
    		MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("RETRANS_TIMER_NDAD", "Time to wait for a reply to n-DAD query (in ms!!!)",
			UintegerValue (1000),
			MakeUintegerAccessor (&Icmpv6L4Protocol_NdSimple::m_retransTimerNDad),
			MakeUintegerChecker<uint32_t> ())
	.AddTraceSource("TxNd", "Send ND packet to outgoing interface.",
            MakeTraceSourceAccessor (&Icmpv6L4Protocol_NdSimple::m_txNdTrace))
    .AddTraceSource ("RxNd", "Receive ND packet from incoming interface.",
                                 MakeTraceSourceAccessor (&Icmpv6L4Protocol_NdSimple::m_rxNdTrace))
	.AddAttribute ("FrwDelay",
			"A random delay before forwarding a mNS or mNA message (in milisec.!!!)",
			StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
			MakePointerAccessor (&Icmpv6L4Protocol_NdSimple::m_frwDelay),
			MakePointerChecker<RandomVariableStream> ())
    /*.AddAttribute ("DAD", "Always do DAD check.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&Icmpv6L4Protocol_NdSimple::m_alwaysDad),
                   MakeBooleanChecker ())*/
  ;
  //check if m_always DAD is set to true - this can be inherited as an attribute from a base class??? (if not - initialize in constructor)
  return tid;
}


void Icmpv6L4Protocol_NdSimple::HandleNS (Ptr<Packet> packet, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface)
{
	this->HandleNS_local(packet, ip, (Ipv6Address &) src, (Ipv6Address &) dst, interface);
}

void Icmpv6L4Protocol_NdSimple::HandleNS_local (Ptr<Packet> packet, Ipv6Header ip, Ipv6Address &src, Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);

  Icmpv6NS nsHeader ("::");
  Ipv6InterfaceAddress ifaddr;
  uint32_t nb = interface->GetNAddresses ();
  uint32_t i = 0;
  bool found = false;
  bool tentative_dad = false;
  bool ndad = false;

  //collecting tracing information:
  NdppMessageType_e messageType = UNKNOWN;

  //Ipv6Address dst_local = dst;
 // Ipv6Address src_local = src;

  //MG: forward a copy of multihop NS packets:
  packet->PeekHeader(nsHeader);
  if(nsHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE && dst.IsAllNodesMulticast() && ip.GetHopLimit() > 1)
  {
	  // NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NS message received");
	  //MG: add MPRs processing here

	Ptr<Packet> p = packet->Copy();	//copy packet to be forwarded!
	ip.SetHopLimit(ip.GetHopLimit() - 1);

	//change source to my main addr.
	//REMARK: the first valid address on the list is chosen!, the address selection procedure from RFC... is not implemented
	Ipv6Address src_copy = interface -> GetLinkLocalAddress().GetAddress();
	ip.SetSourceAddress(src_copy);

	//update checksum in NA body
	Icmpv6NS ns_copy;
	p->RemoveHeader(ns_copy);
	ns_copy.CalculatePseudoHeaderChecksum (src_copy, dst, p->GetSize () + ns_copy.GetSerializedSize (), PROT_NUMBER);

	p->AddHeader(ns_copy);
	p->AddHeader(ip);

	//send packet
	//trace:
	double frwDelay = m_frwDelay->GetValue();
	m_txNdTrace (p, NDPP_NS_MHOP, NO_MPR_OPT, FORWARDED,
			(m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()),
			Seconds(frwDelay));

	Simulator::Schedule(MilliSeconds(frwDelay), &Ipv6Interface::Send, interface,
			p, Ipv6Address::GetAllNodesMulticast());
	//interface->Send (p, Ipv6Address::GetAllNodesMulticast());

	NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NS message forwarded with "
					"HopLimit = " << (int)ip.GetHopLimit());
  }

  packet->RemoveHeader (nsHeader);

  Ipv6Address target = nsHeader.GetIpv6Target ();

  //MG: for mNS adjust src and dst: change dst to solicited node multicast, to process packet normally
  //also change src to unspecified

  if(nsHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE && dst.IsAllNodesMulticast())
  {
	  //trace:
	  messageType = NDPP_NS_MHOP;
	 //  m_rxNdTrace (packet, nsHeader, messageType, NONE, (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

	  ndad = true;
	  dst = Ipv6Address::MakeSolicitedAddress (target);

	  /*check if we belong to MC group of solicited node multicast - if not - drop the packet
	  this should normally be done somewhere else, but probably is not implemented in ns-3
	  (thus checking it by hand would cost the same as comparing target with all my addresses)
	  -> I will leave it for now assuming that if the sender put me in target I will also be in the solicited node MC group and vice versa*/

	  src = Ipv6Address::GetAny();
  }
  else if (nsHeader.GetCode() != Icmpv6L4Protocol_NdSimple::NDAD_CODE)
  {
	  messageType = NDPP_NS_1HOP;
  }


  for (i = 0; i < nb; i++)
    {
      ifaddr = interface->GetAddress (i);

      if (ifaddr.GetAddress () == target)
        {
    	  if(ifaddr.GetState() == Ipv6InterfaceAddress::INVALID)	//only valid addresses are allowed (RFC4861)
    	  {
    		  //silently discard
    		  return;
    	  }
    	  if(ifaddr.GetState() == Ipv6InterfaceAddress::TENTATIVE
    			  || ifaddr.GetState() == Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC)
    	  {
    		  //process as defined in RFC4862 - node MUST NOT respond to this NS
    		 /* src addr	how to process?
			 * ---		---
			 * multicast	invalid -> ignore
			 * unicast	somebody is doing address resolution ->  ignore
			 * unspec	duplicate address detection -> process as in RFC4862*/

    		  if(!src.IsMulticast() && src.IsAny())
    			  tentative_dad = true;
    		  else
    			  return;	//ignore
    	  }
          found = true;
          break;
        }
    }

  if (!found)
    {
      NS_LOG_LOGIC ("Not a NS for us");
      //trace:
      m_rxNdTrace (packet, &nsHeader, messageType, NO_MPR_OPT, NOT_NS_FOR_US,
    		  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

      return;
    }

  //check "RandomID option"
	FlowIdTag randomId;
	if((packet->PeekPacketTag(randomId) && randomId.GetFlowId() == m_node->GetId())
		  || packet->GetUid () == ifaddr.GetNsDadUid ())
	{
	  /* don't process our own DAD probe */
	  NS_LOG_LOGIC ("Hey we receive our DAD probe!");
	  //trace:
	  m_rxNdTrace (packet, &nsHeader, messageType, NO_MPR_OPT, OWN_DAD_NS_PROBE,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

	  return;
	}
  /*if (packet->GetUid () == ifaddr.GetNsDadUid ())
    {
       don't process our own DAD probe
      NS_LOG_LOGIC ("Hey we receive our DAD probe!");
      return;
    }*/

  if(tentative_dad)
  {
	  //somebody else is doing DAD on my tentative address, I cannot use it
//	  ifaddr.SetState(Ipv6InterfaceAddress::INVALID); -> this is wrong since it changes local variable ifaddr only!
	  interface->SetState(ifaddr.GetAddress(), Ipv6InterfaceAddress::INVALID);
	  NS_LOG_INFO("Node " << this->m_node->GetId() << ", Somebody else is doing DAD on my tentative address: " << ifaddr.GetAddress() <<
			  ". The address is INVALID");

	  //trace:
	  m_rxNdTrace (packet, &nsHeader, messageType, NO_MPR_OPT, OTHERS_WITH_MY_ADDR,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

	  return;
  }
  Icmpv6OptionLinkLayerAddress lla (1);
  Address hardwareAddress;
  NdiscCache::Entry* entry = 0;
  Ptr<NdiscCache> cache = FindCache (interface->GetDevice ());
  uint8_t flags = 0;

  /* XXX search all options following the NS header */

  if (src != Ipv6Address::GetAny ())
    {
      uint8_t type;
      packet->CopyData (&type, sizeof(type));

      if (type != Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE)
        {
          return;
        }

      //trace:
      	  m_rxNdTrace (packet, &nsHeader, messageType, NO_MPR_OPT, LLA_NEIGH_CACHE_UPDATE,
      			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
      /* Get LLA */
      packet->RemoveHeader (lla);

      entry = cache->Lookup (src);
      if (!entry)
        {
          entry = cache->Add (src);
          entry->SetRouter (false);
          entry->MarkStale (lla.GetAddress ());
        }
      else if (entry->GetMacAddress () != lla.GetAddress ())
        {
          entry->MarkStale (lla.GetAddress ());
        }

      flags = 3; /* S + O flags */
    }
  else
    {
      /* it means someone do a DAD */
	  //trace:
	  m_rxNdTrace (packet, &nsHeader, messageType, NO_MPR_OPT, DAD_QUERY,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
      flags = 1; /* O flag */
    }

  /* send a NA to src */
  Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();

  if (ipv6->IsForwarding (ipv6->GetInterfaceForDevice (interface->GetDevice ())))
    {
      flags += 4; /* R flag */
    }

  hardwareAddress = interface->GetDevice ()->GetAddress ();
  Ptr<Packet> p;
  if(ndad)
  {
	  p = Forge_mNA (target.IsLinkLocal () ? interface->GetLinkLocalAddress ().GetAddress () : ifaddr.GetAddress (), Ipv6Address::GetAllNodesMulticast(), target, &hardwareAddress, flags );
	  //we should set packet Uid to some global value for a node (in this case node id, which is assumed unique) - otherwise NaDadUid changes with each mNA sent as a reply for the same target

	  //add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
	  //flowId tag is used since it's capabilities are good for this purpose
	  FlowIdTag randomId(this->m_node->GetId());
	  p->AddPacketTag(randomId);
	  //trace:
	  m_txNdTrace (p, NDPP_NA_MHOP, NO_MPR_OPT, DAD_REPLY,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()), Seconds(0.0));
  }
  else
  {
	  p = ForgeNA (target.IsLinkLocal () ? interface->GetLinkLocalAddress ().GetAddress () : ifaddr.GetAddress (), src.IsAny () ? Ipv6Address::GetAllNodesMulticast () : src, target, &hardwareAddress, flags );
	  //trace:
	  m_txNdTrace (p, NDPP_NA_1HOP, NO_MPR_OPT, DAD_REPLY,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()), Seconds(0.0));
  }
  interface->Send (p,  src.IsAny () ? Ipv6Address::GetAllNodesMulticast () : src); 

  /* not a NS for us discard it */
}

void Icmpv6L4Protocol_NdSimple::HandleNA (Ptr<Packet> packet, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  Icmpv6NA naHeader;
  Icmpv6OptionLinkLayerAddress lla (1);

  //collecting tracing information:
  NdppMessageType_e messageType;

  //MG: forward a copy of multihop NA packets:
  packet->PeekHeader(naHeader);
  if(naHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE && dst.IsAllNodesMulticast() && ip.GetHopLimit() > 1)
  {
	 // NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NA message received");
	  //MG: add MPRs processing here


	  Ptr<Packet> p = packet->Copy();	//copy packet to be forwarded!
	  ip.SetHopLimit(ip.GetHopLimit() - 1);

	  //change source to my main addr.
	  //REMARK: the first valid address on the list is chosen!, the address selection procedure from RFC... is not implemented
	  Ipv6Address src_copy = interface -> GetLinkLocalAddress().GetAddress();
	  ip.SetSourceAddress(src_copy);

	  //update checksum in NA body
	  Icmpv6NA na_copy;
	  p->RemoveHeader(na_copy);
	  na_copy.CalculatePseudoHeaderChecksum (src_copy, dst, p->GetSize () + na_copy.GetSerializedSize (), PROT_NUMBER);

	  p->AddHeader(na_copy);
	  p->AddHeader(ip);

	  //send packet
	  //trace:
	  double frwDelay = m_frwDelay->GetValue();
	  m_txNdTrace (p, NDPP_NA_MHOP, NO_MPR_OPT, FORWARDED,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()),
			  Seconds(frwDelay));
	  Simulator::Schedule(MilliSeconds(frwDelay), &Ipv6Interface::Send, interface,
			  p, Ipv6Address::GetAllNodesMulticast());
	  //interface->Send (p, Ipv6Address::GetAllNodesMulticast());

	  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NA message forwarded with "
					"HopLimit = " << (int)ip.GetHopLimit());
  }


  //for tracing purposes only:
  messageType = (naHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE)? NDPP_NA_MHOP : NDPP_NA_1HOP;

  packet->RemoveHeader (naHeader);
  Ipv6Address target = naHeader.GetIpv6Target ();

  Address hardwareAddress;
  NdiscCache::Entry* entry = 0;
  Ptr<NdiscCache> cache = FindCache (interface->GetDevice ());
  std::list<Ptr<Packet> > waiting;

  /* check if we have something in our cache */
  entry = cache->Lookup (target);

  if (!entry)
    {
      /* ouch!! we are victim of a DAD */
      Ipv6InterfaceAddress ifaddr;
      bool found = false;
      uint32_t i = 0;
      uint32_t nb = interface->GetNAddresses ();;

      for (i = 0; i < nb; i++)
        {
    	  ifaddr = interface->GetAddress (i);
          if (ifaddr.GetAddress () == target)
            {
              found = true;
              break;
            }
        }

      if (found)
      {
    	  if(naHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE)
		  {
    		  //check "RandomID option"
    		  FlowIdTag randomId;
    		  if(packet->PeekPacketTag(randomId) && randomId.GetFlowId() == m_node->GetId())
			  {
				  /* don't process our own DAD NA answer */
				  NS_LOG_LOGIC ("Hey we receive our DAD mNA!");
				  //trace:
				  m_rxNdTrace (packet, &naHeader, NDPP_NA_MHOP, NO_MPR_OPT, OWN_DAD_NA_PROBE,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
				  return;
			  }
		  }
          if (ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE || ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC)
          {
        	  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::INVALID);
              NS_LOG_INFO("Node " << this->m_node->GetId() << ": DAD detected duplicated address: " << ifaddr.GetAddress() << ". The address is INVALID");
              //trace:
			  m_rxNdTrace (packet, &naHeader, messageType, NO_MPR_OPT, DAD_DUPLICATION,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
          }
          else if (ifaddr.GetState () == Ipv6InterfaceAddress::PREFERRED)
          {
        	  //somebody is using my address, which is valid. Probably there is a duplication in the network
        	  NS_LOG_INFO("Node " << this->m_node->GetId() << ": Duplicated IPv6 address: " << ifaddr.GetAddress());

        	  //trace:
			  m_rxNdTrace (packet, &naHeader, messageType, NO_MPR_OPT, WARNING_DUPLICATION_POSSIBLE,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
          }
      }
      /* we have not initiated any communication with the target so... discard the NA */
      //trace:
	  m_rxNdTrace (packet, &naHeader, messageType, NO_MPR_OPT, DISCARD_NA,
		  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
      return;
    }

  //MG: just for safety, maybe unnecessary;
  if(naHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE)
	  return;


  /* XXX search all options following the NA header */
  /* Get LLA */
  uint8_t type;
  packet->CopyData (&type, sizeof(type));

  if (type != Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET)
    {
      return;
    }
  //trace:
  m_rxNdTrace (packet, &naHeader, messageType, NO_MPR_OPT, LLA_NEIGH_CACHE_UPDATE,
	  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
  packet->RemoveHeader (lla);

  if (entry->IsIncomplete ())
    {
      /* we receive a NA so stop the retransmission timer */
      entry->StopRetransmitTimer ();

      if (naHeader.GetFlagS ())
        {
          /* mark it to reachable */
          waiting = entry->MarkReachable (lla.GetAddress ());
          entry->StopReachableTimer ();
          entry->StartReachableTimer ();
          /* send out waiting packet */
          for (std::list<Ptr<Packet> >::const_iterator it = waiting.begin (); it != waiting.end (); it++)
            {
              cache->GetInterface ()->Send (*it, src);
            }
          entry->ClearWaitingPacket ();
        }
      else
        {
          entry->MarkStale (lla.GetAddress ());
        }

      if (naHeader.GetFlagR ())
        {
          entry->SetRouter (true);
        }
    }
  else
    {
      /* we receive a NA so stop the probe timer or delay timer if any */
      entry->StopProbeTimer ();
      entry->StopDelayTimer ();

      /* if the Flag O is clear and mac address differs from the cache */
      if (!naHeader.GetFlagO () && lla.GetAddress ()!=entry->GetMacAddress ())
        {
          if (entry->IsReachable ())
            {
              entry->MarkStale ();
            }
          return;
        }
      else
        {
          if ((!naHeader.GetFlagO () && lla.GetAddress () == entry->GetMacAddress ()) || naHeader.GetFlagO ()) /* XXX lake "no target link-layer address option supplied" */
            {
              entry->SetMacAddress (lla.GetAddress ());

              if (naHeader.GetFlagS ())
                {
                  if (!entry->IsReachable ())
                    {
                      if (entry->IsProbe ())
                        {
                          waiting = entry->MarkReachable (lla.GetAddress ());
                          for (std::list<Ptr<Packet> >::const_iterator it = waiting.begin (); it != waiting.end (); it++)
                            {
                              cache->GetInterface ()->Send (*it, src); 
                            }
                          entry->ClearWaitingPacket ();
                        }
                      else
                        {
                          entry->MarkReachable (lla.GetAddress ());
                        }
                    }
                  entry->StopReachableTimer ();
                  entry->StartReachableTimer ();
                }
              else if (lla.GetAddress ()!=entry->GetMacAddress ())
                {
                  entry->MarkStale ();
                }
              entry->SetRouter (naHeader.GetFlagR ());
            }
        }
    }
}


Ptr<Packet> Icmpv6L4Protocol_NdSimple::Forge_mNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags)
{
  NS_LOG_FUNCTION (this << src << dst << hardwareAddress << (uint32_t)flags);
  Ptr<Packet> p = Create<Packet> ();
  Ipv6Header ipHeader;
  Icmpv6NA na;
  Icmpv6OptionLinkLayerAddress llOption (0, *hardwareAddress);  /* we give our mac address in response */

  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": Send mNA ( from " << src << " to " << dst << " target " << target <<")");;

  /* forge the entire NA packet from IPv6 header to ICMPv6 link-layer option, so that the packet does not pass by Icmpv6L4Protocol::Lookup again */

  p->AddHeader (llOption);
  na.SetCode(Icmpv6L4Protocol_NdSimple::NDAD_CODE);
  na.SetIpv6Target (target);

  if ((flags & 1))
    {
      na.SetFlagO (true);
    }
  if ((flags & 2) && src != Ipv6Address::GetAny ())
    {
      na.SetFlagS (true);
    }
  if ((flags & 4))
    {
      na.SetFlagR (true);
    }

  na.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + na.GetSerializedSize (), PROT_NUMBER);
  p->AddHeader (na);

  ipHeader.SetSourceAddress (src);
  ipHeader.SetDestinationAddress (dst);
  ipHeader.SetNextHeader (PROT_NUMBER);
  ipHeader.SetPayloadLength (p->GetSize ());
  ipHeader.SetHopLimit (Icmpv6L4Protocol_NdSimple::m_HopLimit);

  //MG: add RandomID here

  p->AddHeader (ipHeader);

  return p;
}


Ptr<Packet> Icmpv6L4Protocol_NdSimple::Forge_mNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress)
{
	  NS_LOG_FUNCTION (this << src << dst << target << hardwareAddress);
	  Ptr<Packet> p = Create<Packet> ();
	  Ipv6Header ipHeader;
	  Icmpv6NS ns (target);
	  Icmpv6OptionLinkLayerAddress llOption (1, hardwareAddress);  /* we give our mac address in response */

	  /* if the source is unspec, multicast the NS to all-nodes multicast */
	  if (src.IsAny() && !dst.IsAllNodesMulticast())
	    {
		  //MG: this is probably redundant since dst is set during function call anyway, left for safety
	      dst = Ipv6Address::GetAllNodesMulticast();
	    }

	  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": Send mNS ( from " << src << " to " << dst << " target " << target <<")");

	  //set code value in ICMPv6 header:
	  ns.SetCode(Icmpv6L4Protocol_NdSimple::NDAD_CODE);

	  p->AddHeader (llOption);
	  ns.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + ns.GetSerializedSize (), PROT_NUMBER);
	  p->AddHeader (ns);

	  ipHeader.SetSourceAddress (src);
	  ipHeader.SetDestinationAddress (dst);
	  ipHeader.SetNextHeader (PROT_NUMBER);
	  ipHeader.SetPayloadLength (p->GetSize ());
	  ipHeader.SetHopLimit (m_HopLimit);

	  //MG: add random node ID to the header
	  p->AddHeader (ipHeader);

	  return p;
}

void Icmpv6L4Protocol_NdSimple::FunctionDadTimeout (Ptr<Ipv6Interface> interface, Ipv6Address addr)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC (interface << " " << addr);
  Ipv6InterfaceAddress ifaddr;
  bool found = false;
  uint32_t i = 0;
  uint32_t nb = interface->GetNAddresses ();

  for (i = 0; i < nb; i++)
    {
      ifaddr = interface->GetAddress (i);

      if (ifaddr.GetAddress () == addr)
        {
          found = true;
          break;
        }
    }

  /* for the moment, this function is always called, if we were a victim of a DAD the address is INVALID
   * and we do not set it to PREFERRED
   */
  if (found && ifaddr.GetState () != Ipv6InterfaceAddress::INVALID)
    {
	  if(ifaddr.GetState() == Ipv6InterfaceAddress::TENTATIVE)
	  {
		  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC);
		  NS_LOG_INFO ("Node " << this->m_node->GetId() << ": 1-hop DAD OK for " << ifaddr.GetAddress() << ",no duplicates found. Address in state TENTATIVE_OPTIMISTIC");

		  //TODO: add randomness + ndad delay, time to wait for ndad to finish (randomness not needed - is is added before DAD)
		  //NDAD_DELAY kept here although probably not necessary without MPRs
		  Simulator::Schedule (Seconds (Icmpv6L4Protocol_Ndpp::NDAD_DELAY), &Icmpv6L4Protocol_NdSimple::Do_nDAD, this, addr, interface);
		  Simulator::Schedule (Seconds (Icmpv6L4Protocol_Ndpp::NDAD_DELAY) +
				  MilliSeconds (Icmpv6L4Protocol_NdSimple::m_retransTimerNDad),
				  &Icmpv6L4Protocol::FunctionDadTimeout, this, interface, addr);
		  return;
	  }
	  else if (ifaddr.GetState() == Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC)
	  {
		  //we are done with DAD++
		  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::PREFERRED);
		  NS_LOG_INFO ("Node " << this->m_node->GetId() << ": DAD++ OK for " << ifaddr.GetAddress() << ",no duplicates found. Address in state PREFERRED");

		  /* send an RS if our interface is not forwarding (router) and if address is a link-local ones
		   * (because we will send RS with it)
		   */
		  Ptr<Ipv6> ipv6 = m_node->GetObject<Ipv6> ();

		  if (!ipv6->IsForwarding (ipv6->GetInterfaceForDevice (interface->GetDevice ())) && addr.IsLinkLocal ())
			{
			  /* XXX because all nodes start at the same time, there will be many of RS arround 1 second of simulation time
			   * TODO Add random delays before sending RS
			   */
			  Simulator::Schedule (Seconds (0.0), &Icmpv6L4Protocol::SendRS, this, ifaddr.GetAddress (), Ipv6Address::GetAllRoutersMulticast (), interface->GetDevice ()->GetAddress ());
			}
	  }
    }
  else if (!found)
	  NS_LOG_DEBUG("Node " << this->m_node->GetId() << ": DAD time elapsed: no interface address found for " << addr << ". ");
}

void Icmpv6L4Protocol_NdSimple::DoDAD (Ipv6Address target, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << target << interface);
  Ipv6Address addr;
  Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();
  NS_ASSERT (ipv6);

  if(!m_alwaysDad)
    {
      return;
    }

  /* TODO : disable multicast loopback to prevent NS probing to be received by the sender */

  Ptr<Packet> p = ForgeNS ("::",Ipv6Address::MakeSolicitedAddress (target), target, interface->GetDevice ()->GetAddress ());

  /* update last packet UID */
  interface->SetNsDadUid (target, p->GetUid ());

  //trace:
  m_txNdTrace (p, NDPP_NS_1HOP, NO_MPR_OPT, DAD_INIT,
	  ipv6->GetInterfaceForDevice(interface->GetDevice()), Seconds(0.0));

  interface->Send (p, Ipv6Address::MakeSolicitedAddress (target));
}

void Icmpv6L4Protocol_NdSimple::Do_nDAD (Ipv6Address target, Ptr<Ipv6Interface> interface)
{
	 NS_LOG_FUNCTION (this << target << interface);

	  Ipv6Address addr;
	  Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();

	  NS_ASSERT (ipv6);

	  if(!m_alwaysDad)
	    {
	      return;
	    }

	  /* TODO : disable multicast loopback to prevent NS probing to be received by the sender */

//MG: src must be changed for ND++ to allow for using MPRs
	  NS_LOG_INFO ("Node " << this->m_node->GetId() << ": Starting n-DAD for " << target << ". ");

	  Ptr<Packet> p = Forge_mNS ("::",Ipv6Address::GetAllNodesMulticast(), target, interface->GetDevice ()->GetAddress ());

	  /* update last packet UID */
	  interface->SetNsDadUid (target, p->GetUid());

	  //add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
	  //flowId tag is used since it's capabilities are good for this purpose
	  FlowIdTag randomId(this->m_node->GetId());
	  p->AddPacketTag(randomId);

	  //trace:
	  m_txNdTrace (p, NDPP_NS_MHOP, NO_MPR_OPT, NDAD_INIT,
			  ipv6->GetInterfaceForDevice(interface->GetDevice()), Seconds(0.0));

	  interface->Send (p, Ipv6Address::GetAllNodesMulticast());
}

void Icmpv6L4Protocol_NdSimple::SetHopLimit(uint32_t hopLimit)
{
	m_HopLimit = hopLimit;
}


} /* namespace ns3 */

