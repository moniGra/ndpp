/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/flow-id-tag.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"

#include "ipv6-interface.h"
#include "ns3/net-device.h"
#include "loopback-net-device.h"
#include "ipv6-l3-protocol.h"
#include "icmpv6-l4-protocol.h"
#include "ndisc-cache.h"

#include "ns3/double.h"
#include "ns3/mac16-address.h"
#include "ns3/mac64-address.h"
#include "mpr-lists.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("Ipv6Interface_Ndpp");

NS_OBJECT_ENSURE_REGISTERED (Ipv6Interface_Ndpp);

//const double Ipv6Interface_Ndpp::m_MPRP_DELAY = 1.0;



TypeId Ipv6Interface_Ndpp::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6Interface_Ndpp")
    .SetParent<Ipv6Interface> ()
    .AddConstructor<Ipv6Interface_Ndpp>()
    .AddAttribute ("MPRP_DELAY", "Delay between consecutive MPR opts announcement (in sec.)",
    		DoubleValue (1.0),
			MakeDoubleAccessor (&Ipv6Interface_Ndpp::m_MPRP_DELAY),
			MakeDoubleChecker<double> ())
  ;
  return tid;
}

Ipv6Interface_Ndpp::Ipv6Interface_Ndpp()
	: m_mprs_init(false)
{
  NS_LOG_FUNCTION (this);
}

Ipv6Interface_Ndpp::Ipv6Interface_Ndpp (const Ipv6Interface_Ndpp &o)
	: m_mprData(o.m_mprData),
	  m_mprs_init(o.m_mprs_init)
{

}

Ipv6Interface_Ndpp::~Ipv6Interface_Ndpp()
{
	m_mprData.m_mprs.clear();
	m_mprData.m_selectors.clear();
	for(OneHopSetI it = m_mprData.m_neighbours.begin(); it != m_mprData.m_neighbours.end(); it++)
		it->m_2hops.clear();
	m_mprData.m_neighbours.clear();

	m_nDADregistry.clear();
  NS_LOG_FUNCTION_NOARGS ();
}


bool Ipv6Interface_Ndpp::AddAddress (Ipv6InterfaceAddress iface)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ipv6Address addr = iface.GetAddress ();

  /* DAD handling */
  if (!addr.IsAny ())
    {
      for (Ipv6InterfaceAddressListCI it = m_addresses.begin (); it != m_addresses.end (); ++it)
        {
          if (it->GetAddress () == addr)
            {
              return false;
            }
        }

      m_addresses.push_back (iface);

      if (!addr.IsAny () || !addr.IsLocalhost ())
      {
    	  /*start MPR processes on this interface*/
    	  //TODO: clean lists here if already initialized?
    	  MprsInit();


    	  /* DAD handling */
    	  Ptr<Icmpv6L4Protocol> icmpv6 = m_node->GetObject<Ipv6L3Protocol> ()->GetIcmpv6 ();

    	  if (icmpv6 && icmpv6->IsAlwaysDad ())
    	  {
    		  //set address as tentative if not done yet!
    		  if(iface.GetState() != Ipv6InterfaceAddress::TENTATIVE)
    			  iface.SetState(Ipv6InterfaceAddress::TENTATIVE);

    		  //TODO: consider adding delay here - between 0 and MAX_RTR_SOLICITATION_DELAY as specified in RFC4862!!!
    		  //delay is added - on testbed it was there too!
    		  //CHANGE!!! - allow to set higher delay, but use it only during first DAD query after interface up!!!!

    		  //track ND++ latency:
    		  DynamicCast<Icmpv6L4Protocol_Ndpp>(icmpv6)->m_latencyTrace(this, addr, Icmpv6L4Protocol_Ndpp::INIT);

    		  Time delay = Seconds(icmpv6->GetDadDelay());
    		  Simulator::Schedule (delay, &Icmpv6L4Protocol::DoDAD, icmpv6, addr, this);
    		  Simulator::Schedule (delay + MilliSeconds (Icmpv6L4Protocol::RETRANS_TIMER), &Icmpv6L4Protocol::FunctionDadTimeout, icmpv6, this, addr);
    	  }
      }
      return true;
    }

  /* bad address */
  return false;
}

