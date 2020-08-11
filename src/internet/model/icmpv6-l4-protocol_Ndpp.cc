/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 ND modification 2 (Ndpp)
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
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

#include "ipv6-raw-socket-factory-impl.h"
#include "ipv6-l3-protocol.h"
#include "ipv6-interface.h"
#include "icmpv6-l4-protocol.h"
#include "ndisc-cache.h"

//MG:
#include "ns3/flow-id-tag.h"
#include "mpr-lists.h"
#include "ns3/nstime.h"



namespace ns3
{
//ATTRIBUTE_HELPER_H (Icmpv6L4Protocol_Ndpp);

NS_OBJECT_ENSURE_REGISTERED (Icmpv6L4Protocol_Ndpp);
NS_OBJECT_ENSURE_REGISTERED(Icmpv6L4Protocol_NdppFrw);
NS_OBJECT_ENSURE_REGISTERED(Icmpv6L4Protocol_NdppMpr);

NS_LOG_COMPONENT_DEFINE ("Icmpv6L4Protocol_Ndpp");
//NS_LOG_COMPONENT_DEFINE("Icmpv6L4Protocol_NdppFrw");

const double Icmpv6L4Protocol_Ndpp::NDAD_DELAY = 3;

Icmpv6L4Protocol_Ndpp::Icmpv6L4Protocol_Ndpp()
{
	m_willingness = NeighInfo::WILL_DEFAULT;
}

Icmpv6L4Protocol_Ndpp::~Icmpv6L4Protocol_Ndpp()
{
	NS_LOG_FUNCTION_NOARGS ();
}

TypeId Icmpv6L4Protocol_Ndpp::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Icmpv6L4Protocol_Ndpp")
    .SetParent<Icmpv6L4Protocol_NdSimple> ()
    .AddConstructor<Icmpv6L4Protocol_Ndpp> ()
    .AddAttribute ("nDADcount", "Number of consecutive n-DAD trials",
			UintegerValue (1),
			MakeUintegerAccessor (&Icmpv6L4Protocol_Ndpp::m_nDADcount),
			MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MprStartDelay",
    		"A random delay before sending first MPR Parameters message",
			StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),//remember to set max according to MPRP_DELAY!!!!
			MakePointerAccessor (&Icmpv6L4Protocol_Ndpp::m_mprStartDelay),
			MakePointerChecker<RandomVariableStream> ())
	.AddTraceSource("Latency", "Track latency to verify state of address(es) on this interface",
				MakeTraceSourceAccessor (&Icmpv6L4Protocol_Ndpp::m_latencyTrace))
  ;
  return tid;
}



void Icmpv6L4Protocol_Ndpp::HandleNS_local (Ptr<Packet> packet, Ipv6Header ip, Ipv6Address &src, Ipv6Address &dst, Ptr<Ipv6Interface> interface)
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
  if(nsHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE && dst.IsAllNodesMulticast() && ip.GetHopLimit() > 1)
  {
	  // NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NS message received");
	  //MG: MPRs processing here
	  bool forward_ok = false;
	  //if lists are not initialized, or MPR processes not started - do not forward
	  SelectorsI it_sel;
	 // Ptr<Ipv6Interface_Ndpp> interfaceNdpp = interface->GetDevice()->GetObject<Ipv6Interface_Ndpp> ();
	  Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);

	  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
			  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
	  {
		  if(src.IsEqual(it_sel->m_addr))
		  {

			  if(it_sel->m_val_t >= Simulator::Now())
				  forward_ok = true;
			  else
			  {
				  if(!it_sel->m_addr.IsAny())	//never remove "::", MG: changed from kernel impl.
				  {
				  //remove old selector
				  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": old MPR selector removed: " << it_sel->m_addr);
				 interfaceNdpp->m_mprData.m_selectors.erase(it_sel);
				  }
			  }
			  break;
		  }
	  }

	  if(forward_ok)
	  {

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
					(m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice())
					, MilliSeconds(frwDelay));

			Simulator::Schedule(MilliSeconds(frwDelay), &Ipv6Interface::Send, interface,
					p, Ipv6Address::GetAllNodesMulticast());
			//interface->Send (p, Ipv6Address::GetAllNodesMulticast());

			NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NS message forwarded with "
					"HopLimit = " << (int)ip.GetHopLimit());
	  }
	  else
		  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NS message dropped!");
  }

  packet->RemoveHeader (nsHeader);

  Ipv6Address target = nsHeader.GetIpv6Target ();

  //MG: for mNS adjust src and dst: change dst to solicited node multicast, to process packet normally
  //also change src to unspecified

  if(nsHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE && dst.IsAllNodesMulticast())
  {
	  //trace:
	  messageType = NDPP_NS_MHOP;

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
    		  //trace:
			   m_rxNdTrace (packet, &nsHeader, messageType, NO_MPR_OPT, DISCARD_NS_INVALID_ADDR,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

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
  /*if (packet->GetUid () == ifaddr.GetNsDadUid () )
    {
      //don't process our own DAD probe
      NS_LOG_LOGIC ("Hey we receive our DAD probe!");
      return;
    }*/

  if(tentative_dad)
  {
	  //somebody else is doing DAD on my tentative address, I cannot use it
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

void Icmpv6L4Protocol_Ndpp::HandleNA (Ptr<Packet> packet, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  Icmpv6NA naHeader;
  Icmpv6OptionLinkLayerAddress lla (1);

  //collecting tracing information:
  NdppMessageType_e messageType;
  MprOptType_e mprOptType = NO_MPR_OPT;

 // Ptr<Ipv6Interface_Ndpp> interfaceNdpp = interface->GetDevice()->GetObject<Ipv6Interface_Ndpp> ();
  Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);

  //MG: forward a copy of multihop NA packets:
  packet->PeekHeader(naHeader);
  if(naHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE && dst.IsAllNodesMulticast() && ip.GetHopLimit() > 1)
  {
	  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NA message received");

	  //MPRs processing here
	  bool forward_ok = false;
	  //if lists are not initialized, or MPR processes not started - do not forward
	  SelectorsI it_sel;
	  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
			  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
	  {
		  if(src.IsEqual(it_sel->m_addr))
		  {

			  if(it_sel->m_val_t >= Simulator::Now())
				  forward_ok = true;
			  else
			  {
				  if(!it_sel->m_addr.IsAny())	//never remove "::", MG: changed from kernel impl.
				  {
				  //remove old selector
				  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": old MPR selector removed: " << it_sel->m_addr);
				  interfaceNdpp->m_mprData.m_selectors.erase(it_sel);
				  }
			  }
			  break;
		  }
	  }

	  if(forward_ok)
	  {
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
	  else
		  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NA message dropped!");
  }

  //for tracing purposes only:
  messageType = (naHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE)? NDPP_NA_MHOP : NDPP_NA_1HOP;
  Ptr<Packet> p_copy = packet->Copy();	//copy packet for tracing

  packet->RemoveHeader (naHeader);
  Ipv6Address target = naHeader.GetIpv6Target ();

  Address hardwareAddress;
  NdiscCache::Entry* entry = 0;
  Ptr<NdiscCache> cache = FindCache (interface->GetDevice ());
  std::list<Ptr<Packet> > waiting;

  /* XXX search all options following the NA header */
   /* Get LLA */
   uint8_t type;
   Icmpv6OptionMprParams* mprp_opt = NULL;	//this cannot be used twice! - different
   	   	   	   	   	   	   	   	   //object needed for each remove header!!!!
   MprOptsList mprp_opts;
   Icmpv6OptionMprAnnouncement mpra_opt;

   bool mpraFound = false;
   bool llaFound = false;

   OneHopSetI it;
   Ipv6Address my_main_addr;


   //read through all opts, then process (LLa is processed later)
   while(packet->GetSize() > 0)
   {
 	  packet->CopyData (&type, sizeof(type));
 	  switch (type)
 	  {
 	  case Icmpv6Header::ICMPV6_OPT_MPR_PARAMS:
 		  if(mprp_opt)
 			  delete mprp_opt;
 		  mprp_opt = new Icmpv6OptionMprParams();
 		  packet->RemoveHeader(*mprp_opt);
 		  mprp_opts.push_back(*mprp_opt);
 		  break;
 	  case Icmpv6Header::ICMPV6_OPT_MPR_ANNOUNCEMENT:
 		  packet->RemoveHeader(mpra_opt);
 		  mpraFound = true;
 		  break;
 	  case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET:
 		  packet->RemoveHeader (lla);
 		  llaFound = true;
 		  break;
 	  default:
 		  break;
 	  }
   }


   //process options:

   if(!mprp_opts.empty() || mpraFound)
   {
	  //trace:

	  if(!mprp_opts.empty())
		  if(mpraFound)
			  mprOptType = BOTH;
		  else
			  mprOptType = MPR_PARAMS;
	  else
		  mprOptType = MPR_ANNOUNCEMENT;

	  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, MPR_INFO,
		  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

 	  //initiate MPR processes if not started yet on this interface
 	  interfaceNdpp->MprsInit();
 	  my_main_addr = interface->GetLinkLocalAddress().GetAddress();

   }

   if(!mprp_opts.empty())
   {
 	  bool new_entry = true;

 	  for(it = interfaceNdpp->m_mprData.m_neighbours.begin();
 	  				  it != interfaceNdpp->m_mprData.m_neighbours.end(); it++)
 	  {
 		  if(it->m_addr.IsEqual(target))
 		  {
 			  //update entry (clean 2-hop set)
 			  it->m_2hops.clear();
 			  it->m_other = 0x00;
 			  new_entry = false;
 			  break;	//only one entry for a neighbour is allowed
 		  }
 	  }
 	  //add new entry or update old one along with adding new 2hop neighbours:
 	  if(new_entry)
 	  {
 		  interfaceNdpp->m_mprData.m_neighbours.push_front(OneHop(target));
 		  it = interfaceNdpp->m_mprData.m_neighbours.begin();
 		  //set times according to [RFC 3626]
 		  it->m_sym_t = Simulator::Now() - Seconds(1.0); //expired
 		  it->m_lost_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
 		  it->m_neightype = NeighInfo::NOT_NEIGH;

 		  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": MPR Parameters - new neighbour added: " << it->m_addr <<" with sym_t="
 				  << it->m_sym_t.GetSeconds() <<", lost_t=" << (double)it->m_lost_t.GetSeconds());
 	  }
 	  else
 		 NS_LOG_LOGIC("Node " << this->m_node->GetId() << " got NA from a known neighbor with: address " << it->m_addr <<
 		  			  ", willingness=" << (int) it->m_willingness << ", asum_t=" <<it->m_asym_t.GetSeconds());

 	  it->m_willingness = mprp_opts.begin()->m_willingness;
 	  it->m_asym_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);


 	  //add nodes from this and other opts to 2hopSet:

 	  //go through all MPR Parameters opts received from this neighbour (contain 2-hop neighbourhood)
 	  MprOptsI it_opt;
 	  AddrListCI it_addr;
 	  for(it_opt = mprp_opts.begin(); it_opt != mprp_opts.end(); it_opt++)
 	  {
 		  if(it_opt->m_linkCode == NeighInfo::LC_INVALID)
 			  continue;		//discard this option

 		  //go through all entries for one opt
 		  for(it_addr = it_opt->m_NeighAddresses.begin(); it_addr != it_opt->m_NeighAddresses.end(); it_addr++)
 		  {
 			  u_int8_t link_type = it_opt->m_linkCode & 0x03;
 			  if(!my_main_addr.IsEqual(*it_addr))	//ignore my MAIN address listed!
 			  {
 				  //check if there is n-DAD ongoing for the found address:
 				  //this might be a case if my 2-hop neighbour holds the same address
 				  //this neighbour actually has a right to do so, so I should include it in my lists!!!
 				  //confusion might be in the intermediate node!
 				  it->m_2hops.push_front(NeighInfo(*it_addr));
 				  it->m_2hops.begin()->m_linktype = it_opt->m_linkCode & 0x03;
 				  it->m_2hops.begin()->m_neightype = it_opt->m_linkCode & 0x0C;
 				  if(it_opt->m_willingness < it->m_willingness)
 				  {
 					  it->m_willingness = it_opt->m_willingness;
 					  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": MPR Parameters options found with different "
 							  "willingness for the same node, the smaller value is chosen");

 				  }
 				  if(it->m_2hops.begin()->m_neightype == NeighInfo::SYM_NEIGH)
 					  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": new symmetric 2-hop (" << it->m_2hops.begin()->m_addr <<") added for neighbor" << it->m_addr);
 			  }
 			  else
 			  {
 				  if(link_type == NeighInfo::LOST_LINK)
 				  {
 					  it->m_sym_t = Simulator::Now() - Seconds(1.0);	//expired -> I am not heard anymore
 					  it->m_neightype = NeighInfo::NOT_NEIGH;
 					  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": neighbor with address" << it->m_addr << "advertises us as lost");
 				  }
 				  else if(link_type == NeighInfo::SYM_LINK || link_type == NeighInfo::ASYM_LINK)
 				  {
 					  if(it->m_sym_t < Simulator::Now())
 						 NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": new symmetric neighbor:" << it->m_addr);
					  else
						 NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": symmetric neighbor:" << it->m_addr);

 					  it->m_sym_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
 					  it->m_lost_t = Simulator::Now() + Seconds(OneHop::LOST_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);	//sym_t+ neigh. hold time
 					  it->m_neightype = NeighInfo::SYM_NEIGH;
 				 }
 			  }
 		  }
 	  }
 	  it->m_lost_t = (it->m_asym_t > it->m_lost_t)? it->m_asym_t : it->m_lost_t;	//select bigger one
   }
   mprp_opts.clear();
   if(mprp_opt)
	   delete mprp_opt;

   if(mpraFound)
   {
 	  AddrListCI it_addr;
 	  SelectorsI it_sel;
 	 //go through all addresses listed in this opt
 	  for(it_addr = mpra_opt.m_MprAddresses.begin(); it_addr != mpra_opt.m_MprAddresses.end(); it_addr++)
 	  {
 		  //only main address can be announced, so compare with this one only
 		  if(my_main_addr.IsEqual(*it_addr))
 		  {
 			  //add MPR Selector (if not present on the list yet):
 			  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
 			  	  				  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
 			  {
 				  if(target.IsEqual(it_sel->m_addr))
 					  break;
 			  }
 			  if(it_sel == interfaceNdpp->m_mprData.m_selectors.end())	//new selector
 			  {
 				  interfaceNdpp->m_mprData.m_selectors.push_front(Selector(target));
 				  it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
 				 NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": new MPR Selector " << it_sel->m_addr);
 			  }
 			  else
 				  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": old MPR Selector updated" << it_sel->m_addr);

 			  it_sel->m_val_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
 			  break;	//even if more of mine addresses are listed, I just need to have one entry for my MPR Selector
 		  }
 	  }
   }
