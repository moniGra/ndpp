/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 Support for ND modification 1 (NdSimple)
 author: Monika Grajzer
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/names.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/core-config.h"
#include "ns3/arp-l3-protocol.h"
#include "internet-stack-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv6-list-routing-helper.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/boolean.h"
#include <limits>
#include <map>
#include <vector>

NS_LOG_COMPONENT_DEFINE ("InternetStackHelper_Nd");

namespace ns3 {

static bool icmpv6AsciiTracingEnabled = false;


InternetStackHelper_Nd::~InternetStackHelper_Nd()
{
	 /* delete m_routing;
	  delete m_routingv6;*/
	for(DadStatsMessageTypeI it = m_txDadStatsMessageType.begin();
			it != m_txDadStatsMessageType.end();it++)
		delete (*it);
	m_txDadStatsMessageType.clear();

	for(DadStatsMessageTypeI it = m_rxDadStatsMessageType.begin();
			it != m_rxDadStatsMessageType.end();it++)
		delete (*it);
	m_rxDadStatsMessageType.clear();
	for(DadStatsExitCodeI it = m_exitCodeDadStats.begin();
				it != m_exitCodeDadStats.end();it++)
			delete (*it);
	m_exitCodeDadStats.clear();
	for(std::list<LatencyPerNode *>::iterator it = m_latencies.begin();
					it != m_latencies.end();it++)
			delete (*it);
	m_latencies.clear();
}

void
InternetStackHelper_Nd::Install (Ptr<Node> node) const
{
	if(m_ndType == ND_BASIC)
	{
		InternetStackHelper::Install(node);
		return;
	}

  if (m_ipv4Enabled)
    {
      if (node->GetObject<Ipv4> () != 0)
        {
          NS_FATAL_ERROR ("InternetStackHelper_Nd::Install (): Aggregating "
                          "an InternetStack to a node with an existing Ipv4 object");
          return;
        }

      CreateAndAggregateObjectFromTypeId (node, "ns3::ArpL3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv4L3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv4L4Protocol");
      if (m_ipv4ArpJitterEnabled == false)
	  {
		Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol> ();
		NS_ASSERT (arp);
		arp->SetAttribute ("RequestJitter", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
	  }
      // Set routing
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      Ptr<Ipv4RoutingProtocol> ipv4Routing = m_routing->Create (node);
      ipv4->SetRoutingProtocol (ipv4Routing);
    }

  if (m_ipv6Enabled)
    {
      /* IPv6 stack */
      if (node->GetObject<Ipv6> () != 0)
        {
          NS_FATAL_ERROR ("InternetStackHelper_Nd::Install (): Aggregating "
                          "an InternetStack to a node with an existing Ipv6 object");
          return;
        }

      switch(m_ndType)
      {
      case ND_SIMPLE:
    	  CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv6L3Protocol");
    	  CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv6L4Protocol_NdSimple");
    	  break;
      case ND_NDPP:
    	  CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv6L3Protocol_Ndpp");
    	  CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv6L4Protocol_Ndpp");
    	  break;
      case ND_NDPP_FRW:
    	  CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv6L3Protocol_Ndpp");
		  CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv6L4Protocol_NdppFrw");
		  break;
      case ND_NDPP_MPR:
		  CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv6L3Protocol_Ndpp");
		  CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv6L4Protocol_NdppMpr");
		  break;
      case ND_NDPP_MPR2:
		 // CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv6L3Protocol_Ndpp");
		 // CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv6L4Protocol_NdppMpr2");
		  break;
      case ND_BASIC:
      default:
    	  CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv6L3Protocol");
    	  CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv6L4Protocol");
    	  break;
      }

      if (m_ipv6NsRsJitterEnabled == false)
	  {
		Ptr<Icmpv6L4Protocol> icmpv6l4 = node->GetObject<Icmpv6L4Protocol> ();
		NS_ASSERT (icmpv6l4);
		icmpv6l4->SetAttribute ("SolicitationJitter", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
	  }
      // Set routing
      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
      Ptr<Ipv6RoutingProtocol> ipv6Routing = m_routingv6->Create (node);
      ipv6->SetRoutingProtocol (ipv6Routing);
      //MG:
      ipv6->SetAttribute("IpForward", BooleanValue(true));

      /* register IPv6 extensions and options */
      ipv6->RegisterExtensions ();
      ipv6->RegisterOptions ();
    }
  	if (m_ipv4Enabled || m_ipv6Enabled)
	{
		CreateAndAggregateObjectFromTypeId (node, "ns3::UdpL4Protocol");
		node->AggregateObject (m_tcpFactory.Create<Object> ());
		Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory> ();
		node->AggregateObject (factory);
	}
}

void InternetStackHelper_Nd::SetNdType(int type)
{
	m_ndType = type;
}

/////////////////////////////////////////////////////////////////////////////
//trace sinks:

static void
TxNdSinkWithoutContext_DadStats (
		std::vector<int>* txMessages,
		Ptr<const Packet> packet,
		Icmpv6L4Protocol_NdSimple::NdppMessageType_e ndppMessageType,
		Icmpv6L4Protocol_NdSimple::MprOptType_e mprOptType,
		Icmpv6L4Protocol_NdSimple::EntryCode_e entryCode,
		uint32_t interfaceNo,
		Time timeOffsetToSend)
{
	if(icmpv6AsciiTracingEnabled)
	{
		(*txMessages)[1] = interfaceNo;
		if(mprOptType != Icmpv6L4Protocol_NdSimple::NO_MPR_OPT)
		{
			if(ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NA_1HOP)
				ndppMessageType = Icmpv6L4Protocol_NdSimple::NDPP_NA_1HOP_MPRs;
			else
				NS_ASSERT_MSG(false, "TxNdSinkWithoutContext_DadStats: MPR option detected for not 1-hop NA-type message!!!");
		}

		(*txMessages)[ndppMessageType+2]= (*txMessages)[ndppMessageType+2] + 1;
	}
}

static void
RxNdSinkWithoutContext_DadStats (
		std::vector<int>* rxMessages,
		Ptr<const Packet> packet,
		const Icmpv6Header* header,
		Icmpv6L4Protocol_NdSimple::NdppMessageType_e ndppMessageType,
		Icmpv6L4Protocol_NdSimple::MprOptType_e mprOptType,
		Icmpv6L4Protocol_NdSimple::ExitCode_e exitCode,
		uint32_t interfaceNo)
{
	if(icmpv6AsciiTracingEnabled)
	{
		(*rxMessages)[1] = interfaceNo;
		if(mprOptType != Icmpv6L4Protocol_NdSimple::NO_MPR_OPT)
		{
			if(ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NA_1HOP)
				ndppMessageType = Icmpv6L4Protocol_NdSimple::NDPP_NA_1HOP_MPRs;
			else
				NS_ASSERT_MSG(false, "TxNdSinkWithoutContext_DadStats: MPR option detected for not 1-hop NA-type message!!!");
		}

		(*rxMessages)[ndppMessageType+2]= (*rxMessages)[ndppMessageType+2] + 1;
	}
}

static void
ExitCodeSinkWithoutContext_DadStats (
		std::vector<int>* exitCodeVector,
		Ptr<const Packet> packet,
		const Icmpv6Header* header,
		Icmpv6L4Protocol_NdSimple::NdppMessageType_e ndppMessageType,
		Icmpv6L4Protocol_NdSimple::MprOptType_e mprOptType,
		Icmpv6L4Protocol_NdSimple::ExitCode_e exitCode,
		uint32_t interfaceNo)
{
	if(icmpv6AsciiTracingEnabled)
	{
		(*exitCodeVector)[1] = interfaceNo;
		(*exitCodeVector)[exitCode+2] = (*exitCodeVector)[exitCode+2] + 1;
	}
}

static void
FirstmNSTimeSinkWithoutContext_DadStats (
		Time* first_mNStime,
		Ptr<const Packet> packet,
		Icmpv6L4Protocol_NdSimple::NdppMessageType_e ndppMessageType,
		Icmpv6L4Protocol_NdSimple::MprOptType_e mprOptType,
		Icmpv6L4Protocol_NdSimple::EntryCode_e entryCode,
		uint32_t interfaceNo,
		Time timeOffsetToSend)
{
	if(icmpv6AsciiTracingEnabled && *first_mNStime == Seconds(-1.0)
			&& ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP)
	{
		*first_mNStime = Simulator::Now() + timeOffsetToSend;
	}
}

static void
LastmNSTimeSinkWithoutContext_DadStats (
		Time* last_mNStime,
		Ptr<const Packet> packet,
		const Icmpv6Header* header,
		Icmpv6L4Protocol_NdSimple::NdppMessageType_e ndppMessageType,
		Icmpv6L4Protocol_NdSimple::MprOptType_e mprOptType,
		Icmpv6L4Protocol_NdSimple::ExitCode_e exitCode,
		uint32_t interfaceNo)
{
	if(icmpv6AsciiTracingEnabled && ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP)
	{
		*last_mNStime = Simulator::Now();
	}
}


static void
FirstmNDMessTimeSinkWithoutContext_DadStats (
		Time* first_mNDMesstime,
		Ptr<const Packet> packet,
		Icmpv6L4Protocol_NdSimple::NdppMessageType_e ndppMessageType,
		Icmpv6L4Protocol_NdSimple::MprOptType_e mprOptType,
		Icmpv6L4Protocol_NdSimple::EntryCode_e entryCode,
		uint32_t interfaceNo,
		Time timeOffsetToSend)
{
	if(icmpv6AsciiTracingEnabled && *first_mNDMesstime == Seconds(-1.0)
			&& (ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP ||
					ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NA_MHOP))
	{
		NS_ASSERT_MSG(ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP, "mNA message registered first in ND++ - sth. is wrong!!!");
		*first_mNDMesstime = Simulator::Now() + timeOffsetToSend;
	}
}

static void
LastmNDMessTimeSinkWithoutContext_DadStats (
		Time* last_mNDMesstime,
		Ptr<const Packet> packet,
		const Icmpv6Header* header,
		Icmpv6L4Protocol_NdSimple::NdppMessageType_e ndppMessageType,
		Icmpv6L4Protocol_NdSimple::MprOptType_e mprOptType,
		Icmpv6L4Protocol_NdSimple::ExitCode_e exitCode,
		uint32_t interfaceNo)
{
	if(icmpv6AsciiTracingEnabled && (ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP ||
								ndppMessageType == Icmpv6L4Protocol_NdSimple::NDPP_NA_MHOP))
	{
		*last_mNDMesstime = Simulator::Now();
	}
}

static void
LatencySinkWithoutContext_DadStats (
		LatencyPerNode* latency,
		Ptr<Ipv6Interface> interface,
		Ipv6Address address,
		Icmpv6L4Protocol_Ndpp::InterfaceState_e state)
{
	if(icmpv6AsciiTracingEnabled)
	{
		//NS_ASSERT_MSG(latency->m_addr.IsEqual(address),
		//		"LAtency Trace: Latency entry found for another address - sth. is wrong - verify!");
		switch(state)
		{
		case Icmpv6L4Protocol_Ndpp::INIT:
			if(latency->m_latencyInitTime == Seconds(-1.0))
				latency->m_latencyInitTime = Simulator::Now();
			break;
		case Icmpv6L4Protocol_Ndpp::INVALID:
			if(latency->m_latencyInvalidTime == Seconds(-1.0))
				latency->m_latencyInvalidTime = Simulator::Now();
			break;
		case Icmpv6L4Protocol_Ndpp::PREFERRED:
			NS_ASSERT_MSG(latency->m_latencyPreferredTime == Seconds(-1.0),
					"Latency Trace: Preferred state found for the second time - sth. is wrong - verify!");
			latency->m_latencyPreferredTime = Simulator::Now();
			break;
		default:
			break;
		}

	}
}

///////////////////////////////////////////////////////////

void
InternetStackHelper_Nd::EnableAsciiIcmpv6Internal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<Icmpv6L4Protocol> icmpv6,
  bool explicitFilename)
{
	if (!m_ipv6Enabled)
    {
      NS_LOG_INFO ("Call to enable Icmpv6 ascii tracing but Ipv6 not enabled");
      return;
    }

  //
  // Our trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and do a hook WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  	if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.

      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
     /* if (explicitFilename)
      {*/
    	  filename = prefix;
     /* }
      else
      {
    	  filename = asciiTraceHelper.GetFilenameFromInterfacePair (prefix, icmpv6, interface);
      }*/

    	if(!m_outputStream)
    		m_outputStream = asciiTraceHelper.CreateFileStream (filename);
    }
  	else if(!m_outputStream)
  		m_outputStream = stream;

  	//connect with trace sink:

  	Ptr<Icmpv6L4Protocol_NdSimple> icmpv6_nd = DynamicCast<Icmpv6L4Protocol_NdSimple>(icmpv6);
	if(!icmpv6_nd)
	{
	  NS_LOG_INFO ("Call to enable Icmpv6 ascii tracing but neither ND++ nor ND_Simple are enabled");
	  return;
	}

	//create new list entry for this node data collection
	std::vector<int>* txMessages = new std::vector<int>(Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2, 0);
	std::vector<int>* rxMessages = new std::vector<int>(Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2, 0);
	std::vector<int>* exitCodeVector = new std::vector<int>(Icmpv6L4Protocol_NdSimple::ExitCodeTypeSize + 2, 0);
	LatencyPerNode * latency = new LatencyPerNode();

	//put node no in the first element:
	uint32_t nodeNo = icmpv6->GetObject<Node>()->GetId();
	(*txMessages)[0] = nodeNo;
	(*rxMessages)[0] = nodeNo;
	(*exitCodeVector)[0] = nodeNo;
	latency->m_nodeNo = nodeNo;

	m_txDadStatsMessageType.push_back(txMessages);
	m_rxDadStatsMessageType.push_back(rxMessages);
	m_exitCodeDadStats.push_back(exitCodeVector);
	m_latencies.push_back(latency);

  	bool __attribute__ ((unused)) result = icmpv6->TraceConnectWithoutContext ("TxNd",
  			MakeBoundCallback (&TxNdSinkWithoutContext_DadStats, m_txDadStatsMessageType.back()));
	NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
				 "Unable to connect icmpv6L4Protocol_NdSimple \"TxNd\"");