bool Ipv6Interface_Ndpp::AddAddressNoDelay (Ipv6InterfaceAddress iface)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ipv6Address addr = iface.GetAddress ();

  /* DAD handling */
  if (!addr.IsAny ())
    {
      for (Ipv6InterfaceAddressListCI it = m_addresses.begin (); it != m_addresses.end (); ++it)
        {
          if (it->GetAddress () == addr)
            {
              return false;
            }
        }

      m_addresses.push_back (iface);

      if (!addr.IsAny () || !addr.IsLocalhost ())
      {
    	  /*start MPR processes on this interface*/
    	  //TODO: clean lists here if already initialized?
    	  MprsInit();


    	  /* DAD handling */
    	  Ptr<Icmpv6L4Protocol> icmpv6 = m_node->GetObject<Ipv6L3Protocol> ()->GetIcmpv6 ();

    	  if (icmpv6 && icmpv6->IsAlwaysDad ())
    	  {
    		  //set address as tentative if not done yet!
    		  if(iface.GetState() != Ipv6InterfaceAddress::TENTATIVE)
    			  iface.SetState(Ipv6InterfaceAddress::TENTATIVE);

    		  //TODO: consider adding delay here - between 0 and MAX_RTR_SOLICITATION_DELAY as specified in RFC4862!!!
    		  //delay is added - on testbed it was there too!
    		  //CHANGE!!! - no delay - only after auto-assigned new address
    		  Time delay = Seconds(0.0);
    		  Simulator::Schedule (delay, &Icmpv6L4Protocol::DoDAD, icmpv6, addr, this);
    		  Simulator::Schedule (delay + MilliSeconds (Icmpv6L4Protocol::RETRANS_TIMER), &Icmpv6L4Protocol::FunctionDadTimeout, icmpv6, this, addr);
    	  }
      }
      return true;
    }

  /* bad address */
  return false;
}

void Ipv6Interface_Ndpp::SetDown ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_ifup = false;
 // m_addresses.clear (); -> do not clear addresses - good for experiment set-up

  //clear MPr lists:
  m_mprData.m_mprs.clear();
  m_mprData.m_selectors.clear();

  for(OneHopSetI it = m_mprData.m_neighbours.begin(); it != m_mprData.m_neighbours.end(); it++)
	  it->m_2hops.clear();
  m_mprData.m_neighbours.clear();

  //restart MPR processes so that start sending times are set again to the new values!!!!!
  //old cycled MPR processes are finished after detectting interface down in MprTimer
  //UWAGA! ta funkcja przerywa MPRy tylko, jelsi jest wywyolana w trakcie symulacji (scheduled)!!!
  m_mprs_init = false;
  //they are still "running", but if init is false - the new ones are scheduled too!!!! - this is not suppose to happen

  //clear nDAD count registry
  m_nDADregistry.clear();

  //clear ICMPv6 sets, if present
  Ptr<Icmpv6L4Protocol_NdppFrw> icmpv6 =  (m_node->GetObject<Ipv6L3Protocol>())->GetObject<Icmpv6L4Protocol_NdppFrw> ();
  if(icmpv6)
  {
	  icmpv6->ClearFrwSet();
	  icmpv6->ClearDADReplySet();
  }
}

void Ipv6Interface_Ndpp::DoSetupAutoAddress ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_node == 0 || m_device == 0)
    {
      return;
    }

  /* set up link-local address */
  if (!DynamicCast<LoopbackNetDevice> (m_device)) /* no autoconf for ip6-localhost */
    {
      Address addr = GetDevice ()->GetAddress ();

      if (Mac64Address::IsMatchingType (addr))
        {
          Ipv6InterfaceAddress ifaddr = Ipv6InterfaceAddress (Ipv6Address::MakeAutoconfiguredLinkLocalAddress (Mac64Address::ConvertFrom (addr)), Ipv6Prefix (64));
          AddAddressNoDelay (ifaddr);
        }
      else if (Mac48Address::IsMatchingType (addr))
        {
          Ipv6InterfaceAddress ifaddr = Ipv6InterfaceAddress (Ipv6Address::MakeAutoconfiguredLinkLocalAddress (Mac48Address::ConvertFrom (addr)), Ipv6Prefix (64));
          AddAddressNoDelay (ifaddr);
        }
      else if (Mac16Address::IsMatchingType (addr))
        {
          Ipv6InterfaceAddress ifaddr = Ipv6InterfaceAddress (Ipv6Address::MakeAutoconfiguredLinkLocalAddress (Mac16Address::ConvertFrom (addr)), Ipv6Prefix (64));
          AddAddressNoDelay (ifaddr);
        }
      else
        {
          NS_ASSERT_MSG (false, "IPv6 autoconf for this kind of address not implemented.");
        }
    }
  else
    {
      return; /* no NDISC cache for ip6-localhost */
    }

  Ptr<Icmpv6L4Protocol> icmpv6 = m_node->GetObject<Ipv6L3Protocol> ()->GetIcmpv6 ();
  if (m_device->NeedsArp ())
    {
      m_ndCache = icmpv6->CreateCache (m_device, this);
    }
}