/*
   if (lla.GetLength())
   {
	   packet->RemoveHeader (lla);
   }
*/


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
				  m_rxNdTrace (p_copy, &naHeader, NDPP_NA_MHOP, mprOptType, OWN_DAD_NA_PROBE,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
				  return;
			  }
    	  }

		  if (ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE || ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC)
		  {
			  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::INVALID);
			  NS_LOG_INFO("Node " << this->m_node->GetId() << ": DAD detected duplicated address: " << ifaddr.GetAddress() << ". The address is INVALID");

			  //track ND++ latency:
			 m_latencyTrace(interface, ifaddr.GetAddress(), INVALID);

			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, DAD_DUPLICATION,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
		  else if (ifaddr.GetState () == Ipv6InterfaceAddress::PREFERRED)
		  {
			  //somebody is using my address, which is valid. Probably there is a duplication in the network
			  NS_LOG_INFO("Node " << this->m_node->GetId() << ": Duplicated IPv6 address: " << ifaddr.GetAddress());
			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, WARNING_DUPLICATION_POSSIBLE,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
		  else if (ifaddr.GetState () == Ipv6InterfaceAddress::INVALID)
		  {
			 //duplicated message to the address already marked invalid - register and discard
			  NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": duplicated message to the address already marked invalid "
					  << ifaddr.GetAddress());
			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, DISCARD_NA_INVALID_ADDR,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
      }
      /* we have not initiated any communication with the target so... discard the NA */
      //trace: (without this else some nDAD packets were registered twice!)
      else if(mprOptType == NO_MPR_OPT)	//otherwise this packet was already registered in trace
		  m_rxNdTrace (p_copy, &naHeader, messageType, NO_MPR_OPT, DISCARD_NA,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
      return;
    }

  //MG: just for safety, maybe unnecessary;
  if(naHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE)
	  return;


  if (!llaFound)
  {
	return;
  }

  //trace:
   m_rxNdTrace (packet, &naHeader, messageType, NO_MPR_OPT, LLA_NEIGH_CACHE_UPDATE,
 	  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

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


/*Ptr<Packet> Icmpv6L4Protocol_Ndpp::Forge_mNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags)
{
  NS_LOG_FUNCTION (this << src << dst << hardwareAddress << (uint32_t)flags);
  Ptr<Packet> p = Create<Packet> ();
  Ipv6Header ipHeader;
  Icmpv6NA na;
  Icmpv6OptionLinkLayerAddress llOption (0, *hardwareAddress);   we give our mac address in response

  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": Send mNS ( from " << src << " to " << dst << " target " << target <<")");;

   forge the entire NA packet from IPv6 header to ICMPv6 link-layer option, so that the packet does not pass by Icmpv6L4Protocol::Lookup again

  p->AddHeader (llOption);
  na.SetCode(Icmpv6L4Protocol_Ndpp::NDAD_CODE);
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
  ipHeader.SetHopLimit (Icmpv6L4Protocol_Ndpp::m_HopLimit);

  //MG: add RandomID here

  p->AddHeader (ipHeader);

  return p;
}


Ptr<Packet> Icmpv6L4Protocol_Ndpp::Forge_mNS (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address hardwareAddress)
{
	  NS_LOG_FUNCTION (this << src << dst << target << hardwareAddress);
	  Ptr<Packet> p = Create<Packet> ();
	  Ipv6Header ipHeader;
	  Icmpv6NS ns (target);
	  Icmpv6OptionLinkLayerAddress llOption (1, hardwareAddress);   we give our mac address in response

	   if the source is unspec, multicast the NS to all-nodes multicast
	  if (src.IsAny() && !dst.IsAllNodesMulticast())
	    {
		  //MG: this is probably redundant since dst is set during function call anyway, left for safety
	      dst = Ipv6Address::GetAllNodesMulticast();
	    }

	  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": Send mNS ( from " << src << " to " << dst << " target " << target <<")");

	  //set code value in ICMPv6 header:
	  ns.SetCode(Icmpv6L4Protocol_Ndpp::NDAD_CODE);

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
}*/

void Icmpv6L4Protocol_Ndpp::FunctionDadTimeout (Ptr<Ipv6Interface> interface, Ipv6Address addr)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC (interface << " " << addr);

  //stop this scheduled procedure if interface became down!!!
  //Default behaviour is that sending is blocked in NS-3, but scheduled DAD, n_DAD in upper layer are still running
  //this should not be a case
  if(!interface->IsUp())
  	return;

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
   *
   *MG: if INVALID -> reply came, no need for n-DAD (first or next if current n_dadCount is less than
   *specified
   */

  if (found && ifaddr.GetState () != Ipv6InterfaceAddress::INVALID)
  {
	  Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);
	  NS_ASSERT(interfaceNdpp != NULL);

	  if(ifaddr.GetState() == Ipv6InterfaceAddress::TENTATIVE)
	  {
		  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC);
		  NS_LOG_INFO ("Node " << (double) this->m_node->GetId() << ": 1-hop DAD OK for " << ifaddr.GetAddress() << ",no duplicates found. Address in state TENTATIVE_OPTIMISTIC");

		  /*************************this part moved to do_nDAD !!!!!!!!!!!!!!!!!!******************
		  //choose MPRs, send MPR announcement and wait:
		  //Ptr<Ipv6Interface_Ndpp> interfaceNdpp = interface->GetDevice()->GetObject<Ipv6Interface_Ndpp> ();
		  Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);
		  NS_ASSERT(interfaceNdpp != NULL);

		  interfaceNdpp->MprLookUp();

		  if(!interfaceNdpp->m_mprData.m_mprs.empty())	//e.g. if no 2-hop neigh. list can be empty
		  {
			  NS_LOG_INFO ("Node " << this->m_node->GetId() << ": send first MPR Announcement option after MPR selection");
			  //todo: this is probably not necessary, because nd6_mpr_timer would send it anyway soon

			  Ipv6Address src = interface->GetLinkLocalAddress().GetAddress();
			  Address hardwareAddress = interface->GetDevice ()->GetAddress ();
			  Ptr<Packet> p = ForgeNA(src, Ipv6Address::GetAllNodesMulticast(), src, &hardwareAddress,
					 interface->IsForwarding()? 4 : 0, Icmpv6L4Protocol_Ndpp::MPR_ANNOUNCEMENT, &interfaceNdpp->m_mprData);

			 //add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
			 FlowIdTag randomId(this->m_node->GetId());
			 p->AddPacketTag(randomId);

			 //trace:
			 m_txNdTrace (p, NDPP_NA_1HOP, MPR_ANNOUNCEMENT,MPR_PROCESSES,
					 (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()),
					 Seconds(0.0));

			 Simulator::Schedule (Seconds (0.0), &Ipv6Interface::Send, interface, p, Ipv6Address::GetAllNodesMulticast());

		  }
		  */

		  Simulator::Schedule(Seconds(Icmpv6L4Protocol_Ndpp::NDAD_DELAY),
				  &Icmpv6L4Protocol_Ndpp::SendAsyncMPRann, this, interface);
		  //DONE add randomness (not needed) + ndad delay, time to wait for ndad to finish
		  Simulator::Schedule (Seconds (Icmpv6L4Protocol_Ndpp::NDAD_DELAY),
				  &Icmpv6L4Protocol_Ndpp::Do_nDAD, this, addr, interface);
		  //ndad_delay may be too long, this time is needed for the others to read MPR announcement -> not only,
		  //also to wait for the MPR processes to finish and confirm 2-sided connections, for the node that assigned
		  //its first address it is more secure to have this time longer

		  //add n_dad count entry for this interface and address to the registry
		  std::pair<Ipv6Interface_Ndpp::NdadCountMapI, bool> insertResult =
				  interfaceNdpp->m_nDADregistry.insert(std::make_pair(addr, 1));
		  NS_ASSERT_MSG(insertResult.second, "nDAD registry entry already exists for address " << addr
				  << "but the address is TENATTIVE and nDAD has just been started for the first time!!!");

		  Simulator::Schedule (Seconds (Icmpv6L4Protocol_Ndpp::NDAD_DELAY) +
				  MilliSeconds (Icmpv6L4Protocol_NdSimple::m_retransTimerNDad),
				  &Icmpv6L4Protocol::FunctionDadTimeout, this, interface, addr);
		 /* if(m_nDADcount==1)
		  			  Simulator::Schedule (Seconds (Icmpv6L4Protocol_Ndpp::NDAD_DELAY) +
		  					  MilliSeconds (Icmpv6L4Protocol_NdSimple::m_retransTimerNDad),
		  					  &Icmpv6L4Protocol::FunctionDadTimeout, this, interface, addr);
		  		  else
		  			  Simulator::Schedule (Seconds (Icmpv6L4Protocol_Ndpp::NDAD_DELAY) +
		  					  MilliSeconds (Icmpv6L4Protocol::RETRANS_TIMER),
		  					  &Icmpv6L4Protocol::FunctionDadTimeout, this, interface, addr);*/
		  return;
	  }
	  else if (ifaddr.GetState() == Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC)
	  {
		  Ipv6Interface_Ndpp::NdadCountMapI it = interfaceNdpp->m_nDADregistry.find(addr);
		  NS_ASSERT_MSG(it != interfaceNdpp->m_nDADregistry.end(), "nDAD registry with no corresponding entry for address"
				  <<addr << "but address in a TENTATIVE_OPTIMISTIC state");
		  if(it->second < m_nDADcount)
		  {
			  //send another n-DAD query for this interface and address
			  //TODO: check delay here!!!
			  Simulator::Schedule (Seconds (0.0),
					  &Icmpv6L4Protocol_Ndpp::Do_nDAD, this, addr, interface);
			  Simulator::Schedule (MilliSeconds (Icmpv6L4Protocol_NdSimple::m_retransTimerNDad),
			  				  &Icmpv6L4Protocol::FunctionDadTimeout, this, interface, addr);

			  //increase n_dad counter for this interface and address
			  (it->second)++;
			  return;
		  }
		  else
		  {
			  //we are done with DAD++
			  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::PREFERRED);
			  NS_LOG_INFO ("Node " << (double) this->m_node->GetId() << ": DAD++ OK for " << ifaddr.GetAddress() << ",no duplicates found. Address in state PREFERRED");

			  //track ND++ latency:
			  m_latencyTrace(interface, addr, PREFERRED);

			  /* send an RS if our interface is not forwarding (router) and if address is a link-local ones
			   * (because we will send RS with it)
			   */
			  Ptr<Ipv6> ipv6 = m_node->GetObject<Ipv6> ();

			  if (!ipv6->IsForwarding (ipv6->GetInterfaceForDevice (interface->GetDevice ())) && addr.IsLinkLocal ())
			  {
				  /* XXX because all nodes start at the same time, there will be many of RS around 1 second of simulation time
				   * TODO Add random delays before sending RS -  this is done - in SendRS there is a solicitation delay added
				   */
				  Simulator::Schedule (Seconds (0.0), &Icmpv6L4Protocol::SendRS, this, ifaddr.GetAddress (), Ipv6Address::GetAllRoutersMulticast (), interface->GetDevice ()->GetAddress ());
			  }
		  }
	  }
    }
  else if (!found)
	  NS_LOG_DEBUG("Node " << this->m_node->GetId() << ": DAD time elapsed: no interface address found for " << addr << ". ");
}