	bool __attribute__ ((unused)) result2 = icmpv6->TraceConnectWithoutContext ("RxNd",
	  			MakeBoundCallback (&RxNdSinkWithoutContext_DadStats, m_rxDadStatsMessageType.back()));
		NS_ASSERT_MSG (result2 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
					 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd\"");
	bool __attribute__ ((unused)) result3 = icmpv6->TraceConnectWithoutContext ("RxNd",
						MakeBoundCallback (&ExitCodeSinkWithoutContext_DadStats, m_exitCodeDadStats.back()));
		NS_ASSERT_MSG (result3 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
					 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd - Exit Stats\"");
	bool __attribute__ ((unused)) result4 = icmpv6->TraceConnectWithoutContext ("TxNd",
					MakeBoundCallback (&FirstmNSTimeSinkWithoutContext_DadStats, &m_first_mNStime));
			NS_ASSERT_MSG (result4 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
						 "Unable to connect icmpv6L4Protocol_NdSimple \"TxNd - first_mNStime\"");
	bool __attribute__ ((unused)) result5 = icmpv6->TraceConnectWithoutContext ("RxNd",
					MakeBoundCallback (&LastmNSTimeSinkWithoutContext_DadStats, &m_last_mNStime));
			NS_ASSERT_MSG (result5 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
						 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd - last_mNStime\"");
	bool __attribute__ ((unused)) result6 = icmpv6->TraceConnectWithoutContext ("Latency",
					MakeBoundCallback (&LatencySinkWithoutContext_DadStats, m_latencies.back()));
			NS_ASSERT_MSG (result6 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
						 "Unable to connect icmpv6L4Protocol_Ndpp \"LAtency trace\"");
	bool __attribute__ ((unused)) result7 = icmpv6->TraceConnectWithoutContext ("TxNd",
					MakeBoundCallback (&FirstmNDMessTimeSinkWithoutContext_DadStats, &m_first_mNDMessTime));
			NS_ASSERT_MSG (result7 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
						 "Unable to connect icmpv6L4Protocol_NdSimple \"TxNd - m_first_mNDMessTime\"");
	bool __attribute__ ((unused)) result8 = icmpv6->TraceConnectWithoutContext ("RxNd",
					MakeBoundCallback (&LastmNDMessTimeSinkWithoutContext_DadStats, &m_last_mNDTtime));
			NS_ASSERT_MSG (result8 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
						 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd - m_last_mNDTtime\"");
}

