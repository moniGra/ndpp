/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"


//
// 6-node MANET, no AP
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Ipv6Nd");

void AddAddress(Ipv6InterfaceContainer& Ipv6Interfaces, NetDeviceContainer& NetDevices, uint32_t initiator, uint32_t target)
{
	NS_LOG_INFO ("Assign IPv6 addresses of node " << target << " to node " << initiator << ".");
	Ptr<NetDevice> device_init = NetDevices.Get(initiator);
	Ptr<NetDevice> device_target = NetDevices.Get(target);

	Ptr<Ipv6> ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
	Ptr<Ipv6> ipv6_target = device_target->GetNode()->GetObject<Ipv6> ();

	int32_t ifIndex_init = 0;
	int32_t ifIndex_target = 0;
	ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
	if (ifIndex_init == -1)
	{
	  ifIndex_init = ipv6_init->AddInterface (device_init);
	}
	ifIndex_target = ipv6_target->GetInterfaceForDevice (device_target);
	if (ifIndex_target == -1)
	{
	  ifIndex_target = ipv6_init->AddInterface (device_target);
	}

	Ipv6InterfaceAddress ipv6IntAddr_target = ipv6_target->GetAddress(ifIndex_target, 0);
	//create new interface address from Ipv6Address of a target (otherwise other interface params are copied as well:( )
	Ipv6InterfaceAddress ipv6Addr = Ipv6InterfaceAddress (ipv6IntAddr_target.GetAddress());
	ipv6_init->SetMetric (ifIndex_init, 1);
	ipv6_init->SetUp (ifIndex_init);
	ipv6_init->AddAddress (ifIndex_init, ipv6Addr);

	Ipv6Interfaces.Add (ipv6_init, ifIndex_init);
}

void SetInterfacesUp (NetDeviceContainer& NetDevices)
{
	Ptr<NetDevice> device;
	Ptr<Ipv6> ipv6;
	int32_t ifIndex = 0;
	NS_ASSERT_MSG (ifIndex >= 0, "SetInterfacesUp(): Interface index not found");

	for (u_int32_t i = 0; i < NetDevices.GetN(); i++)
	{
		device = NetDevices.Get(i);
		ipv6 = device->GetNode()->GetObject<Ipv6> ();
		ifIndex = ipv6->GetInterfaceForDevice (device);
		ipv6->SetUp(ifIndex);
	}
}

void SetInterfacesDown (NetDeviceContainer& NetDevices)
{
	Ptr<NetDevice> device;
	Ptr<Ipv6> ipv6;
	int32_t ifIndex = 0;
	NS_ASSERT_MSG (ifIndex >= 0, "SetInterfacesUp(): Interface index not found");

	for (u_int32_t i = 0; i < NetDevices.GetN(); i++)
	{
		device = NetDevices.Get(i);
		ipv6 = device->GetNode()->GetObject<Ipv6> ();
		ifIndex = ipv6->GetInterfaceForDevice (device);
		ipv6->SetDown(ifIndex);
	}
}