void Icmpv6L4Protocol_Ndpp::SendAsyncMPRann( Ptr<Ipv6Interface> interface)
{
	//moved here from do_nDAD!!!!!!!!!!!!!!!!!!!!!


	//choose MPRs, send MPR announcement before sending nDAD packet (just for safety):
	//Ptr<Ipv6Interface_Ndpp> interfaceNdpp = interface->GetDevice()->GetObject<Ipv6Interface_Ndpp> ();
	Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);
	NS_ASSERT(interfaceNdpp != NULL);

	interfaceNdpp->MprLookUp();

	if(!interfaceNdpp->m_mprData.m_mprs.empty())	//e.g. if no 2-hop neigh. list can be empty
	{
		Ipv6Address src = interface->GetLinkLocalAddress().GetAddress();
		if(src.IsAny())
		{
			 //this should not happen, but is possible - e.g. when a duplication was detected through the reception of
			 //NA MPR options message - after DAD but before scheduled nDAD start and before sending async. MPR Ann.- the address is then removed,
			 //new one is assigned, but it is not after DAD yet, so there is no working address to use
			 NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": No valid link local address forasyncroneous MPR Ann., returned with no action");
			 return;
		}

		NS_LOG_INFO ("Node " << this->m_node->GetId() << ": send MPR Announcement before starting nDAD");


		Address hardwareAddress = interface->GetDevice ()->GetAddress ();
		Ptr<Packet> p_mpr = ForgeNA(src, Ipv6Address::GetAllNodesMulticast(), src, &hardwareAddress,
		 interface->IsForwarding()? 4 : 0, Icmpv6L4Protocol_Ndpp::MPR_ANNOUNCEMENT, &interfaceNdpp->m_mprData);

		//add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
		FlowIdTag randomId(this->m_node->GetId());
		p_mpr->AddPacketTag(randomId);

		//trace:
		m_txNdTrace (p_mpr, NDPP_NA_1HOP, MPR_ANNOUNCEMENT,MPR_PROCESSES,
		 (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()),
		 Seconds(0.0));

		interface->Send(p_mpr, Ipv6Address::GetAllNodesMulticast());
	}
}

void Icmpv6L4Protocol_Ndpp::Do_nDAD (Ipv6Address target, Ptr<Ipv6Interface> interface)
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
	 Ipv6Address src = interface->GetLinkLocalAddress().GetAddress();
	 if(src.IsAny())
	 {
		 //this should not happen, but is posisble - e.g. when a duplication was detected through the reception of
		 //NA MPR options message - after DAD but before scheduled nDAD starte d- the address is then removed,
		 //new one is assigned, but it is not after DAD yet, so there is no working address to use
		 NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": No valid link local address for n-DAD, nDAD stopped");
		 return;
	 }

	 NS_LOG_INFO ("Node " << this->m_node->GetId() << ": Starting n-DAD for " << target << ". ");
	 Ptr<Packet> p = Forge_mNS (src,Ipv6Address::GetAllNodesMulticast(), target, interface->GetDevice ()->GetAddress ());

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

Ptr<Packet> Icmpv6L4Protocol_Ndpp::ForgeNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags,
		uint8_t mpr_opts, MprData* mpr_data)
{
  NS_LOG_FUNCTION (this << src << dst << hardwareAddress << (uint32_t)flags);
  Ptr<Packet> p = Create<Packet> ();
  Ipv6Header ipHeader;
  Icmpv6NA na;
  Icmpv6OptionLinkLayerAddress llOption (0, *hardwareAddress);  /* we give our mac address in response */

  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": Send NA ( from " << src << " to " << dst << ")");

  /* forge the entire NA packet from IPv6 header to ICMPv6 link-layer option, so that the packet does not pass by Icmpv6L4Protocol::Lookup again */

  //include mprp and mpra options:
  if(mpr_opts & MPR_ANNOUNCEMENT)
  {
	  NeighSetI it = mpr_data->m_mprs.begin();
	  Icmpv6OptionMprAnnouncement mpraOpt;

	  while (it != mpr_data->m_mprs.end())
	  {
		  mpraOpt.AddMPR(it->m_addr);
		  it++;
	  }

	  //add option: (even if it is empty)
	  p->AddHeader(mpraOpt);
  }


  if(mpr_opts & MPR_PARAMS)
  {
	  //add neighbours to appropriate options based on the link code field

	  Icmpv6OptionMprParams *mprPramsOpts[NeighInfo::LC_ALL_ENTRIES] = {NULL};	//array of pointers to MPRParams options for each link code
	  //options to send
	  u_int8_t link_code = 0x00;

	  OneHopSetI it = mpr_data->m_neighbours.begin();

	  if(!mpr_data->m_neighbours.empty())
	  {

		  while(it != mpr_data->m_neighbours.end())	//begin equals end for empty list
		  {
			  if(it->m_lost_t < Simulator::Now())
			  {
				  //remove neighbor entry - the link is lost permanently
				  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": neighbor removed (lost): " << it->m_addr);
				  it->m_2hops.clear();
				  it = mpr_data->m_neighbours.erase(it);
				  continue;
			  }
			  else if(it->m_sym_t >= Simulator::Now())	//not expired
			  {
				  it->m_linktype = NeighInfo::SYM_LINK;
				  it->m_neightype = NeighInfo::SYM_NEIGH;
			  }
			  else if(it->m_asym_t >= Simulator::Now())	//sym. time expired, asym. time not expired
			  {
				  it->m_linktype = NeighInfo::ASYM_LINK;
				  it->m_neightype = NeighInfo::NOT_NEIGH;
			  }
			  else	//both sym. and asym. times expired, lost time not expire
			  {
				  it->m_linktype = NeighInfo::LOST_LINK;
				  it->m_neightype = NeighInfo::NOT_NEIGH;
			  }

			  link_code = it->m_linktype | it->m_neightype;
			  switch(link_code)
			  {
			  //ignore mpr_neigh. entries - they shouldn't be in this set
			  case 0x00:	//not neigh, unspec. link
			  case 0x01:	//not neigh, asymmetric link
			  case 0x03:	//not neigh, lost link
			  case 0x04:	//symm. neigh, unspec. link
			  case 0x05:	//symm. neigh, asymm. link
			  case 0x06:	//symm. neigh, symm. link
			  case 0x07:	//symm. neigh, lost link
			  //case 0x08:
			  //case 0x09:
			  //case 0x0a:
			  //case 0x0b:
				  if(mprPramsOpts[link_code] == NULL)
				  {
					  mprPramsOpts[link_code] = new Icmpv6OptionMprParams (m_willingness, link_code);
					 // NS_LOG_LOGIC("New MPR Params Opt added for link code: " << link_code);
				  }
				  mprPramsOpts[link_code]->AddNeighbor(it->m_addr);
				  break;
			  default:
				  break;	//silently ignore
			  }
			  it++;
		  }
	  }
	  else
	  {
		  //send mpr params with no neighbors listed. It can be stored in cell LC_INVALID, as it will be empty anyway
		  mprPramsOpts[NeighInfo::LC_INVALID] = new Icmpv6OptionMprParams (m_willingness);	//NOT_NEIGH, UNSPEC_LINK;
	  }

	  //add options:
	  for(int i=0; i<NeighInfo::LC_ALL_ENTRIES; i++)
	  {
		  if(mprPramsOpts[i] != NULL)
		  {
		//	  NS_LOG_LOGIC("Adding header for the MPR Params Opt with link code = " << i);
			  p->AddHeader(*mprPramsOpts[i]);
			  delete mprPramsOpts[i];
		  }
	  }
  }


  p->AddHeader (llOption);

  //changed below according to RFC 4861, chapter 7.2.4. (assumption - ForgeNA is to send solicited NAs only!)
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
  ipHeader.SetHopLimit (255);

  p->AddHeader (ipHeader);

  return p;
}