///////////////////////////////////////////////////////////

void
InternetStackHelper_Nd::EnableAsciiIcmpv6Internal (
  Ptr<Icmpv6L4Protocol> icmpv6,
  bool explicitFilename)
{
	if (!m_ipv6Enabled)
    {
      NS_LOG_INFO ("Call to enable Icmpv6 ascii tracing but Ipv6 not enabled");
      return;
    }

  //
  // Our trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  	//connect with trace sink:

  	Ptr<Icmpv6L4Protocol_NdSimple> icmpv6_nd = DynamicCast<Icmpv6L4Protocol_NdSimple>(icmpv6);
	if(!icmpv6_nd)
	{
	  NS_LOG_INFO ("Call to enable Icmpv6 ascii tracing but neither ND++ nor ND_Simple are enabled");
	  return;
	}

	//create new list entry for this node data collection
	std::vector<int>* txMessages = new std::vector<int>(Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2, 0);
	std::vector<int>* rxMessages = new std::vector<int>(Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2, 0);
	std::vector<int>* exitCodeVector = new std::vector<int>(Icmpv6L4Protocol_NdSimple::ExitCodeTypeSize + 2, 0);
	LatencyPerNode * latency = new LatencyPerNode();

	//put node no in the first element:
	uint32_t nodeNo = icmpv6->GetObject<Node>()->GetId();
	(*txMessages)[0] = nodeNo;
	(*rxMessages)[0] = nodeNo;
	(*exitCodeVector)[0] = nodeNo;
	latency->m_nodeNo = nodeNo;

	m_txDadStatsMessageType.push_back(txMessages);
	m_rxDadStatsMessageType.push_back(rxMessages);
	m_exitCodeDadStats.push_back(exitCodeVector);
	m_latencies.push_back(latency);

  	bool __attribute__ ((unused)) result = icmpv6->TraceConnectWithoutContext ("TxNd",
  			MakeBoundCallback (&TxNdSinkWithoutContext_DadStats, m_txDadStatsMessageType.back()));
	NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
				 "Unable to connect icmpv6L4Protocol_NdSimple \"TxNd\"");

	bool __attribute__ ((unused)) result2 = icmpv6->TraceConnectWithoutContext ("RxNd",
	  			MakeBoundCallback (&RxNdSinkWithoutContext_DadStats, m_rxDadStatsMessageType.back()));
		NS_ASSERT_MSG (result2 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
					 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd\"");
	bool __attribute__ ((unused)) result3 = icmpv6->TraceConnectWithoutContext ("RxNd",
					MakeBoundCallback (&ExitCodeSinkWithoutContext_DadStats, m_exitCodeDadStats.back()));
			NS_ASSERT_MSG (result3 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
						 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd - Exit Stats\"");
	bool __attribute__ ((unused)) result4 = icmpv6->TraceConnectWithoutContext ("TxNd",
					MakeBoundCallback (&FirstmNSTimeSinkWithoutContext_DadStats, &m_first_mNStime));
			NS_ASSERT_MSG (result4 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
						 "Unable to connect icmpv6L4Protocol_NdSimple \"TxNd - first_mNStime\"");
	bool __attribute__ ((unused)) result5 = icmpv6->TraceConnectWithoutContext ("RxNd",
						MakeBoundCallback (&LastmNSTimeSinkWithoutContext_DadStats, &m_last_mNStime));
				NS_ASSERT_MSG (result5 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
							 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd - last_mNStime\"");
	bool __attribute__ ((unused)) result6 = icmpv6->TraceConnectWithoutContext ("Latency",
						MakeBoundCallback (&LatencySinkWithoutContext_DadStats, m_latencies.back()));
				NS_ASSERT_MSG (result6 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
							 "Unable to connect icmpv6L4Protocol_Ndpp \"LAtency trace\"");
	bool __attribute__ ((unused)) result7 = icmpv6->TraceConnectWithoutContext ("TxNd",
						MakeBoundCallback (&FirstmNDMessTimeSinkWithoutContext_DadStats, &m_first_mNDMessTime));
				NS_ASSERT_MSG (result7 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
							 "Unable to connect icmpv6L4Protocol_NdSimple \"TxNd - m_first_mNDMessTime\"");
		bool __attribute__ ((unused)) result8 = icmpv6->TraceConnectWithoutContext ("RxNd",
						MakeBoundCallback (&LastmNDMessTimeSinkWithoutContext_DadStats, &m_last_mNDTtime));
				NS_ASSERT_MSG (result8 == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
							 "Unable to connect icmpv6L4Protocol_NdSimple \"RxNd - m_last_mNDTtime\"");


}