void Ipv6Interface_Ndpp::MprsInit()
{
	NS_LOG_FUNCTION_NOARGS();
	if(m_mprs_init)
		return;

	m_mprs_init = true;

	//add unspecified address (::) to the list:

	Selector selector;	//address set to "::" by default
	m_mprData.m_selectors.push_front(selector);

	//send own MPR parameters:
	NS_LOG_LOGIC ("Attempt to send first MPR Parameters option");

	//send NA
	Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
	 //Ptr<Icmpv6L4Protocol> icmpv6 = ipv6->GetIcmpv6();
	 Ptr<Icmpv6L4Protocol_Ndpp> icmpv6 = ipv6->GetObject<Icmpv6L4Protocol_Ndpp> ();
	 Ptr<Packet> p;
	 Ipv6Address src = this->GetLinkLocalAddress().GetAddress();

	 //randomizing delay to avoid synchronization between nodes
	//int delay = (arc4random() % (mprp_delay * hz))+ ndad_delay*hz;
	double delay = icmpv6->GetMprStartDelay();	//uniform instead of exponential here -
	//exp. would make collisions more probable, since delay value close to or smaller than mean would
	//be most frequent
	//double delay = m_node->GetId() * MPRP_DELAY/30.0;

	 if(src != Ipv6Address::GetAny())
	 {
		 Address hardwareAddress = this->GetDevice ()->GetAddress ();
		 p = icmpv6->ForgeNA(src, Ipv6Address::GetAllNodesMulticast(), src, &hardwareAddress,
				 IsForwarding()? 4 : 0, Icmpv6L4Protocol_Ndpp::MPR_PARAMS, &m_mprData);

		 //add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
		 FlowIdTag randomId (this->m_node->GetId());
		 p->AddPacketTag(randomId);

		 NS_LOG_LOGIC ("Sending first MPR Parameters option from " << src);

		 //trace:
		 icmpv6->m_txNdTrace (p, Icmpv6L4Protocol_Ndpp::NDPP_NA_1HOP,
				 Icmpv6L4Protocol_Ndpp::MPR_PARAMS, Icmpv6L4Protocol_Ndpp::MPR_PROCESSES,
				 ipv6->GetInterfaceForDevice(this->GetDevice()), Seconds(delay));


		 Simulator::Schedule (Seconds (delay), &Ipv6Interface::Send, this, p, Ipv6Address::GetAllNodesMulticast());
		// Simulator::Schedule (Seconds (0.0), &Ipv6Interface::Send, this, p, Ipv6Address::GetAllNodesMulticast());
	 }

	 //"set timer" - schedule next event
	 Simulator::Schedule (Seconds (m_MPRP_DELAY + delay), &Ipv6Interface_Ndpp::MprTimeout, this);
}