int64_t Icmpv6L4Protocol_Ndpp::AssignStreams(int64_t stream)
{
	int64_t currStream = stream;
	currStream += Icmpv6L4Protocol::AssignStreams(stream);

	NS_LOG_FUNCTION (this << stream);
	m_mprStartDelay->SetStream (currStream);
	currStream++;
	return currStream - stream;
}

double Icmpv6L4Protocol_Ndpp::GetMprStartDelay()
{
	return m_mprStartDelay->GetValue();
}

//********************************************************************************//
////////************* NdppFrw: ****************************/////////////////////////
//********************************************************************************//

ForwardingTuple::ForwardingTuple(Ipv6Address targetAddr, /*Ipv6Address dstAddr,*/
		int randomID, int sequenceNo, Ipv6Address srcAddr):
		m_targetAddr(targetAddr),
		//m_dstAddr(dstAddr),
		m_sequenceNo(sequenceNo),
		m_randomID(randomID),
		m_srcAddr(srcAddr)
{
}

bool operator < (const ForwardingTuple& a, const ForwardingTuple& b)
{
	if(a.m_srcAddr.IsAny() && b.m_srcAddr.IsAny())
	{
		bool addrComp = a.m_targetAddr < b.m_targetAddr;
		bool randIdComp =  a.m_randomID < b.m_randomID;
		return (addrComp || (!addrComp && randIdComp)
				|| (!addrComp && !randIdComp && a.m_sequenceNo < b.m_sequenceNo));
	}
	else
	{
		bool addrComp = a.m_srcAddr < b.m_srcAddr;
		bool randIdComp =  a.m_randomID < b.m_randomID;
		return (addrComp || (!addrComp && randIdComp)
				|| (!addrComp && !randIdComp && a.m_sequenceNo < b.m_sequenceNo));
	}
}

bool operator == (const ForwardingTuple& a, const ForwardingTuple& b)
{
	if(a.m_srcAddr.IsAny() && b.m_srcAddr.IsAny())
		return (a.m_srcAddr == b.m_srcAddr && a.m_randomID == b.m_randomID
				&&  a.m_sequenceNo == b.m_sequenceNo);
	else
		return (a.m_targetAddr == b.m_targetAddr && a.m_randomID == b.m_randomID
				&&  a.m_sequenceNo == b.m_sequenceNo);
}

bool operator != (const ForwardingTuple& a, const ForwardingTuple& b)
{
	if(a.m_srcAddr.IsAny() && b.m_srcAddr.IsAny())
		return ! (a.m_targetAddr == b.m_targetAddr && a.m_randomID == b.m_randomID
				&&  a.m_sequenceNo == b.m_sequenceNo);
	else
		return ! (a.m_targetAddr == b.m_targetAddr && a.m_randomID == b.m_randomID
						&&  a.m_sequenceNo == b.m_sequenceNo);
}

/////////////////////************************************////////////////////////////////////

Icmpv6L4Protocol_NdppFrw::Icmpv6L4Protocol_NdppFrw():
		m_sequenceNo(1),
		m_frwCountLimit(1),
		m_DADReplyCountLimit(1)
{
}

Icmpv6L4Protocol_NdppFrw::~Icmpv6L4Protocol_NdppFrw()
{
	m_frwSet.clear();
	m_DADReplySet.clear();
	NS_LOG_FUNCTION_NOARGS ();
}

TypeId Icmpv6L4Protocol_NdppFrw::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Icmpv6L4Protocol_NdppFrw")
    .SetParent<Icmpv6L4Protocol_Ndpp> ()
    .AddConstructor<Icmpv6L4Protocol_NdppFrw> ()
    .AddAttribute ("FrwCountLimit",
    		"A number of times multicast ND++ message can be forwarded",
    		UintegerValue (1),
			MakeUintegerAccessor (&Icmpv6L4Protocol_NdppFrw::m_frwCountLimit),
			MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("DADReplyCountLimit",
			"A number of times a reply to DAD query for the same target can be sent",
			UintegerValue (1),
			MakeUintegerAccessor (&Icmpv6L4Protocol_NdppFrw::m_DADReplyCountLimit),
			MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("frwUseSrcAddr",
				"Whether to use src address differentiation during forwarding packet filtering",
				BooleanValue (0),
				MakeBooleanAccessor(&Icmpv6L4Protocol_NdppFrw::m_frwUseSrcAddr),
				MakeBooleanChecker())
	.AddAttribute ("autoAssignNewAddr",
					"Whether to auto-assign a new address in case there is no valid address on the interface",
					BooleanValue (0),
					MakeBooleanAccessor(&Icmpv6L4Protocol_NdppFrw::m_autoAssignNewAddr),
					MakeBooleanChecker())
  ;
  return tid;
}

int Icmpv6L4Protocol_NdppFrw::GetSequenceNo()
{
	return m_sequenceNo++;
}

void Icmpv6L4Protocol_NdppFrw::ClearFrwSet()
{
	m_frwSet.clear();
}

void Icmpv6L4Protocol_NdppFrw::ClearDADReplySet()
{
	m_DADReplySet.clear();
}

void Icmpv6L4Protocol_NdppFrw::Do_nDAD (Ipv6Address target, Ptr<Ipv6Interface> interface)
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
	 Ipv6Address src = interface->GetLinkLocalAddress().GetAddress();
	 if(src.IsAny())
	 {
		 //this should not happen, but is possisble - e.g. when a duplication was detected through the reception of
		 //NA MPR options message - after DAD but before scheduled nDAD starte d- the address is then removed,
		 //new one is assigned, but it is not after DAD yet, so there is no working address to use
		 NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": No valid link local address for n-DAD, nDAD stopped");
		 return;
	 }

	 NS_LOG_INFO ("Node " << this->m_node->GetId() << ": Starting n-DAD for " << target << ". ");
	 Ptr<Packet> p = Forge_mNS (src,Ipv6Address::GetAllNodesMulticast(), target, interface->GetDevice ()->GetAddress ());

	  /* update last packet UID */
	 interface->SetNsDadUid (target, p->GetUid());

	 //add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
	 //flowId tag is used since it's capabilities are good for this purpose
	 int rndID = this->m_node->GetId();
	 int seqNo = GetSequenceNo();
	 FlowIdTag randomId(rndID);
	 SeqNoTag sequenceNo(seqNo);
	 p->AddPacketTag(randomId);
	 p->AddPacketTag(sequenceNo);

	 //add outgoing packet to FrwSet
	 if(m_frwUseSrcAddr)
		 m_frwSet.insert(std::make_pair(ForwardingTuple(target, rndID, seqNo, src), 1));
	 else
		 m_frwSet.insert(std::make_pair(ForwardingTuple(target, rndID, seqNo), 1));


	 //trace:
	 m_txNdTrace (p, NDPP_NS_MHOP, NO_MPR_OPT, NDAD_INIT,
			  ipv6->GetInterfaceForDevice(interface->GetDevice()), Seconds(0.0));

	 interface->Send (p, Ipv6Address::GetAllNodesMulticast());
}