void InternetStackHelper_Nd::EnableIcmpv6AsciiTracing()
{
	icmpv6AsciiTracingEnabled = true;
}

void InternetStackHelper_Nd::DisableIcmpv6AsciiTracing()
{
	icmpv6AsciiTracingEnabled = false;
}


void InternetStackHelper_Nd::PrintResultsTxRxStats(std::string additionalInfo)
{
	std::string tab = "";

	 *m_outputStream->GetStream () << "Results for experiment (SingleRunFullTxRxTables): "
			 << additionalInfo << std::endl <<std::endl;

	 //print table - stats (ndpp message type) per node in one simulation run

	 //tx stats:
	 *m_outputStream->GetStream () << "txStats:" << std::endl;
	 //header:
	 *m_outputStream->GetStream() << "Node no." << "\t" << "Interface no." << "\t" << "UNKNOWN" << "\t" << "NDPP_NA_1HOP" << "\t"
			 << "NDPP_NA_MHOP" << "\t" << "NDPP_NS_1HOP" << "\t" << "NDPP_NS_MHOP" << std::endl;

	 std::vector<int> txTotal(Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize, 0);
	 for(DadStatsMessageTypeI it = m_txDadStatsMessageType.begin(); it != m_txDadStatsMessageType.end(); it++)
	 {
		 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2,
				 "Assertion failed! - txMessages vector length not corresponding to expected size");
		 tab = "";
		 for(uint i=0;  i< (*it)->size(); i++)
		 {
			 *m_outputStream->GetStream() << tab << (*(*it))[i];
			 tab = "\t";
			 if(i>=2)
			 {
				 txTotal[i-2] += (*(*it))[i];
			 }
		 }
		 *m_outputStream->GetStream() << std::endl;
	 }
	 //print the "total" line
	 *m_outputStream->GetStream() << "total tx" << "\t" << " ";
	 for(uint i=0; i < txTotal.size(); i++)
	 {
		 *m_outputStream->GetStream() << "\t" << txTotal[i];
	 }

	 *m_outputStream->GetStream() << std::endl << std::endl;


	 //rx stats:

	 *m_outputStream->GetStream () << "rxStats:" << std::endl;
	 //header:
	 *m_outputStream->GetStream() << "Node no." << "\t" << "Interface no." << "\t" << "UNKNOWN" << "\t" << "NDPP_NA_1HOP" << "\t"
			 << "NDPP_NA_MHOP" << "\t" << "NDPP_NS_1HOP" << "\t" << "NDPP_NS_MHOP" << std::endl;

	 std::vector<int> rxTotal(Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize, 0);
	 for(DadStatsMessageTypeI it = m_rxDadStatsMessageType.begin(); it != m_rxDadStatsMessageType.end(); it++)
	 {
		 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2,
				 "Assertion failed! - rxMessages vector length not corresponding to expected size");
		 tab = "";
		 for(uint i=0;  i< (*it)->size(); i++)
		 {
			 *m_outputStream->GetStream() << tab << (*(*it))[i];
			 tab = "\t";
			 if(i>=2)
			 {
				 rxTotal[i-2] += (*(*it))[i];
			 }
		 }
		 *m_outputStream->GetStream() << std::endl;
	 }

	 //print the "total" line
	 *m_outputStream->GetStream() << "total rx" << "\t" << " ";
	 for(uint i=0; i < rxTotal.size(); i++)
	 {
		 *m_outputStream->GetStream() << "\t" << rxTotal[i];
	 }
	 *m_outputStream->GetStream() << std::endl << std::endl;

	 //print sum of rx and tx stats:
	 *m_outputStream->GetStream () << "sum of rx and tx messages:" << std::endl;
	 *m_outputStream->GetStream() << "total rx+tx" << "\t" << " ";
	 for(uint i=0; i < rxTotal.size(); i++)
	 {
		 *m_outputStream->GetStream() << "\t" << rxTotal[i]+txTotal[i];
	 }
	 *m_outputStream->GetStream() << std::endl;