void Ipv6Interface_Ndpp::MprTimeout()
{
	NS_LOG_FUNCTION_NOARGS();

	//stop ccyled MPR send on interface down!!!
	if(!IsUp())
	{
		m_mprs_init = false;	//do it for safety - it may happen that MPRs will be started on
								//interface down with addAddress and will be stopped here without forcing
								//init to be false before that
		return;
	}

	//MPR selection
	//TODO:check for updates to reduce computation effort
	MprLookUp();

	Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
	Ptr<Icmpv6L4Protocol> icmpv6 = ipv6->GetIcmpv6();
	Ptr<Packet> p;
	Ipv6Address src = this->GetLinkLocalAddress().GetAddress();
	Address hardwareAddress = this->GetDevice ()->GetAddress ();

	Icmpv6L4Protocol_Ndpp::MprOptType_e mprOptType = Icmpv6L4Protocol_Ndpp::NO_MPR_OPT;

	if(src != Ipv6Address::GetAny())
	{
		if(!m_mprData.m_mprs.empty())
		{
			//send MPR Announcement too
			NS_LOG_LOGIC ("Sending MPR Parameters and MPR Announcement option from " << src);
			mprOptType = Icmpv6L4Protocol_Ndpp::BOTH;
			p = icmpv6->ForgeNA(src, Ipv6Address::GetAllNodesMulticast(), src, &hardwareAddress,
				 IsForwarding()? 4 : 0, Icmpv6L4Protocol_Ndpp::MPR_PARAMS + Icmpv6L4Protocol_Ndpp::MPR_ANNOUNCEMENT,
					&m_mprData);
		}
		else
		{
			NS_LOG_LOGIC ("Sending MPR Parameters option from " << src);
			mprOptType = Icmpv6L4Protocol_Ndpp::MPR_PARAMS;
			p = icmpv6->ForgeNA(src, Ipv6Address::GetAllNodesMulticast(), src, &hardwareAddress,
				 IsForwarding()? 4 : 0, Icmpv6L4Protocol_Ndpp::MPR_PARAMS, &m_mprData);
		}
		//add tag to indicate Random Node ID (functionality similar to RandomID option in hbh)
		 FlowIdTag randomId(this->m_node->GetId());
		 p->AddPacketTag(randomId);

		 //trace:
		 (DynamicCast<Icmpv6L4Protocol_Ndpp>(icmpv6))->m_txNdTrace (p, Icmpv6L4Protocol_Ndpp::NDPP_NA_1HOP,
				 mprOptType, Icmpv6L4Protocol_Ndpp::MPR_PROCESSES,
				 ipv6->GetInterfaceForDevice(this->GetDevice()), Seconds(0.0));
		 Simulator::Schedule (Seconds (0.0), &Ipv6Interface::Send, this, p, Ipv6Address::GetAllNodesMulticast());
	}
	//"set timer" - schedule next event
	Simulator::Schedule (Seconds (m_MPRP_DELAY), &Ipv6Interface_Ndpp::MprTimeout, this);

}

void Ipv6Interface_Ndpp::MprLookUp()
{
	OneHopSet N;	//1-hop neigh.
	TwoHopSet N2;	//2-hops
	int sizeN2 = 0;

	NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Choosing MPRs");
	//clean MPR Set first:
	m_mprData.m_mprs.clear();

	sizeN2=ListCopyStat(&N, &N2);

	OneHopSetI it = N.begin();
	OneHopSetI temp;

	while(it != N.end())
	{
		if(it->m_willingness == NeighInfo::WILL_ALWAYS)		//(1)
		{
			temp = it++;
			sizeN2 = AddToMPR(it, &N, &N2, sizeN2);
			it = temp;
		}
		else
			it++;
	}

	NeighSetCI it2 = N2.begin();
	while(it2 != N2.end())		//(3)
	{
		if(it2->m_other == 1)		// if node from N2 is covered by only one node from N
		{
			for (OneHopSetI it = N.begin (); it != N.end (); ++it)
			{
				if(IsOn2hopList(it2->m_addr, &(it->m_2hops)))
				{
					sizeN2 = AddToMPR(it, &N, &N2, sizeN2);
					break;
				}
			}
			it2 = N2.begin();
		}
		else
			it2++;
	}

	while(sizeN2>0)		//(4)
	{
		CleanList(&N, &N2);		//(4.1)

		OneHopSetI nMPR = N.begin();

		for (OneHopSetI it = N.begin (); it != N.end (); ++it)	//(4.2)
		{
			if(it->m_willingness < nMPR->m_willingness)
			{
				continue;
			}
			if(it->m_willingness > nMPR->m_willingness)
			{
				nMPR = it;
				continue;
			}
			else
			{
				if(it->m_reachability > nMPR->m_reachability)
				{
					nMPR = it;
					continue;
				}
				if(it->m_reachability == nMPR->m_reachability)
				{
					if(it->m_other > nMPR->m_other)	//compare D(y)
					{
						nMPR = it;
						continue;
					}
				}
			}
		}
		sizeN2 = AddToMPR(nMPR, &N, &N2, sizeN2);
	}

	//clean remaining elements of N (not MPRs):

	while(!N.empty())
	{
		it = N.begin();
		//remove 2-hops
		it->m_2hops.clear();

		//remove node from N:
		N.erase(it);
	}
}

