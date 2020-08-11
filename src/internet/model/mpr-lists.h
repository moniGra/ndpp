/*
 * mpr-lists.h
 *
 *  Created on: Nov 8, 2011
 *      Author: monika
 */

#ifndef MPR_LISTS_H_
#define MPR_LISTS_H_

#include <list>
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"

namespace ns3
{

class Ipv6AddrRandomIDPair : public Ipv6Address
{
public:
	int32_t m_randomID;

	Ipv6AddrRandomIDPair() : m_randomID(-1) {}
	/**
	* \brief Constructs an Ipv6Address by parsing the input C-string.
	* \param address the C-string containing the IPv6 address (e.g. 2001:660:4701::1).
	*/
	Ipv6AddrRandomIDPair (char const* address) : Ipv6Address(address), m_randomID(-1) {}
	Ipv6AddrRandomIDPair (char const* address, int32_t randomID) : Ipv6Address(address), m_randomID(randomID) {}

	/**
	* \brief Constructs an Ipv6Address by using the input 16 bytes.
	* \param address the 128-bit address
	* \warning the parameter must point on a 16 bytes integer array!
	*/
	Ipv6AddrRandomIDPair (uint8_t address[16])  : Ipv6Address(address), m_randomID(-1) {} ;
	Ipv6AddrRandomIDPair (uint8_t address[16], int32_t randomID) : Ipv6Address(address), m_randomID(randomID) {}

	/**
	* \brief Copy constructor.
	* \param addr Ipv6Address object
	*/
	Ipv6AddrRandomIDPair (Ipv6AddrRandomIDPair const & addr) : Ipv6Address(addr), m_randomID(addr.m_randomID) {}
	Ipv6AddrRandomIDPair (Ipv6Address const & addr, int32_t randomId) : Ipv6Address(addr), m_randomID(randomId) {}
	/**
	* \brief Copy constructor.
	* \param addr Ipv6Address pointer
	*/
	Ipv6AddrRandomIDPair (Ipv6AddrRandomIDPair const* addr) : Ipv6Address(addr), m_randomID (addr->m_randomID) {}

	virtual ~Ipv6AddrRandomIDPair(){};

	 /**
	   * \brief Comparison operation between two Ipv6Addresses.
	   *
	   * \param other the IPv6 address to which to compare thisaddress
	   * \return true if the addresses are equal, false otherwise
	   */
//	bool IsEqual (const Ipv6AddrRandomIDPair& other) const;

	friend bool operator == (Ipv6AddrRandomIDPair const &a, Ipv6AddrRandomIDPair const &b);
	friend bool operator != (Ipv6AddrRandomIDPair const &a, Ipv6AddrRandomIDPair const &b);
	//przeladowany operator ==
};

inline bool operator == (const Ipv6AddrRandomIDPair& a, const Ipv6AddrRandomIDPair& b)
{
	if (a.m_randomID == -1 || b.m_randomID == -1)
		return Ipv6Address(a) == Ipv6Address(b);

  return (!std::memcmp (a.m_address, b.m_address, 16) && a.m_randomID == b.m_randomID);
}

inline bool operator != (const Ipv6AddrRandomIDPair& a, const Ipv6AddrRandomIDPair& b)
{
	if (a.m_randomID == -1 || b.m_randomID == -1)
		return Ipv6Address(a) != Ipv6Address(b);

	return (std::memcmp (a.m_address, b.m_address, 16) || a.m_randomID != b.m_randomID);
}

class NeighInfo{
public:

	//willingness
	enum Willingness_e
	{
		WILL_NEVER = 0x00,
		WILL_LOW = 0x01,
		WILL_DEFAULT = 0x03,
		WILL_HIGH = 0X06,
		WILL_ALWAYS = 0x07,
	};

	enum Neightype_e
	{
		NOT_NEIGH = 0x00,	//=0	//no longer neigh OR not yet symmetric (==not sym. neighbour)
		SYM_NEIGH = 0x04,	//=1
		MPR_NEIGH = 0x08,	//=2	//not used
	};

	enum Linktype_e
	{
		UNSPEC_LINK = 0x00,	//=0
		ASYM_LINK = 0x01,	//=1	//I can hear a neighbour
		SYM_LINK = 0x02,	//=2
		LOST_LINK = 0x03,	//=3	//lost symmetry - I cannot be heard anymore
	};