/////////////////////////////////////////////////////////////////////////////
	 //print code stats:

	 //Exit Code stats:
	 *m_outputStream->GetStream () << "Exit Code Stats:" << std::endl;
	 //header:
	 *m_outputStream->GetStream() << "Node no." << "\t" << "Interface no." << "\t" << "NO_EXIT_CODE" << "\t" << "NOT_NS_FOR_US" << "\t"
			 << "OWN_DAD_NS_PROBE" << "\t" << "OTHERS_WITH_MY_ADDR" << "\t" << "DAD_QUERY"<< "\t"
			 << "LLA_NEIGH_CACHE_UPDATE" << "\t" << "OWN_DAD_NA_PROBE" << "\t" << "DAD_DUPLICATION" << "\t"
			 << "WARNING_DUPLICATION_POSSIBLE" << "\t" << "DISCARD_NA" << "\t" << "DISCARD_NA_INVALID_ADDR" << "\t"
			 << "DISCARD_NS_INVALID_ADDR" << "\t" << " MPR_INFO" << "\t" << "DUPLICATED_MNS_DROPPED" << "\t"
			 << "DUPLICATED_MNA_DROPPED"<< std::endl;


	 std::vector<int> exitCodeTotal(Icmpv6L4Protocol_NdSimple::ExitCodeTypeSize, 0);
	 for(DadStatsMessageTypeI it = m_exitCodeDadStats.begin(); it != m_exitCodeDadStats.end(); it++)
	 {
		 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::ExitCodeTypeSize + 2,
				 "Assertion failed! - exitCodeDadStats vector length not corresponding to expected size");
		 tab = "";
		 for(uint i=0;  i< (*it)->size(); i++)
		 {
			 *m_outputStream->GetStream() << tab << (*(*it))[i];
			 tab = "\t";
			 if(i>=2)
			 {
				 exitCodeTotal[i-2] += (*(*it))[i];
			 }
		 }
		 *m_outputStream->GetStream() << std::endl;
	 }

	 //print the "total" line
	 *m_outputStream->GetStream() << "total for this exit code" << "\t" << " " << std::endl;
	 for(uint i=0; i < exitCodeTotal.size(); i++)
	 {
		 *m_outputStream->GetStream() << "\t" << exitCodeTotal[i];
	 }
	 *m_outputStream->GetStream() << std::endl << std::endl;
}

