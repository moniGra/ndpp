/*
 * mpr-lists.cc
 *
 *  Created on: Nov 8, 2011
 *      Author: monika
 */
#include "ns3/log.h"

#include "mpr-lists.h"
#include "ns3/ipv6-address.h"

namespace ns3
{

const int OneHop::VAL_T_INTERVAL = 3;
const int OneHop::LOST_T_INTERVAL = 6;

//////////////Ipv6AddrRandomIDPair:
/*bool Ipv6AddrRandomIDPair::IsEqual (const Ipv6AddrRandomIDPair& other) const
{
 // NS_LOG_FUNCTION (this << other);
	if (m_randomID == -1)
		return Ipv6Address::IsEqual(Ipv6Address(other));


  if (!memcmp (m_address, other.m_address, 16) && m_randomID == other.m_randomID)
    {
      return true;
    }
  return false;
}*/


//////////////////////////////////////////

NeighInfo::NeighInfo(Ipv6Address addr, uint8_t willingness, uint8_t neightype, uint8_t linktype, uint8_t other)
	: m_addr(Ipv6AddrRandomIDPair(addr, -1)),
	  m_willingness(willingness),
	  m_neightype(neightype),
	  m_linktype(linktype),
	  m_other(other)
{

}

NeighInfo::NeighInfo(Ipv6AddrRandomIDPair addr, uint8_t willingness, uint8_t neightype, uint8_t linktype, uint8_t other)
	: m_addr(addr),
	  m_willingness(willingness),
	  m_neightype(neightype),
	  m_linktype(linktype),
	  m_other(other)
{

}

NeighInfo::NeighInfo(const NeighInfo &o)
	:m_addr(o.m_addr),
	 m_willingness(o.m_willingness),
	 m_neightype(o.m_neightype),
	 m_linktype(o.m_linktype),
	 m_other(o.m_other)
{
}

OneHop::OneHop(uint8_t reachability, Time sym_t, Time asym_t, Time lost_t)
	: m_reachability(reachability),
	  m_sym_t(sym_t),
	  m_asym_t(asym_t),
	  m_lost_t(lost_t)
{
}

OneHop::OneHop(Ipv6Address addr, uint8_t willingness, uint8_t neightype, uint8_t linktype,
			uint8_t other, uint8_t reachability, Time sym_t, Time asym_t, Time lost_t)
	: NeighInfo(addr, willingness, neightype, linktype, other),
	  m_reachability(reachability),
	  m_sym_t(sym_t),
	  m_asym_t(asym_t),
	  m_lost_t(lost_t)
{
}

OneHop::OneHop(Ipv6AddrRandomIDPair addr, uint8_t willingness, uint8_t neightype, uint8_t linktype,
			uint8_t other, uint8_t reachability, Time sym_t, Time asym_t, Time lost_t)
	: NeighInfo(addr, willingness, neightype, linktype, other),
	  m_reachability(reachability),
	  m_sym_t(sym_t),
	  m_asym_t(asym_t),
	  m_lost_t(lost_t)
{
}

OneHop::OneHop(const OneHop &o)
	: NeighInfo(o),
	  m_reachability(o.m_reachability),
	  m_sym_t(o.m_sym_t),
	  m_asym_t(o.m_asym_t),
	  m_lost_t(o.m_lost_t)
{
}

Selector::Selector(Ipv6Address addr, Time val_t)
	: m_addr(Ipv6AddrRandomIDPair(addr, -1)),
	  m_val_t(val_t)
{
}

Selector::Selector(Ipv6AddrRandomIDPair addr, Time val_t)
	: m_addr(addr),
	  m_val_t(val_t)
{
}

Selector::Selector(const Selector &o)
	: m_addr(o.m_addr),
	  m_val_t(o.m_val_t)
{

}


}