void Icmpv6L4Protocol_NdppFrw::HandleNS_local (Ptr<Packet> packet, Ipv6Header ip, Ipv6Address &src, Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  Icmpv6NS nsHeader ("::");
  Ipv6InterfaceAddress ifaddr;
  uint32_t nb = interface->GetNAddresses ();
  uint32_t i = 0;
  bool found = false;
  bool tentative_dad = false;
  bool ndad = false;

  uint32_t nodeNo = this->m_node->GetId();

  //collecting tracing information:
   NdppMessageType_e messageType = UNKNOWN;

  //Ipv6Address dst_local = dst;
 // Ipv6Address src_local = src;

  //MG: forward a copy of multihop NS packets:
  packet->PeekHeader(nsHeader);
  if(nsHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE && dst.IsAllNodesMulticast() && ip.GetHopLimit() > 1)
  {
	  // NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Multicast NS message received");
	  //MG: MPRs processing here
	  bool forward_ok = false;
	  //if lists are not initialized, or MPR processes not started - do not forward
	  SelectorsI it_sel;
	 // Ptr<Ipv6Interface_Ndpp> interfaceNdpp = interface->GetDevice()->GetObject<Ipv6Interface_Ndpp> ();
	  Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);

	  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
			  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
	  {
		  if(src.IsEqual(it_sel->m_addr))
		  {

			  if(it_sel->m_val_t >= Simulator::Now() || src.IsAny())
				  forward_ok = true;
			  else
			  {
				  if(!it_sel->m_addr.IsAny())	//never remove "::", MG: changed from kernel impl.
				  {
				  //remove old selector
				  NS_LOG_LOGIC("Node " << nodeNo << ": old MPR selector removed: " << it_sel->m_addr);
				 interfaceNdpp->m_mprData.m_selectors.erase(it_sel);
				  }
			  }
			  break;
		  }
	  }

	  if(forward_ok)
	  {
		  //check the Forwarding Set for duplications first:
		  FlowIdTag randomId;
		  SeqNoTag sequenceNo;
		  NS_ASSERT_MSG(packet->PeekPacketTag(randomId), "Peek Packet Tag for Random ID failed");
		  NS_ASSERT_MSG(packet->PeekPacketTag(sequenceNo), "Peek Packet Tag for Sequence No failed");

		  ForwardingTuple* frwTuple;
		  if(m_frwUseSrcAddr)
			 frwTuple = new ForwardingTuple(nsHeader.GetIpv6Target(), randomId.GetFlowId(),
					  	  sequenceNo.GetFlowId(), src);
		  else
			 frwTuple = new ForwardingTuple(nsHeader.GetIpv6Target(), randomId.GetFlowId(),
		  				  sequenceNo.GetFlowId());

		 /* FrwMapI it = m_frwSet.find(frwTuple);
		  if(it == m_frwSet.end())
		  {
			  //pair not found - insert new entry and forward
			  m_frwSet.insert(std::make_pair(frwTuple, 1));
		  }*/

		  //insert returns pointer to the inserted or found value and bool indicating whether insertion actually took place
		  //this is a work around, because find is most likely not working for maps size bigger than 1
		  std::pair<FrwMapI, bool> insertResult = m_frwSet.insert(std::make_pair(*frwTuple, 1));
		  delete frwTuple;
		  if(insertResult.second == true)
		  {
			  //pair not found -  new entry already inserted, forward
		  }
		  else if((insertResult.first)->second >= m_frwCountLimit)
		  {
			  //we have a duplication - do not process and do not forward further!!!
			  forward_ok = false;
			  NS_LOG_LOGIC("Node " << nodeNo << ": Duplicated multicast NS message from MPR selector: "
					  << src << " - drop it");
			  //trace:
			   m_rxNdTrace (packet, &nsHeader, NDPP_NS_MHOP, NO_MPR_OPT, DUPLICATED_MNS_DROPPED,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
			   return;
		  }
		  else
			  ((insertResult.first)->second)++;	//increase forwarded messages counter

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
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice())
				  , MilliSeconds(frwDelay));

		  Simulator::Schedule(MilliSeconds(frwDelay), &Ipv6Interface::Send, interface,
				  p, Ipv6Address::GetAllNodesMulticast());
		  //interface->Send (p, Ipv6Address::GetAllNodesMulticast());

		  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NS message forwarded with "
				  "HopLimit = " << (int)ip.GetHopLimit());
	  }
	  else
		  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NS message dropped! - not in MPR Selectors Set");
  }

  packet->RemoveHeader (nsHeader);

  Ipv6Address target = nsHeader.GetIpv6Target ();

  //MG: for mNS adjust src and dst: change dst to solicited node multicast, to process packet normally
  //also change src to unspecified

  if(nsHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE && dst.IsAllNodesMulticast())
  {
	  //trace:
	  messageType = NDPP_NS_MHOP;

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
    		  //trace:
			  m_rxNdTrace (packet, &nsHeader, messageType, NO_MPR_OPT, DISCARD_NS_INVALID_ADDR,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

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
  /*if (packet->GetUid () == ifaddr.GetNsDadUid () )
    {
      //don't process our own DAD probe
      NS_LOG_LOGIC ("Hey we receive our DAD probe!");
      return;
    }*/

  if(tentative_dad)
  {
	  //somebody else is doing DAD on my tentative address, I cannot use it
	  interface->SetState(ifaddr.GetAddress(), Ipv6InterfaceAddress::INVALID);
	  NS_LOG_INFO("Node " << nodeNo << ", Somebody else is doing DAD on my tentative address: " << ifaddr.GetAddress() <<
			  ". The address is INVALID");

	  //check if there is any address possible to be used, if not - try to assign new one based on MAC address
	  //WARNING! if the above MAC-based address is already there marked as INVALID - leave it
	  //this approach makes sense in my simulations, in general probbaly a random selection shoudl be made
	  if(m_autoAssignNewAddr)
		  AutoAssignNewAddr(interface);

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
	  //check here if we have sent the reply already
	  //check the Forwarding Set for duplications first:
	  FlowIdTag randomId;
	  SeqNoTag sequenceNo;
	  NS_ASSERT_MSG(packet->PeekPacketTag(randomId), "Peek Packet Tag for Random ID failed");
	  NS_ASSERT_MSG(packet->PeekPacketTag(sequenceNo), "Peek Packet Tag for Sequence No failed");

	  ForwardingTuple *frwTuple;
	  if(m_frwUseSrcAddr)
		  frwTuple = new ForwardingTuple(target, randomId.GetFlowId(), sequenceNo.GetFlowId(), src);
	  else
		  frwTuple = new ForwardingTuple(target, randomId.GetFlowId(), sequenceNo.GetFlowId());

	  /*FrwMapI it = m_DADReplySet.find(frwTuple);
	  if(it == m_DADReplySet.end())
	  {
		  //pair not found - insert new entry and reply to DAD query
		  m_DADReplySet.insert(std::make_pair(frwTuple, 1));
	  }*/

	  //insert returns pointer to the inserted or found value and bool indicating whetehr insertion actually took place
	  //this is a work around, because find is most likely not working for maps size bigger than 1
	  std::pair<FrwMapI, bool> insertResult = m_DADReplySet.insert(std::make_pair(*frwTuple, 1));
	  delete frwTuple;
	  if(insertResult.second == true)
	  {
		  //pair not found -  new entry already inserted, forward
	  }
	  else if((insertResult.first)->second >= m_DADReplyCountLimit)
	  {
		  //we have a duplication - do not reply any more!!!
		  NS_LOG_LOGIC("Node " << nodeNo << ": Duplicated multicast NS DAD query with target: "
				  << target << " - do not reply");
		  return;
	  }
	  else
		  ((insertResult.first)->second)++;	//increase DAD reply counter

	  p = Forge_mNA (target.IsLinkLocal () ? interface->GetLinkLocalAddress ().GetAddress () : ifaddr.GetAddress (),
			  Ipv6Address::GetAllNodesMulticast(), target, &hardwareAddress, flags );
	 //we should set packet Uid to some global value for a node (in this case node id, which is assumed unique) - otherwise NaDadUid changes with each mNA sent as a reply for the same target

	  //add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
	 //flowId tag is used since it's capabilities are good for this purpose
	  int rndIDOwn = this->m_node->GetId();
	  int seqNoOwn = GetSequenceNo();
	  FlowIdTag randomIdOwn(rndIDOwn);
	  SeqNoTag sequenceNoOwn(seqNoOwn);
	  p->AddPacketTag(randomIdOwn);
	  p->AddPacketTag(sequenceNoOwn);

	  //add outgoing packet to FrwSet
	  if(m_frwUseSrcAddr)
		  m_frwSet.insert(std::make_pair(ForwardingTuple(target, rndIDOwn, seqNoOwn, src), 1));
	  else
		  m_frwSet.insert(std::make_pair(ForwardingTuple(target, rndIDOwn, seqNoOwn), 1));

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

void Icmpv6L4Protocol_NdppFrw::HandleNA(Ptr<Packet> packet, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface)
{
	NS_LOG_FUNCTION (this << packet << src << dst << interface);
	Icmpv6NA naHeader;
	Icmpv6OptionLinkLayerAddress lla (1);

	//collecting tracing information:
	NdppMessageType_e messageType;
	MprOptType_e mprOptType = NO_MPR_OPT;

	uint32_t nodeNo = this->m_node->GetId();

	// Ptr<Ipv6Interface_Ndpp> interfaceNdpp = interface->GetDevice()->GetObject<Ipv6Interface_Ndpp> ();
	Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);

	//MG: forward a copy of multihop NA packets:
	packet->PeekHeader(naHeader);
	if(naHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE && dst.IsAllNodesMulticast() && ip.GetHopLimit() > 1)
	{
	  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NA message received");

	  //MPRs processing here
	  bool forward_ok = false;
	  //if lists are not initialized, or MPR processes not started - do not forward
	  SelectorsI it_sel;
	  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
			  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
	  {
		  if(src.IsEqual(it_sel->m_addr))
		  {

			  if(it_sel->m_val_t >= Simulator::Now() || src.IsAny())
				  forward_ok = true;
			  else
			  {
				  if(!it_sel->m_addr.IsAny())	//never remove "::", MG: changed from kernel impl.
				  {
				  //remove old selector
				  NS_LOG_LOGIC("Node " << nodeNo << ": old MPR selector removed: " << it_sel->m_addr);
				  interfaceNdpp->m_mprData.m_selectors.erase(it_sel);
				  }
			  }
			  break;
		  }
	  }

	  if(forward_ok)
	  {
		  //check the Forwarding Set for duplications first:
		  FlowIdTag randomId;
		  SeqNoTag sequenceNo;
		  NS_ASSERT_MSG(packet->PeekPacketTag(randomId), "Peek Packet Tag for Random ID failed");
		  NS_ASSERT_MSG(packet->PeekPacketTag(sequenceNo), "Peek Packet Tag for Sequence No failed");

		  ForwardingTuple *frwTuple;
		  if(m_frwUseSrcAddr)
			  frwTuple = new ForwardingTuple(naHeader.GetIpv6Target(), randomId.GetFlowId(),
						  sequenceNo.GetFlowId(), src);
		  else
			  frwTuple = new ForwardingTuple(naHeader.GetIpv6Target(), randomId.GetFlowId(),
						  sequenceNo.GetFlowId());

		 /* FrwMapI it = m_frwSet.find(frwTuple);
		  if(it == m_frwSet.end())
		  {
			  //pair not found - insert new entry and forward
			  m_frwSet.insert(std::make_pair(frwTuple, 1));
		  }*/
		  //insert returns pointer to the inserted or found value and bool indicating whetehr insertion actually took place
		  //this is a work around, because find is most likely not working for maps size bigger than 1
		  std::pair<FrwMapI, bool> insertResult = m_frwSet.insert(std::make_pair(*frwTuple, 1));
		  delete frwTuple;
		  if(insertResult.second == true)
		  {
			  //pair not found -  new entry already inserted, forward
		  }
		  else if((insertResult.first)->second >= m_frwCountLimit)
		  {
			  //we have a duplication - do not process and do not forward further!!!
			  forward_ok = false;
			  NS_LOG_LOGIC("Node " << nodeNo << ": Duplicated multicast NA message from MPR selector: "
					  << src << " - drop it");
			  //trace:
			   m_rxNdTrace (packet, &naHeader, NDPP_NA_MHOP, NO_MPR_OPT, DUPLICATED_MNA_DROPPED,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

			  return;
		  }
		  else
			  ((insertResult.first)->second)++;	//increase forwarded messages counter
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

		  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NA message forwarded with "
					"HopLimit = " << (int)ip.GetHopLimit());
	  }
	  else
		  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NA message dropped!");
	}

	//for tracing purposes only:
	messageType = (naHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE)? NDPP_NA_MHOP : NDPP_NA_1HOP;
	Ptr<Packet> p_copy = packet->Copy();	//copy packet for tracing

	packet->RemoveHeader (naHeader);
	Ipv6Address target = naHeader.GetIpv6Target ();

	Address hardwareAddress;
	NdiscCache::Entry* entry = 0;
	Ptr<NdiscCache> cache = FindCache (interface->GetDevice ());
	std::list<Ptr<Packet> > waiting;

	/* XXX search all options following the NA header */
	/* Get LLA */
	uint8_t type;
	Icmpv6OptionMprParams* mprp_opt = NULL;	//this cannot be used twice! - different
								   //object needed for each remove header!!!!
	MprOptsList mprp_opts;
	Icmpv6OptionMprAnnouncement mpra_opt;

	bool mpraFound = false;
	bool llaFound = false;

	OneHopSetI it;
	Ipv6Address my_main_addr;


	//read through all opts, then process (LLa is processed later)
	while(packet->GetSize() > 0)
	{
	  packet->CopyData (&type, sizeof(type));
	  switch (type)
	  {
	  case Icmpv6Header::ICMPV6_OPT_MPR_PARAMS:
		  if(mprp_opt)
			  delete mprp_opt;
		  mprp_opt = new Icmpv6OptionMprParams();
		  packet->RemoveHeader(*mprp_opt);
		  mprp_opts.push_back(*mprp_opt);
		  break;
	  case Icmpv6Header::ICMPV6_OPT_MPR_ANNOUNCEMENT:
		  packet->RemoveHeader(mpra_opt);
		  mpraFound = true;
		  break;
	  case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET:
		  packet->RemoveHeader (lla);
		  llaFound = true;
		  break;
	  default:
		  break;
	  }
	}


	//process options:

	if(!mprp_opts.empty() || mpraFound)
	{
	  //trace:

	  if(!mprp_opts.empty())
		  if(mpraFound)
			  mprOptType = BOTH;
		  else
			  mprOptType = MPR_PARAMS;
	  else
		  mprOptType = MPR_ANNOUNCEMENT;

	  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, MPR_INFO,
		  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

	  //initiate MPR processes if not started yet on this interface
	  interfaceNdpp->MprsInit();
	  //TODO - what if a node has only one GLOBAL address?
	  my_main_addr = interface->GetLinkLocalAddress().GetAddress();

	}

	if(!mprp_opts.empty())
	{
	  bool new_entry = true;

	  for(it = interfaceNdpp->m_mprData.m_neighbours.begin();
					  it != interfaceNdpp->m_mprData.m_neighbours.end(); it++)
	  {
		  if(it->m_addr.IsEqual(target))
		  {
			  //update entry (clean 2-hop set)
			  it->m_2hops.clear();
			  it->m_other = 0x00;
			  new_entry = false;
			  break;	//only one entry for a neighbour is allowed
		  }
	  }
	  //add new entry or update old one along with adding new 2hop neighbours:
	  if(new_entry)
	  {
		  interfaceNdpp->m_mprData.m_neighbours.push_front(OneHop(target));
		  it = interfaceNdpp->m_mprData.m_neighbours.begin();
		  //set times according to [RFC 3626]
		  it->m_sym_t = Simulator::Now() - Seconds(1.0); //expired
		  it->m_lost_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
		  it->m_neightype = NeighInfo::NOT_NEIGH;

		  NS_LOG_LOGIC("Node " << nodeNo << ": MPR Parameters - new neighbour added: " << it->m_addr <<" with sym_t="
				  << it->m_sym_t.GetSeconds() <<", lost_t=" << (double)it->m_lost_t.GetSeconds());
	  }
	  else
		 NS_LOG_LOGIC("Node " << nodeNo << " got NA from a known neighbor with: address " << it->m_addr <<
					  ", willingness=" << (int) it->m_willingness << ", asum_t=" <<it->m_asym_t.GetSeconds());

	  it->m_willingness = mprp_opts.begin()->m_willingness;
	  it->m_asym_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);


	  //add nodes from this and other opts to 2hopSet:

	  //go through all MPR Parameters opts received from this neighbour (contain 2-hop neighbourhood)
	  MprOptsI it_opt;
	  AddrListCI it_addr;
	  for(it_opt = mprp_opts.begin(); it_opt != mprp_opts.end(); it_opt++)
	  {
		  if(it_opt->m_linkCode == NeighInfo::LC_INVALID)
			  continue;		//discard this option

		  //go through all entries for one opt
		  for(it_addr = it_opt->m_NeighAddresses.begin(); it_addr != it_opt->m_NeighAddresses.end(); it_addr++)
		  {
			  u_int8_t link_type = it_opt->m_linkCode & 0x03;
			  if(!my_main_addr.IsEqual(*it_addr))	//ignore my MAIN address listed!
			  {
				  //check if there is n-DAD ongoing for the found address:
				  //this might be a case if my 2-hop neighbour holds the same address
				  //this neighbour actually has a right to do so, so I should include it in my lists!!!
				  //confusion might be in the intermediate node!
				  it->m_2hops.push_front(NeighInfo(*it_addr));
				  it->m_2hops.begin()->m_linktype = it_opt->m_linkCode & 0x03;
				  it->m_2hops.begin()->m_neightype = it_opt->m_linkCode & 0x0C;
				  if(it_opt->m_willingness < it->m_willingness)
				  {
					  it->m_willingness = it_opt->m_willingness;
					  NS_LOG_LOGIC("Node " << nodeNo << ": MPR Parameters options found with different "
							  "willingness for the same node, the smaller value is chosen");

				  }
				  if(it->m_2hops.begin()->m_neightype == NeighInfo::SYM_NEIGH)
					  NS_LOG_LOGIC("Node " << nodeNo << ": new symmetric 2-hop (" << it->m_2hops.begin()->m_addr <<") added for neighbor" << it->m_addr);
			  }
			  else
			  {
				  if(link_type == NeighInfo::LOST_LINK)
				  {
					  it->m_sym_t = Simulator::Now() - Seconds(1.0);	//expired -> I am not heard anymore
					  it->m_neightype = NeighInfo::NOT_NEIGH;
					  NS_LOG_LOGIC("Node " << nodeNo << ": neighbor with address" << it->m_addr << "advertises us as lost");
				  }
				  else if(link_type == NeighInfo::SYM_LINK || link_type == NeighInfo::ASYM_LINK)
				  {
					  if(it->m_sym_t < Simulator::Now())
						 NS_LOG_LOGIC("Node " << nodeNo << ": new symmetric neighbor:" << it->m_addr);
					  else
						 NS_LOG_LOGIC("Node " << nodeNo << ": symmetric neighbor:" << it->m_addr);

					  it->m_sym_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
					  it->m_lost_t = Simulator::Now() + Seconds(OneHop::LOST_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);	//sym_t+ neigh. hold time
					  it->m_neightype = NeighInfo::SYM_NEIGH;
				 }
			  }
		  }
	  }
	  it->m_lost_t = (it->m_asym_t > it->m_lost_t)? it->m_asym_t : it->m_lost_t;	//select bigger one
	}
	mprp_opts.clear();
	if(mprp_opt)
	   delete mprp_opt;

	if(mpraFound)
	{
	  AddrListCI it_addr;
	  SelectorsI it_sel;
	 //go through all addresses listed in this opt
	  for(it_addr = mpra_opt.m_MprAddresses.begin(); it_addr != mpra_opt.m_MprAddresses.end(); it_addr++)
	  {
		  //only main address can be announced, so compare with this one only
		  if(my_main_addr.IsEqual(*it_addr))
		  {
			  //add MPR Selector (if not present on the list yet):
			  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
								  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
			  {
				  if(target.IsEqual(it_sel->m_addr))
					  break;
			  }
			  if(it_sel == interfaceNdpp->m_mprData.m_selectors.end())	//new selector
			  {
				  interfaceNdpp->m_mprData.m_selectors.push_front(Selector(target));
				  it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
				 NS_LOG_LOGIC("Node " << nodeNo << ": new MPR Selector " << it_sel->m_addr);
			  }
			  else
				  NS_LOG_LOGIC("Node " << nodeNo << ": old MPR Selector updated" << it_sel->m_addr);

			  it_sel->m_val_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
			  break;	//even if more of mine addresses are listed, I just need to have one entry for my MPR Selector
		  }
	  }
	}
	/*
	if (lla.GetLength())
	{
	   packet->RemoveHeader (lla);
	}
	*/


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
				  m_rxNdTrace (p_copy, &naHeader, NDPP_NA_MHOP, mprOptType, OWN_DAD_NA_PROBE,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
				  return;
			  }
		  }

		  if (ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE || ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC)
		  {
			  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::INVALID);
			  NS_LOG_INFO("Node " << nodeNo << ": DAD detected duplicated address: " << ifaddr.GetAddress() << ". The address is INVALID");

			  //track ND++ latency:
			  m_latencyTrace(interface, ifaddr.GetAddress(), INVALID);


			  //check if there is any address possible to be used, if not - try to assign new one based on MAC address
			  //WARNING! if the above MAC-based address is already there marked as INVALID - leave it
			  //this approach makes sense in my simulations, in general probably a random selection should be made
			  if(m_autoAssignNewAddr)
				  AutoAssignNewAddr(interface);

			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, DAD_DUPLICATION,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
		  else if (ifaddr.GetState () == Ipv6InterfaceAddress::PREFERRED)
		  {
			  //somebody is using my address, which is valid. Probably there is a duplication in the network
			  NS_LOG_INFO("Node " << nodeNo << ": Duplicated IPv6 address: " << ifaddr.GetAddress());
			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, WARNING_DUPLICATION_POSSIBLE,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
		  else if (ifaddr.GetState () == Ipv6InterfaceAddress::INVALID)
		  {
			 //duplicated message to the address already marked invalid - register and discard (UWAGA! - rejestruje mNA
			  //i zwykle NA wysyłane przy MPRach!!!)
			  NS_LOG_LOGIC("Node " << nodeNo << ": duplicated message to the address already marked invalid "
					  << ifaddr.GetAddress());
			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, DISCARD_NA_INVALID_ADDR,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
	  }
	  /* we have not initiated any communication with the target so... discard the NA */
	  //trace: (without this else some nDAD packets were registered twice!)
	  else if(mprOptType == NO_MPR_OPT)	//otherwise this packet was already registered in trace
		  m_rxNdTrace (p_copy, &naHeader, messageType, NO_MPR_OPT, DISCARD_NA,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
	  return;
	}

	//MG: just for safety, maybe unnecessary;
	if(naHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE)
	  return;


	if (!llaFound)
	{
	return;
	}

	//trace:
	m_rxNdTrace (packet, &naHeader, messageType, NO_MPR_OPT, LLA_NEIGH_CACHE_UPDATE,
	  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

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

void Icmpv6L4Protocol_NdppFrw::AutoAssignNewAddr(Ptr<Ipv6Interface> interface)
{
	uint32_t nb = interface->GetNAddresses ();
	Ipv6InterfaceAddress ifaddr;
	bool found = false;

	for (uint32_t i = 0; i < nb; i++)
	{
		ifaddr = interface->GetAddress(i);

		if(ifaddr.GetState() != Ipv6InterfaceAddress::INVALID)
		{
		  found = true;
		  return;
		}
	}

	if(!found)
	{
		//assign link-local address based on MAC address
		 Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);
		 interfaceNdpp->DoSetupAutoAddress();
		//interface->SetNode(this->m_node);
		NS_LOG_INFO("New address auto-assigned to node " << m_node->GetId());
	}
}


////////////////////////////////////////////////////////////
////////NDPP_MPR version:
//////////////////////////////////////////////////////////////

TypeId Icmpv6L4Protocol_NdppMpr::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Icmpv6L4Protocol_NdppMpr")
    .SetParent<Icmpv6L4Protocol_NdppFrw> ()
    .AddConstructor<Icmpv6L4Protocol_NdppMpr> ()
  ;
  return tid;
}

Ptr<Packet> Icmpv6L4Protocol_NdppMpr::ForgeNA (Ipv6Address src, Ipv6Address dst, Ipv6Address target, Address* hardwareAddress, uint8_t flags,
		uint8_t mpr_opts, MprData* mpr_data)
{
  NS_LOG_FUNCTION (this << src << dst << hardwareAddress << (uint32_t)flags);
  Ptr<Packet> p = Create<Packet> ();
  Ipv6Header ipHeader;
  Icmpv6NA na;
  Icmpv6OptionLinkLayerAddress llOption (0, *hardwareAddress);  /* we give our mac address in response */

  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": Send NA ( from " << src << " to " << dst << ")");

  /* forge the entire NA packet from IPv6 header to ICMPv6 link-layer option, so that the packet does not pass by Icmpv6L4Protocol::Lookup again */

  //include mprp and mpra options:
  if(mpr_opts & MPR_ANNOUNCEMENT)
  {
	  NeighSetI it = mpr_data->m_mprs.begin();
	  Icmpv6OptionMprAnnWithRandomIds mpraOpt;

	  while (it != mpr_data->m_mprs.end())
	  {
		  mpraOpt.AddMPR(it->m_addr);
		  it++;
	  }

	  //add option: (even if it is empty)
	  p->AddHeader(mpraOpt);
  }


  if(mpr_opts & MPR_PARAMS)
  {
	  //add neighbours to appropriate options based on the link code field

	  Icmpv6OptionMprParamsWithRandomIds *mprPramsOpts[NeighInfo::LC_ALL_ENTRIES] = {NULL};	//array of pointers to MPRParams options for each link code
	  //options to send
	  u_int8_t link_code = 0x00;

	  OneHopSetI it = mpr_data->m_neighbours.begin();

	  if(!mpr_data->m_neighbours.empty())
	  {

		  while(it != mpr_data->m_neighbours.end())	//begin equals end for empty list
		  {
			  if(it->m_lost_t < Simulator::Now())
			  {
				  //remove neighbor entry - the link is lost permanently
				  NS_LOG_LOGIC ("Node " << this->m_node->GetId() << ": neighbor removed (lost): " << it->m_addr);
				  it->m_2hops.clear();
				  it = mpr_data->m_neighbours.erase(it);
				  continue;
			  }
			  else if(it->m_sym_t >= Simulator::Now())	//not expired
			  {
				  it->m_linktype = NeighInfo::SYM_LINK;
				  it->m_neightype = NeighInfo::SYM_NEIGH;
			  }
			  else if(it->m_asym_t >= Simulator::Now())	//sym. time expired, asym. time not expired
			  {
				  it->m_linktype = NeighInfo::ASYM_LINK;
				  it->m_neightype = NeighInfo::NOT_NEIGH;
			  }
			  else	//both sym. and asym. times expired, lost time not expire
			  {
				  it->m_linktype = NeighInfo::LOST_LINK;
				  it->m_neightype = NeighInfo::NOT_NEIGH;
			  }

			  link_code = it->m_linktype | it->m_neightype;
			  switch(link_code)
			  {
			  //ignore mpr_neigh. entries - they shouldn't be in this set
			  case 0x00:	//not neigh, unspec. link
			  case 0x01:	//not neigh, asymmetric link
			  case 0x03:	//not neigh, lost link
			  case 0x04:	//symm. neigh, unspec. link
			  case 0x05:	//symm. neigh, asymm. link
			  case 0x06:	//symm. neigh, symm. link
			  case 0x07:	//symm. neigh, lost link
			  //case 0x08:
			  //case 0x09:
			  //case 0x0a:
			  //case 0x0b:
				  if(mprPramsOpts[link_code] == NULL)
				  {
					  mprPramsOpts[link_code] = new Icmpv6OptionMprParamsWithRandomIds (m_willingness, link_code);
					 // NS_LOG_LOGIC("New MPR Params Opt added for link code: " << link_code);
				  }
				  mprPramsOpts[link_code]->AddNeighbor(it->m_addr);
				  break;
			  default:
				  break;	//silently ignore
			  }
			  it++;
		  }
	  }
	  else
	  {
		  //send mpr params with no neighbors listed. It can be stored in cell LC_INVALID, as it will be empty anyway
		  mprPramsOpts[NeighInfo::LC_INVALID] = new Icmpv6OptionMprParamsWithRandomIds (m_willingness);	//NOT_NEIGH, UNSPEC_LINK;
	  }

	  //add options:
	  for(int i=0; i<NeighInfo::LC_ALL_ENTRIES; i++)
	  {
		  if(mprPramsOpts[i] != NULL)
		  {
		//	  NS_LOG_LOGIC("Adding header for the MPR Params Opt with link code = " << i);
			  p->AddHeader(*mprPramsOpts[i]);
			  delete mprPramsOpts[i];
		  }
	  }
  }


  p->AddHeader (llOption);

  //changed below according to RFC 4861, chapter 7.2.4. (assumption - ForgeNA is to send solicited NAs only!)
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
  ipHeader.SetHopLimit (255);

  p->AddHeader (ipHeader);

  return p;
}