/*
void InternetStackHelper_Nd::PrintPartialResultHopLimit(int resultTypeCode, std::string additionalInfo, uint32_t hopLimit)
{
	if(!m_resultPrintInit)
	{
		m_resultPrintInit = true;
		*m_outputStream->GetStream () << "Results for experiment with code: " << resultTypeCode
				<< "(HopLimit vs no of multihop NS messages)" <<std::endl;
		*m_outputStream->GetStream () << additionalInfo << std::endl <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Var" << std::endl;

	}

	*m_outputStream->GetStream() << hopLimit << "\t" << m_avarageResult.Avg() << "\t" << m_avarageResult.Min()
		<< "\t" << m_avarageResult.Max() << "\t" << m_avarageResult.Stddev() << "\t" << m_avarageResult.Var()
		<< std::endl;
}


void InternetStackHelper_Nd::UpdateHopLimitStats()
{
	int txTotalMultihopNS = 0;
	for(DadStatsMessageTypeI it = m_txDadStatsMessageType.begin(); it != m_txDadStatsMessageType.end(); it++)
	{
		 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2,
				 "Assertion failed! - txMessages vector length not corresponding to expected size");

		 txTotalMultihopNS += (*(*it))[Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP+2];
	}
	m_avarageResult.Update(txTotalMultihopNS);
}
*/