int Ipv6Interface_Ndpp::ListCopyStat(OneHopSet *tN, TwoHopSet *tN2)
{
	int sizeN2=0;
	NeighSetI it3;
	NeighSetCI it2;

	for (OneHopSetCI it = m_mprData.m_neighbours.begin(); it != m_mprData.m_neighbours.end(); ++it)
	{
		//check also node status - only symmetric neighbours are selected for N and only their symmetric 2-hops are taken into consideration

		if(it->m_willingness == NeighInfo::WILL_NEVER || it->m_neightype != NeighInfo::SYM_NEIGH)  // (i) , if willingness==WILL_NEVER the node is not added to 1 hop list N
		{
			continue;
		}
		//insert OneHop node
		tN->push_back(OneHop(it->m_addr, it->m_willingness, it->m_neightype, it->m_linktype));

		for (it2 = it->m_2hops.begin (); it2 != it->m_2hops.end (); ++it2)
		{
			if(it2->m_neightype != NeighInfo::SYM_NEIGH)	//add symmetric 2-hop neighbours only!
				continue;
			if(IsOn1hopList(it2->m_addr)) // (ii), (iii) -> do not add node to 2 hop list because it belongs to 1 hop list (and is a symmetric neighbour)
				continue;
			else
			{
				//insert 2-hop node to the 2 hop list from the current element of tN
				tN->back().m_2hops.push_back((*it2));
				tN->back().m_other++;	//(2) -> D(y)

				for (it3 = tN2->begin (); it3 != tN2->end (); ++it3)
				{
					if((*it3).m_addr == it2->m_addr)
					{
						// do not add node to 2 hop list because it is already there
						(*it3).m_other++;	//"reachability"	//TODO: check it!!!!
						break;
					}
				}
				if(it3 == tN2->end())
				{
					//insert 2-hop node to tN2
					tN2->push_back(NeighInfo(it2->m_addr, it2->m_willingness, it2->m_neightype, it2->m_linktype, 1));
					sizeN2++;	// update the size of the N2 list
				}
			}
		}

	}

	return sizeN2;
}

int Ipv6Interface_Ndpp::IsOn1hopList(Ipv6AddrRandomIDPair taddr)
{
	for (OneHopSetCI it = m_mprData.m_neighbours.begin (); it != m_mprData.m_neighbours.end (); ++it)
		if(it->m_addr == taddr && it->m_neightype == NeighInfo::SYM_NEIGH)
			return 1;
	return 0;
}

int Ipv6Interface_Ndpp::AddToMPR(OneHopSetI elem_it,OneHopSet *tN,TwoHopSet *tN2, int tsizeN2)
{
	m_mprData.m_mprs.push_back(NeighInfo(elem_it->m_addr, elem_it->m_willingness, elem_it->m_neightype, elem_it->m_linktype));

	NS_LOG_LOGIC("Node " << this->m_node->GetId() << ": Neighbor address " << elem_it->m_addr << " added to MPRs with: willingness "
			<< (int) elem_it->m_willingness << ", neightype " << (int) elem_it->m_neightype << ", linktype " << (int)elem_it->m_linktype);

	for (NeighSetCI it = elem_it->m_2hops.begin(); it != elem_it->m_2hops.end(); ++it)
	{
		if(!tN2->empty())
		{
			for(NeighSetI it2 = tN2->begin(); it2 != tN2->end(); ++it2)
			{
				if(it->m_addr == it2->m_addr)
				{
					tN2->erase(it2);
					tsizeN2--;
					break;
				}
			}
		}
		else
		{
			tsizeN2 = 0;	//just for safety, probably unnecessary
			break;
		}
	}

	//remove neigh. from N:
	elem_it->m_2hops.clear();
	tN->erase(elem_it);

	return tsizeN2;
}

void Ipv6Interface_Ndpp::CleanList(OneHopSet *tN, TwoHopSet *tN2)
{
	NeighSetI it2;
	OneHopSetI it = tN->begin ();

	while(it != tN->end ())
	{
		it2 = it->m_2hops.begin();
		while (it2 != it->m_2hops.end())
		{
			if(!IsOn2hopList(it2->m_addr, tN2))
			{
				it2 = it->m_2hops.erase(it2);	//erase and move it2 to next elem
			}
			else
			{
				it->m_reachability++;
				it2++;
			}
		}
		//extended to cover (4.2) in RFC3626!
		if(it->m_reachability == 0)
		{
			//do not consider this any further - node does not have neighbours or they are all already covered
			it->m_2hops.clear();
			it = tN->erase(it);
		}
		else
			it++;
	}
}

int  Ipv6Interface_Ndpp::IsOn2hopList(Ipv6AddrRandomIDPair taddr, TwoHopSet *thead)
{
	for (NeighSetCI it = thead->begin(); it != thead->end(); ++it)
	{
		if(it->m_addr == taddr)	//is on 2-hop list
			return 1;
	}
	return 0;
}

} /* namespace ns3 */