void Icmpv6L4Protocol_NdppMpr::HandleNA(Ptr<Packet> packet, Ipv6Header ip, Ipv6Address const &src, Ipv6Address const &dst, Ptr<Ipv6Interface> interface)
{
	NS_LOG_FUNCTION (this << packet << src << dst << interface);
	Icmpv6NA naHeader;
	Icmpv6OptionLinkLayerAddress lla (1);

	//collecting tracing information:
	NdppMessageType_e messageType;
	MprOptType_e mprOptType = NO_MPR_OPT;

	uint32_t nodeNo = this->m_node->GetId();

	FlowIdTag randomIdTag;
	bool randIDfound = packet->PeekPacketTag(randomIdTag);
	uint32_t randomId = (randIDfound)? randomIdTag.GetFlowId() : -1;

	// Ptr<Ipv6Interface_Ndpp> interfaceNdpp = interface->GetDevice()->GetObject<Ipv6Interface_Ndpp> ();
	Ptr<Ipv6Interface_Ndpp> interfaceNdpp = DynamicCast<Ipv6Interface_Ndpp>(interface);

	//MG: forward a copy of multihop NA packets:
	packet->PeekHeader(naHeader);
	if(naHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE && dst.IsAllNodesMulticast() && ip.GetHopLimit() > 1)
	{
	  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NA message received");

	  //MPRs processing here
	  bool forward_ok = false;
	  //if lists are not initialized, or MPR processes not started - do not forward
	  SelectorsI it_sel;
	  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
			  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
	  {
		  //UWAGA! it is not possible to compare with random id here, since it is not changed during FRW (it is always set to originator's node id!!!)
		  if(src.IsEqual(it_sel->m_addr))
		  {

			  if(it_sel->m_val_t >= Simulator::Now() || src.IsAny())
				  forward_ok = true;
			  else
			  {
				  if(!it_sel->m_addr.IsAny())	//never remove "::", MG: changed from kernel impl.
				  {
				  //remove old selector
				  NS_LOG_LOGIC("Node " << nodeNo << ": old MPR selector removed: " << it_sel->m_addr);
				  interfaceNdpp->m_mprData.m_selectors.erase(it_sel);
				  }
			  }
			  break;
		  }
	  }

	  if(forward_ok)
	  {
		  //check the Forwarding Set for duplications first:
		//  FlowIdTag randomId;
		  SeqNoTag sequenceNo;
		 // NS_ASSERT_MSG(packet->PeekPacketTag(randomId), "Peek Packet Tag for Random ID failed");
		  NS_ASSERT_MSG(packet->PeekPacketTag(sequenceNo), "Peek Packet Tag for Sequence No failed");

		  ForwardingTuple *frwTuple;
		  if(m_frwUseSrcAddr)
			  frwTuple = new ForwardingTuple(naHeader.GetIpv6Target(), randomId,
						  sequenceNo.GetFlowId(), src);
		  else
			  frwTuple = new ForwardingTuple(naHeader.GetIpv6Target(), randomId,
						  sequenceNo.GetFlowId());

		 /* FrwMapI it = m_frwSet.find(frwTuple);
		  if(it == m_frwSet.end())
		  {
			  //pair not found - insert new entry and forward
			  m_frwSet.insert(std::make_pair(frwTuple, 1));
		  }*/
		  //insert returns pointer to the inserted or found value and bool indicating whetehr insertion actually took place
		  //this is a work around, because find is most likely not working for maps size bigger than 1
		  std::pair<FrwMapI, bool> insertResult = m_frwSet.insert(std::make_pair(*frwTuple, 1));
		  delete frwTuple;
		  if(insertResult.second == true)
		  {
			  //pair not found -  new entry already inserted, forward
		  }
		  else if((insertResult.first)->second >= m_frwCountLimit)
		  {
			  //we have a duplication - do not process and do not forward further!!!
			  forward_ok = false;
			  NS_LOG_LOGIC("Node " << nodeNo << ": Duplicated multicast NA message from MPR selector: "
					  << src << " - drop it");
			  //trace:
			   m_rxNdTrace (packet, &naHeader, NDPP_NA_MHOP, NO_MPR_OPT, DUPLICATED_MNA_DROPPED,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

			  return;
		  }
		  else
			  ((insertResult.first)->second)++;	//increase forwarded messages counter
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

		  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NA message forwarded with "
					"HopLimit = " << (int)ip.GetHopLimit());
	  }
	  else
		  NS_LOG_LOGIC("Node " << nodeNo << ": Multicast NA message dropped!");
	}

	//for tracing purposes only:
	messageType = (naHeader.GetCode() == Icmpv6L4Protocol_NdSimple::NDAD_CODE)? NDPP_NA_MHOP : NDPP_NA_1HOP;
	Ptr<Packet> p_copy = packet->Copy();	//copy packet for tracing

	packet->RemoveHeader (naHeader);
	Ipv6Address target = naHeader.GetIpv6Target ();
	Ipv6AddrRandomIDPair targetWithRandId(target, randomId);

	Address hardwareAddress;
	NdiscCache::Entry* entry = 0;
	Ptr<NdiscCache> cache = FindCache (interface->GetDevice ());
	std::list<Ptr<Packet> > waiting;

	/* XXX search all options following the NA header */
	/* Get LLA */
	uint8_t type;
	Icmpv6OptionMprParamsWithRandomIds* mprp_opt = NULL;	//this cannot be used twice! - different
								   //object needed for each remove header!!!!
	MprOptsListWithIds mprp_opts;
	Icmpv6OptionMprAnnWithRandomIds mpra_opt;

	bool mpraFound = false;
	bool llaFound = false;

	OneHopSetI it;
	Ipv6Address my_main_addr;
	Ipv6AddrRandomIDPair my_main_addr_withRandID;


	//read through all opts, then process (LLa is processed later)
	while(packet->GetSize() > 0)
	{
	  packet->CopyData (&type, sizeof(type));
	  switch (type)
	  {
	  case Icmpv6Header::ICMPV6_OPT_MPR_PARAMS:
		  if(mprp_opt)
			  delete mprp_opt;
		  mprp_opt = new Icmpv6OptionMprParamsWithRandomIds();
		  packet->RemoveHeader(*mprp_opt);
		  mprp_opts.push_back(*mprp_opt);
		  break;
	  case Icmpv6Header::ICMPV6_OPT_MPR_ANNOUNCEMENT:
		  packet->RemoveHeader(mpra_opt);
		  mpraFound = true;
		  break;
	  case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET:
		  packet->RemoveHeader (lla);
		  llaFound = true;
		  break;
	  default:
		  break;
	  }
	}


	//process options:

	if(!mprp_opts.empty() || mpraFound)
	{
	  //trace:

	  if(!mprp_opts.empty())
		  if(mpraFound)
			  mprOptType = BOTH;
		  else
			  mprOptType = MPR_PARAMS;
	  else
		  mprOptType = MPR_ANNOUNCEMENT;

	  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, MPR_INFO,
		  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

	  //initiate MPR processes if not started yet on this interface
	  interfaceNdpp->MprsInit();
	  //TODO - what if a node has only one GLOBAL address? -> for now I assume the node MUST have a link-local address
	  my_main_addr = interface->GetLinkLocalAddress().GetAddress();
	  my_main_addr_withRandID = Ipv6AddrRandomIDPair(my_main_addr, nodeNo);

	}

	if(!mprp_opts.empty())
	{
	  bool new_entry = true;

	  for(it = interfaceNdpp->m_mprData.m_neighbours.begin();
					  it != interfaceNdpp->m_mprData.m_neighbours.end(); it++)
	  {
		  if(it->m_addr == targetWithRandId)
		  {
			  //update entry (clean 2-hop set)
			  it->m_2hops.clear();
			  it->m_other = 0x00;
			  new_entry = false;
			  break;	//only one entry for a neighbour is allowed
		  }
	  }
	  //add new entry or update old one along with adding new 2hop neighbours:
	  if(new_entry)
	  {
		  interfaceNdpp->m_mprData.m_neighbours.push_front(OneHop(targetWithRandId));
		  it = interfaceNdpp->m_mprData.m_neighbours.begin();
		  //set times according to [RFC 3626]
		  it->m_sym_t = Simulator::Now() - Seconds(1.0); //expired
		  it->m_lost_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
		  it->m_neightype = NeighInfo::NOT_NEIGH;

		  NS_LOG_LOGIC("Node " << nodeNo << ": MPR Parameters - new neighbour added: " << it->m_addr << " with randomID=" << randomId <<
				  ", sym_t=" << it->m_sym_t.GetSeconds() <<", lost_t=" << (double)it->m_lost_t.GetSeconds());
	  }
	  else
		 NS_LOG_LOGIC("Node " << nodeNo << " got NA from a known neighbor with: address " << it->m_addr << ", randomID=" << randomId <<
					  ", willingness=" << (int) it->m_willingness << ", asum_t=" <<it->m_asym_t.GetSeconds());

	  it->m_willingness = mprp_opts.begin()->m_willingness;
	  it->m_asym_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);


	  //add nodes from this and other opts to 2hopSet:

	  //go through all MPR Parameters opts received from this neighbour (contain 2-hop neighbourhood)
	  MprOptsWithIdsI it_opt;
	  AddrListWithIdsCI it_addr;
	  for(it_opt = mprp_opts.begin(); it_opt != mprp_opts.end(); it_opt++)
	  {
		  if(it_opt->m_linkCode == NeighInfo::LC_INVALID)
			  continue;		//discard this option

		  //go through all entries for one opt
		  for(it_addr = it_opt->m_NeighAddresses.begin(); it_addr != it_opt->m_NeighAddresses.end(); it_addr++)
		  {
			  u_int8_t link_type = it_opt->m_linkCode & 0x03;
			  if(my_main_addr_withRandID != *it_addr)	//ignore my MAIN address listed!
			  {
				  //check if there is n-DAD ongoing for the found address:
				  //this might be a case if my 2-hop neighbour holds the same address
				  //this neighbour actually has a right to do so, so I should include it in my lists!!!
				  //confusion might be in the intermediate node!
				  it->m_2hops.push_front(NeighInfo(*it_addr));
				  it->m_2hops.begin()->m_linktype = it_opt->m_linkCode & 0x03;
				  it->m_2hops.begin()->m_neightype = it_opt->m_linkCode & 0x0C;
				  if(it_opt->m_willingness < it->m_willingness)
				  {
					  it->m_willingness = it_opt->m_willingness;
					  NS_LOG_LOGIC("Node " << nodeNo << ": MPR Parameters options found with different "
							  "willingness for the same node, the smaller value is chosen");

				  }
				  if(it->m_2hops.begin()->m_neightype == NeighInfo::SYM_NEIGH)
					  NS_LOG_LOGIC("Node " << nodeNo << ": new symmetric 2-hop (" << it->m_2hops.begin()->m_addr <<") added for neighbor" << it->m_addr);
			  }
			  else
			  {
				  if(link_type == NeighInfo::LOST_LINK)
				  {
					  it->m_sym_t = Simulator::Now() - Seconds(1.0);	//expired -> I am not heard anymore
					  it->m_neightype = NeighInfo::NOT_NEIGH;
					  NS_LOG_LOGIC("Node " << nodeNo << ": neighbor with address" << it->m_addr << "advertises us as lost");
				  }
				  else if(link_type == NeighInfo::SYM_LINK || link_type == NeighInfo::ASYM_LINK)
				  {
					  if(it->m_sym_t < Simulator::Now())
						 NS_LOG_LOGIC("Node " << nodeNo << ": new symmetric neighbor:" << it->m_addr);
					  else
						 NS_LOG_LOGIC("Node " << nodeNo << ": symmetric neighbor:" << it->m_addr);

					  it->m_sym_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
					  it->m_lost_t = Simulator::Now() + Seconds(OneHop::LOST_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);	//sym_t+ neigh. hold time
					  it->m_neightype = NeighInfo::SYM_NEIGH;
				 }
			  }
		  }
	  }
	  it->m_lost_t = (it->m_asym_t > it->m_lost_t)? it->m_asym_t : it->m_lost_t;	//select bigger one
	}
	mprp_opts.clear();
	if(mprp_opt)
	   delete mprp_opt;

	if(mpraFound)
	{
	  AddrListWithIdsCI it_addr;
	  SelectorsI it_sel;
	 //go through all addresses listed in this opt
	  for(it_addr = mpra_opt.m_MprAddresses.begin(); it_addr != mpra_opt.m_MprAddresses.end(); it_addr++)
	  {
		  //only main address can be announced, so compare with this one only
		  if(my_main_addr_withRandID == *it_addr)
		  {
			  //add MPR Selector (if not present on the list yet):
			  //comparison with target and not remembering the random ids has purpose:
			  //FRW is base on address ONLY - using random ids here could result in 2 entries for the same address and problems!!!
			  for(it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
					  it_sel != interfaceNdpp->m_mprData.m_selectors.end(); it_sel++)
			  {
				  if(target == it_sel->m_addr)
					  break;
			  }
			  if(it_sel == interfaceNdpp->m_mprData.m_selectors.end())	//new selector
			  {
				  interfaceNdpp->m_mprData.m_selectors.push_front(Selector(target));
				  it_sel = interfaceNdpp->m_mprData.m_selectors.begin();
				 NS_LOG_LOGIC("Node " << nodeNo << ": new MPR Selector - " << it_sel->m_addr << " (randomID=" << randomId << ")");
			  }
			  else
				  NS_LOG_LOGIC("Node " << nodeNo << ": old MPR Selector updated - " << it_sel->m_addr << " (randomID=" << randomId << ")");

			  it_sel->m_val_t = Simulator::Now() + Seconds(OneHop::VAL_T_INTERVAL * interfaceNdpp->m_MPRP_DELAY);
			  break;	//even if more of mine addresses are listed, I just need to have one entry for my MPR Selector
		  }
	  }
	}
	/*
	if (lla.GetLength())
	{
	   packet->RemoveHeader (lla);
	}
	*/


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
			 // FlowIdTag randomId;
			  if(/*packet->PeekPacketTag(randomId) && randomId.GetFlowId()*/ randomId == nodeNo)
			  {
				  /* don't process our own DAD NA answer */
				  NS_LOG_LOGIC ("Hey we receive our DAD mNA!");
				  //trace:
				  m_rxNdTrace (p_copy, &naHeader, NDPP_NA_MHOP, mprOptType, OWN_DAD_NA_PROBE,
					  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
				  return;
			  }
		  }

		  if (ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE || ifaddr.GetState () == Ipv6InterfaceAddress::TENTATIVE_OPTIMISTIC)
		  {
			  interface->SetState (ifaddr.GetAddress (), Ipv6InterfaceAddress::INVALID);
			  NS_LOG_INFO("Node " << nodeNo << ": DAD detected duplicated address: " << ifaddr.GetAddress() << ". The address is INVALID");

			  //track ND++ latency:
			  m_latencyTrace(interface, ifaddr.GetAddress(), INVALID);


			  //check if there is any address possible to be used, if not - try to assign new one based on MAC address
			  //WARNING! if the above MAC-based address is already there marked as INVALID - leave it
			  //this approach makes sense in my simulations, in general probably a random selection should be made
			  if(m_autoAssignNewAddr)
				  AutoAssignNewAddr(interface);

			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, DAD_DUPLICATION,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
		  else if (ifaddr.GetState () == Ipv6InterfaceAddress::PREFERRED)
		  {
			  //somebody is using my address, which is valid. Probably there is a duplication in the network
			  NS_LOG_INFO("Node " << nodeNo << ": Duplicated IPv6 address: " << ifaddr.GetAddress());
			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, WARNING_DUPLICATION_POSSIBLE,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
		  else if (ifaddr.GetState () == Ipv6InterfaceAddress::INVALID)
		  {
			 //duplicated message to the address already marked invalid - register and discard (UWAGA! - rejestruje mNA
			  //i zwykle NA wysyłane przy MPRach!!!)
			  NS_LOG_LOGIC("Node " << nodeNo << ": duplicated message to the address already marked invalid "
					  << ifaddr.GetAddress());
			  //trace:
			  m_rxNdTrace (p_copy, &naHeader, messageType, mprOptType, DISCARD_NA_INVALID_ADDR,
				  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
		  }
	  }
	  /* we have not initiated any communication with the target so... discard the NA */
	  //trace: (without this else some nDAD packets were registered twice!)
	  else if(mprOptType == NO_MPR_OPT)	//otherwise this packet was already registered in trace
		  m_rxNdTrace (p_copy, &naHeader, messageType, NO_MPR_OPT, DISCARD_NA,
			  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));
	  return;
	}

	//MG: just for safety, maybe unnecessary;
	if(naHeader.GetCode() == Icmpv6L4Protocol_Ndpp::NDAD_CODE)
	  return;


	if (!llaFound)
	{
	return;
	}

	//trace:
	m_rxNdTrace (packet, &naHeader, messageType, NO_MPR_OPT, LLA_NEIGH_CACHE_UPDATE,
	  (m_node->GetObject<Ipv6L3Protocol>())->GetInterfaceForDevice(interface->GetDevice()));

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
} /* namespace ns3 */