	enum Linkcode_e
	{
		LC_INVALID = 0x02,	//invalid - not neigh. & symm. link
		LC_ALL_ENTRIES = 12,	//including 1 invalid entry - 0x02 (not neigh. & symm. link)
	};

	NeighInfo(Ipv6AddrRandomIDPair addr = Ipv6AddrRandomIDPair ("::", -1), uint8_t willingness = WILL_DEFAULT, uint8_t neightype = NOT_NEIGH, uint8_t linktype = UNSPEC_LINK, uint8_t other = 0);
	NeighInfo(Ipv6Address addr, uint8_t willingness = WILL_DEFAULT, uint8_t neightype = NOT_NEIGH, uint8_t linktype = UNSPEC_LINK, uint8_t other = 0);
	NeighInfo(const NeighInfo &o);
	~NeighInfo(){};

	Ipv6AddrRandomIDPair m_addr;
	uint8_t m_willingness;
	uint8_t m_neightype;
	uint8_t m_linktype;	//not needed?
	uint8_t m_other;
};
typedef std::list<NeighInfo> TwoHopSet;	//list for storing info about 2-hop neighbours

class OneHop : public NeighInfo{
public:

	/**
	* \brief MPR algorithm constants
	*/
	static const int VAL_T_INTERVAL;
	static const int LOST_T_INTERVAL;

	OneHop(uint8_t reachability = 0, Time sym_t= Seconds(0), Time asym_t = Seconds(0), Time lost_t = Seconds(0));
	OneHop(Ipv6Address addr, uint8_t willingness = WILL_DEFAULT, uint8_t neightype = NOT_NEIGH, uint8_t linktype = UNSPEC_LINK,
				uint8_t other = 0, uint8_t reachability = 0, Time sym_t= Seconds(0), Time asym_t = Seconds(0), Time lost_t = Seconds(0));
	OneHop(Ipv6AddrRandomIDPair addr, uint8_t willingness = WILL_DEFAULT, uint8_t neightype = NOT_NEIGH, uint8_t linktype = UNSPEC_LINK,
				uint8_t other = 0, uint8_t reachability = 0, Time sym_t= Seconds(0), Time asym_t = Seconds(0), Time lost_t = Seconds(0));
	OneHop(const OneHop &o);

	~OneHop(){};

	uint8_t m_reachability;

	Time	m_sym_t;	//in seconds, time until link is considered symmetric
	Time	m_asym_t;	//in seconds, time until neigh. on this link is considered heard
	Time	m_lost_t;	//in seconds, time until record expires and MUST be removed (lost link is advertised until this time, then associated neigh. is removed

	TwoHopSet m_2hops;
};

class Selector{
public:

	Selector(Ipv6AddrRandomIDPair addr = Ipv6AddrRandomIDPair("::", -1), Time val_t = Seconds(0));
	Selector(Ipv6Address addr, Time val_t = Seconds(0));
	//copy constructor:
	Selector(const Selector &o);
	~Selector(){};

	Ipv6AddrRandomIDPair m_addr;
	Time m_val_t;	//validity time of each entry

};

typedef std::list<Selector> MprSelectors;	//list for storing info about my MPR Selectors (those who chosen me as MPR)
typedef std::list<Selector>::iterator SelectorsI;
typedef std::list<NeighInfo> MprSet;	//list for storing info about 2-hop neighbours
typedef std::list<OneHop> OneHopSet;	//list for storing info about 1-hop neighbours
typedef std::list<OneHop>::iterator OneHopSetI;
typedef std::list<OneHop>::const_iterator OneHopSetCI;
typedef std::list<NeighInfo>::iterator NeighSetI;
typedef std::list<NeighInfo>::const_iterator NeighSetCI;


//typedef std::list<NeighInfo> LcNeigh;	//list for storing neigh. information for neighbors with the same link code

class MprData{
public:

	MprData() {};
	~MprData(){};

	MprSelectors m_selectors;
	ns3::MprSet m_mprs;
	OneHopSet m_neighbours;

	//TODO: add timer?
};

}

#endif /* MPR_LISTS_H_ */