void InternetStackHelper_Nd::ResetDadStats()
{
	for(DadStatsMessageTypeI it = m_txDadStatsMessageType.begin(); it != m_txDadStatsMessageType.end(); it++)
	{
		NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2,
				 "Assertion failed! - txMessages vector length not corresponding to expected size");
		for(uint i=2;  i< (*it)->size(); i++)	//leave the first two (node & interface no) unchanged
			(*(*it))[i] = 0;
	}

	for(DadStatsMessageTypeI it = m_rxDadStatsMessageType.begin(); it != m_rxDadStatsMessageType.end(); it++)
	{
	 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2,
			 "Assertion failed! - rxMessages vector length not corresponding to expected size");
	 for(uint i=2;  i< (*it)->size(); i++)	//leave the first two (node & interface no) unchanged
		(*(*it))[i] = 0;
	}

	for(DadStatsExitCodeI it = m_exitCodeDadStats.begin(); it != m_exitCodeDadStats.end(); it++)
	{
	 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::ExitCodeTypeSize + 2,
			 "Assertion failed! - exitCodeDadStats vector length not corresponding to expected size");
	 for(uint i=2;  i< (*it)->size(); i++)	//leave the first two (node & interface no) unchanged
		(*(*it))[i] = 0;
	}

	for(LatenciesI it = m_latencies.begin(); it != m_latencies.end(); it++)
	{
		//leave node no without change
		(*it)->m_addr.Set("::");
		(*it)->m_interface = 0;
		(*it)->m_latencyInitTime = Seconds(-1.0);
		(*it)->m_latencyInvalidTime = Seconds(-1.0);
		(*it)->m_latencyPreferredTime = Seconds(-1.0);
	}

	m_first_mNStime = Seconds(-1.0);
	m_last_mNStime = Seconds(-1.0);
	m_first_mNDMessTime = Seconds(-1.0);
	m_last_mNDTtime = Seconds(-1.0);
}

/*void InternetStackHelper_Nd::ResetAverageStats()
{
	m_avarageResult.Reset();
}*/

} // namespace ns3