int 
main (int argc, char *argv[])
{
  bool verbose = false;
  uint32_t nWifi = 20;
  int initiatorNr = 2;
  int targetNr = 5;
  enum resultTypeId
  {
	  SingleRunFullTxRxTables = 0
  };
  int resultType = SingleRunFullTxRxTables;


	enum ndTypeId
	{
		ND_BASIC = 0,
		ND_SIMPLE = 1,
		ND_NDPP = 2
	};
	int ndType = ND_BASIC;

	u_int32_t hopLimit = 3;
	std::string resultFileSuffix = "";

	enum topologyTypeId
	{
		Grid = 0,
		CrossGrid
	};
	int topologyType = Grid;


  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi nodes", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("initiatorNr", "Selected node - DAD initiator", initiatorNr);
  cmd.AddValue ("targetNr", "Select\"target\" of DAD - node nr from which we will assign the address to initiating node", targetNr);
  cmd.AddValue ("ndType", "Select ND mode (Basic=0, simple=1, ND++ = 2", ndType);
  cmd.AddValue ("hopLimit", "HopLimit value for ND++", hopLimit);
  cmd.AddValue ("resultType", "Type of expected result to be collected during simulation", resultType);
  cmd.AddValue ("resultFileSuffix", "Suffix to be added to result file name", resultFileSuffix);
  cmd.AddValue ("topology", "Topology type (0-Grid, 1-CrossGrid)", topologyType);

  cmd.Parse (argc,argv);

  if (verbose)
  {
	  LogComponentEnable ("Ipv6Interface_Ndpp", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
	  LogComponentEnable ("Icmpv6L4Protocol_Ndpp", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
	  LogComponentEnable ("Icmpv6L4Protocol_NdSimple", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  }
  else
  {
	  LogComponentEnable ("Ipv6Interface_Ndpp", LOG_LEVEL_INFO);
	  LogComponentEnable ("Icmpv6L4Protocol_Ndpp", LOG_LEVEL_INFO);
	  LogComponentEnable ("Icmpv6L4Protocol_NdSimple", LOG_LEVEL_INFO);
  }

  //LogComponentEnable("Ipv6PingManet", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipv6L3Protocol_Ndpp", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipv6Interface", LOG_LEVEL_INFO);

  LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_INFO);

  LogComponentEnable ("Ipv6RawSocketImpl", LOG_LEVEL_INFO);
  LogComponentEnable ("Ping6Application", LOG_LEVEL_INFO);
  LogComponentEnable ("InternetStackHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("InternetStackHelper_Nd", LOG_LEVEL_INFO);
  LogComponentEnable("Ipv6Nd", LOG_LEVEL_INFO);

  //LogComponentEnable("Buffer", LogLevel(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  //LogComponentEnable("Packet", LogLevel(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
   //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
   //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  //create nodes:
  NodeContainer wifiNodes;
  wifiNodes.Create (nWifi);	//creates nWifi+1 nodes


 //create wifi network:
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard(WIFI_PHY_STANDARD_80211g);

  //specify network details:
  double maxRange = 100.0;
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  channel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(maxRange));
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  Ssid ssid = Ssid ("monika");
  mac.SetType ("ns3::AdhocWifiMac",
               "Ssid", SsidValue (ssid));

  //create interfaces and link the interfaces for given nodes to the wifi network:
  NetDeviceContainer wifiDevices;
  wifiDevices = wifi.Install (phy, mac, wifiNodes);


  MobilityHelper mobility;

  double distance;
  switch(topologyType)
  {
  case CrossGrid:
	  distance = maxRange/std::sqrt(2)-5;
	  NS_ASSERT_MSG(distance>maxRange/2, "CrossGrid is wrongly created!, maxRAnge value too small");
	  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
	                                  "MinX", DoubleValue (0.0),
	                                  "MinY", DoubleValue (0.0),
	                                  "DeltaX", DoubleValue (distance),
	                                  "DeltaY", DoubleValue (distance),
	                                  "GridWidth", UintegerValue ((int) ceil(sqrt(nWifi))),
	                                  "LayoutType", StringValue ("RowFirst"));
	  break;
  case Grid:
  default:
	  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
	                                  "MinX", DoubleValue (0.0),
	                                  "MinY", DoubleValue (0.0),
	                                  "DeltaX", DoubleValue (maxRange-5),
	                                  "DeltaY", DoubleValue (maxRange-5),
	                                  "GridWidth", UintegerValue ((int) ceil(sqrt(nWifi))),
	                                  "LayoutType", StringValue ("RowFirst"));
	  break;

  }

  /*mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));*/
  /*mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
		  	  	  	  	  	  "X",  RandomVariableValue (UniformVariable (0.0, 200.0)),
		  	  	  	  	  	  "Y",  RandomVariableValue (UniformVariable (0.0, 200.0)));*/
  mobility.Install (wifiNodes);



  InternetStackHelper_Nd* internetv6 = new InternetStackHelper_Nd (ndType);
  internetv6->SetIpv4StackInstall(false);
  internetv6->InstallAll();
  if(ndType==1 || ndType==2)
  {
	  Ptr<Node> node;
	  for (NodeContainer::Iterator i = wifiNodes.Begin(); i != wifiNodes.End(); ++i)
	  {
			  Ptr<Icmpv6L4Protocol> icmpv6 = (*i)->GetObject<Ipv6L3Protocol>()->GetIcmpv6();
			  icmpv6->SetHopLimit(hopLimit);
	  }
  }



  Ipv6AddressHelper addresses;
  Ipv6InterfaceContainer wifiInterfaces;
  wifiInterfaces = addresses.Assign(wifiDevices, std::vector<bool> (nWifi, false));	//this way only link-local addresses are assigned
 // wifiInterfaces = addresses.Assign(wifiDevices);
 //addresses.Assign(wifiDevices);


  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Schedule(Seconds(7.0), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
  Simulator::Schedule(Seconds(7.2), AddAddress, wifiInterfaces, wifiDevices, initiatorNr, targetNr);

 // Simulator::Schedule(Seconds(5.25), AddAddress, wifiInterfaces, wifiDevices, 5, nodeAddr);

  Simulator::Stop (Seconds (20.0));

  //phy.EnablePcap ("/home/monika/Workspaces/COST_workspace/ns-3.11/pcaps/ipv6-nd", wifiDevices.Get (0), false, true);
 /*for(NetDeviceContainer::Iterator it=wifiDevices.Begin(); it < wifiDevices.End(); it++)
 {
	 phy.EnablePcap("/home/monika/Workspaces/COST_workspace/ns-3.11/pcaps/ipv6-nd-pcap", *it, false, true);
 }*/
  phy.EnablePcapAll ("/home/monika/Workspaces/COST_workspace/ns-3.11/pcaps/ipv6-nd", true);
 // internetv6->EnablePcapIpv6All ("/home/monika/Workspaces/COST_workspace/ns-3.11/pcaps/ipv6-nd");
 // internetv6->EnableAsciiIpv6All ("/home/monika/Workspaces/COST_workspace/ns-3.11/ascii/ipv6-nd-ascii");
  internetv6->EnableAsciiIcmpv6All("/home/monika/Workspaces/COST_workspace/ns-3.11/simul_results/ipv6-nd", resultFileSuffix);
  //phy.EnableAsciiAll ("/home/monika/Workspaces/COST_workspace/ns-3.11/ascii/ipv6-nd-ascii");

  NS_LOG_UNCOND("Start ND demo");
  Simulator::Run ();
  Simulator::Destroy ();

  //collect data:
  std::string inputCommand = "";
  for(int i=0; i<argc; i++)
  {
	  inputCommand +=*argv;
	  argv++;
  }

  internetv6->PrintResultsTxRxStats(inputCommand);

  return 0;
}
