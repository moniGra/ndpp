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

#include "ipv6-ndpp-experiments.h"
//#include "ipv6-ndpp-resultsCollection.h"


//
// 6-node MANET, no AP
//

using namespace ns3;

//NS_LOG_COMPONENT_DEFINE ("Ipv6Nd");

/*static void
CourseChange (std::string context, Ptr<const MobilityModel> position)
{
  Vector pos = position->GetPosition ();
  NS_LOG_UNCOND(Simulator::Now () << ", pos=" << position << ", x=" << pos.x << ", y=" << pos.y
            << ", z=" << pos.z << std::endl);
}*/

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

void AddAddress(Ipv6InterfaceContainer* Ipv6Interfaces, NetDeviceContainer* NetDevices, uint32_t initiator)
{
	Ptr<NetDevice> device_init = NetDevices->Get(initiator);
	Ptr<Ipv6> ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();

	int32_t ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
	if (ifIndex_init == -1)
	{
	  ifIndex_init = ipv6_init->AddInterface (device_init);
	}

	Ipv6InterfaceAddress ipv6Addr = Ipv6InterfaceAddress (Ipv6Address("2002:0000:0000:0000:0200:00ff:fe00:000e"));
	ipv6_init->SetMetric (ifIndex_init, 1);
	ipv6_init->SetUp (ifIndex_init);
	ipv6_init->AddAddress (ifIndex_init, ipv6Addr);

	Ipv6Interfaces->Add (ipv6_init, ifIndex_init);
	NS_LOG_INFO ("Assign new IPv6 address " << ipv6Addr<< " to node " << initiator << ".");
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

void SetInterfacesDown (NetDeviceContainer* NetDevices)
{
	Ptr<NetDevice> device;
	Ptr<Ipv6> ipv6;
	int32_t ifIndex = 0;
	NS_ASSERT_MSG (ifIndex >= 0, "SetInterfacesDown(): Interface index not found");

	for (u_int32_t i = 0; i < NetDevices->GetN(); i++)
	{
		device = NetDevices->Get(i);
		ipv6 = device->GetNode()->GetObject<Ipv6> ();
		ifIndex = ipv6->GetInterfaceForDevice (device);
		ipv6->SetDown(ifIndex);
	}
}

int 
main (int argc, char *argv[])
{
  bool verbose = false;
  int nWifi = 30;
  int initiatorNr = 2;
//  int targetNr = 5;
  enum resultTypeId
  {
	  SingleRunFullTxRxTables = 0,
	  HopLimitTest
  };
  int resultType = HopLimitTest;


	enum ndTypeId
	{
		ND_BASIC = 0,
		ND_SIMPLE = 1,
		ND_NDPP = 2
	};
	int ndType = ND_NDPP;

	std::string resultFileSuffix = "";

	enum topologyTypeId
	{
		Grid = 0,
		CrossGrid = 1,
		RandomRectangle = 2,
		UniformDisc = 3,
		RandomDisc =4 ,
		RandomWalk = 5,
		RandomWaypoint = 6
	};
	int topologyType = Grid;

	enum phyWifiStdId
	{
	  Standard_802_11_a = 0,
	  Standard_802_11_b,
	  Standard_802_11_g
	};
	int wifiStd = Standard_802_11_g;

	enum wifiModeId
	{
		OfdmRate6Mbps,
		OfdmRate24Mbps,
		OfdmRate54Mbps,
		DsssRate2Mbps,
		DsssRate11Mbps
	};
	int wifiMode = OfdmRate54Mbps;

	uint32_t initialHopLimit = 4;
	int32_t maxHopLimit = 4;


  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi nodes", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
 // cmd.AddValue ("initiatorNr", "Selected node - DAD initiator", initiatorNr);
 // cmd.AddValue ("targetNr", "Select\"target\" of DAD - node nr from which we will assign the address to initiating node", targetNr);
  cmd.AddValue ("ndType", "Select ND mode (Basic=0, simple=1, ND++ = 2", ndType);
  cmd.AddValue ("initialHopLimit", "HopLimit value for ND++ to start with", initialHopLimit);
  cmd.AddValue ("maxHopLimit", "Max HopLimit value for simulation", maxHopLimit);
  cmd.AddValue ("resultType", "Type of expected result to be collected during simulation", resultType);
  cmd.AddValue ("resultFileSuffix", "Suffix to be added to result file name", resultFileSuffix);
  cmd.AddValue ("topology", "Topology type (0-Grid, 1-CrossGrid)", topologyType);
  cmd.AddValue ("wifiStd", "Wi-Fi standard (a,b,g))", wifiStd);
  cmd.AddValue ("wifiMode", "Wi-Fi Mode: modulation + bitrate", wifiMode);

  cmd.Parse (argc,argv);

  if (verbose)
  {
	  LogComponentEnable ("Ipv6Interface_Ndpp", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
	  LogComponentEnable ("Icmpv6L4Protocol_Ndpp", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
	  LogComponentEnable ("Icmpv6L4Protocol_NdSimple", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  }
  else
  {
	  LogComponentEnable ("Ipv6Interface_Ndpp", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
	  LogComponentEnable ("Icmpv6L4Protocol_Ndpp",(LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
	  LogComponentEnable ("Icmpv6L4Protocol_NdSimple", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  }

 // LogComponentEnable("Ipv6PingManet", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("Ipv6L3Protocol", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("Ipv6L3Protocol_Ndpp", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ERROR);

  LogComponentEnable ("Icmpv6L4Protocol", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));

  LogComponentEnable ("Ipv6RawSocketImpl", LOG_LEVEL_INFO);
  LogComponentEnable ("Ping6Application", LOG_LEVEL_INFO);
  LogComponentEnable ("InternetStackHelper", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("InternetStackHelper_Nd", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable("Ipv6Nd", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));

  //LogComponentEnable("Buffer", LogLevel(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  //LogComponentEnable("Packet", LogLevel(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
   //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
   //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable("Ipv6Nd", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));



  maxHopLimit = std::min((int) (nWifi * 1.5), maxHopLimit);

  NodeContainer* wifiNodes;
  WifiHelper* wifi;
  YansWifiChannelHelper* channel;
  YansWifiPhyHelper* phy;
  NqosWifiMacHelper* mac;
 // std::list<MobilityHelper> mobilities;
  std::list<Vector> topology;
  typedef std::list<Vector>::iterator TopologyI;
  MobilityHelper mobility;
  typedef std::list<MobilityHelper>::iterator MobilityHelpersI;
  NetDeviceContainer* wifiDevices;
  InternetStackHelper_Nd* internetv6;
  Ipv6InterfaceContainer* wifiInterfaces;

  ////////////////////////////////////
  //create wifi network:
  ///////////////////////////////////////

  //TODO: consider - to raczej nie ma wplywu, lepiej zostawic wieksze
  // disable fragmentation for frames below 2200 bytes
    //Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
   // Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));

  wifi = new WifiHelper();

  std::string phyMode;
  switch(wifiMode)
  {
  case OfdmRate6Mbps:
	  if(wifiStd == Standard_802_11_a)
		  phyMode = "OfdmRate6Mbps";
	  else
		  phyMode = "ErpOfdmRate6Mbps";
	  break;
  case OfdmRate24Mbps:
	  if(wifiStd == Standard_802_11_a)
		  phyMode = "OfdmRate24Mbps";
	  else
		  phyMode = "ErpOfdmRate24Mbps";
  	  break;
  case DsssRate2Mbps:
	  phyMode = "DsssRate2Mbps";
	  break;
  case DsssRate11Mbps:
  	  phyMode = "DsssRate11Mbps";
  	  break;
  case OfdmRate54Mbps:
  default:
	  if(wifiStd == Standard_802_11_a)
		  phyMode = "OfdmRate54Mbps";
	  else
		  phyMode = "ErpOfdmRate54Mbps";
  	  break;
  }

  switch(wifiStd)
  {
  case Standard_802_11_a:
	  wifi->SetStandard(WIFI_PHY_STANDARD_80211a);
	  break;
  case Standard_802_11_b:
  	  wifi->SetStandard(WIFI_PHY_STANDARD_80211b);
  	  break;
  case Standard_802_11_g:
  default:
  	  wifi->SetStandard(WIFI_PHY_STANDARD_80211g);
  	  break;
  }

  wifi->SetRemoteStationManager("ns3::ConstantRateWifiManager",
		  "NonUnicastMode", StringValue(phyMode),
		  "DataMode", StringValue(phyMode),
		  "ControlMode", StringValue(phyMode)/*,
		  "MaxSsrc", UintegerValue(20),
		  "MaxSlrc", UintegerValue(20)*/);

  Config::SetDefault ("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue (1000));

  //specify network details:
  double maxRange = 100.0;

  //do not use default here! - this results in having 2 propagation loss models acting simultaneously!
  channel = new YansWifiChannelHelper();
  channel->AddPropagationLoss("ns3::RangePropagationLossModel",
		  "MaxRange", DoubleValue(maxRange));
  channel->SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  phy = new YansWifiPhyHelper(YansWifiPhyHelper::Default ());
 // wifiPhy.Set ("RxGain", DoubleValue (0) );
  // wifiPhy.Set ("TxGain", DoubleValue (0) );

  Ssid ssid = Ssid ("monika");
  mac = new NqosWifiMacHelper(NqosWifiMacHelper::Default ());
  mac->SetType ("ns3::AdhocWifiMac",
			"Ssid", SsidValue (ssid));


  ///////////// Mobility //////////////////////////////////////////////////
  bool mobilityHierarchyInit = false;
  double distance = maxRange/std::sqrt(2)-5;

 /* for(int i=0; i<nWifi; i++)
  {
	  mobilities.push_back(MobilityHelper());	//in general we need one helper per node to allow re-creating
	  //topology in each run from hierarchical mobility model (will be different for each node - associated
	  //with different position*/
  switch(topologyType)
  {
	case CrossGrid:
	  NS_ASSERT_MSG(distance>maxRange/2, "CrossGrid is wrongly created!, maxRAnge value too small");
	  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
									  "MinX", DoubleValue (0.0),
									  "MinY", DoubleValue (0.0),
									  "DeltaX", DoubleValue (distance),
									  "DeltaY", DoubleValue (distance),
									  "GridWidth", UintegerValue ((int) ceil(sqrt(nWifi))),
									  "LayoutType", StringValue ("RowFirst"));
	  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  break;
	case RandomRectangle:
		mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
								  "X",  RandomVariableValue (UniformVariable (0.0, 350.0)),
								  "Y",  RandomVariableValue (UniformVariable (0.0, 350.0)));
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  break;
	case RandomDisc:
		mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
								  "X",  DoubleValue (0.0),
								  "Y",  DoubleValue (0.0),
								  "Theta",  RandomVariableValue (UniformVariable (0.0, 6.2830)),
								  "Rho",  RandomVariableValue (UniformVariable (0.0, 200.0)));
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		break;
	case UniformDisc:
		mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator",
									  "X",  DoubleValue (0.0),
									  "Y",  DoubleValue (0.0),
									  "rho",  DoubleValue (200.0));
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		break;
	case RandomWalk:
		mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
								   "Bounds", RectangleValue (Rectangle (0, 0, 200, 200)));
		break;
	case RandomWaypoint:
		mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel");
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
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  break;
  }

  //TODO: sprawdzic, czy kazda symulacja ma ta sama topologie!
  //TODO: srawdzic, czy wiadomosci multicast nie sa wysylane z wieksza moca!!!
  //TODO: sprawdzic dropy w warstwie phy

  Ipv6AddressHelper* addresses = new Ipv6AddressHelper();
 // addresses->NewNetwork(Ipv6Address("fe80::"), Ipv6Prefix(64));

  ResultsCollection results;
  results.EnableResultsCollection("/home/monika/Workspaces/COST_workspace/ns-3.11/simul_results/ipv6-nd"
		  , resultFileSuffix);


  //get input command for printing:
  std::string inputCommand = "";
  for(int i=0; i<argc; i++)
  {
	  inputCommand +=*argv;
	  argv++;
  }


	//.............................................................................
	//Simulation:
	//.............................................................................

  NS_LOG_UNCOND("Start ND demo");

  for(int hopLimit = initialHopLimit; hopLimit <= maxHopLimit; hopLimit += 2)
  {
	  NS_LOG_UNCOND("Run with HopLimit = " << hopLimit);

	  for(int j=0; j < nWifi/2; j++)
	  {
		  NS_LOG_UNCOND("Run no " << j << " with HopLimit = " << hopLimit);
		  for(int k=0; k<5; k++)
		  {
			  NS_LOG_UNCOND("Run no " << k << " with HopLimit = " << hopLimit << "for node: "
					  << initiatorNr*j);

			  //initial set-up:
			 // SeedManager::SetRun(1);

			  //create nodes:
			   wifiNodes = new NodeContainer();
			   wifiNodes->Create (nWifi);	//creates nWifi+1 nodes (??? - nwifi nodes)

			   phy->SetChannel (channel->Create ());

			   //create interfaces and link the interfaces for given nodes to the wifi network:
			   wifiDevices = new NetDeviceContainer(wifi->Install (*phy, *mac, *wifiNodes));

			  /* //trace topology changes - to checkif topology is the same for each run:
			   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
				   (*it)->TraceConnectWithoutContext("CourseChange", MakeCallback (&CourseChange));*/

			   mobility.Install(*wifiNodes);
			   Ptr<MobilityModel> model;
			   TopologyI it_top;
			   if(mobilityHierarchyInit)
			   {
				   NS_ASSERT_MSG(topology.size() == wifiNodes->GetN(), "Topology vector size different "
						   "form node container size!");
				   it_top = topology.begin();
				   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
				   {
					   model = (*it)->GetObject<MobilityModel> ();
					   model->SetPosition(*it_top);
					   it_top++;
				   }
			   }
			   else
			   {
				   mobilityHierarchyInit = true;
				   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
				   {
					   //remember initial topology setup for each node
					   model = (*it)->GetObject<MobilityModel> ();
					   topology.push_back(model->GetPosition());
				   }
			   }
			   internetv6 = new InternetStackHelper_Nd (ndType);
			   internetv6->SetIpv4StackInstall(false);
			   internetv6->InstallAll();
			   if(ndType==1 || ndType==2)
			   {
				   for (NodeContainer::Iterator i = wifiNodes->Begin(); i != wifiNodes->End(); ++i)
					   (*i)->GetObject<Ipv6L3Protocol>()->GetIcmpv6()->SetHopLimit(hopLimit);
			   }

				//wifiInterfaces = new Ipv6InterfaceContainer(addresses->Assign(*wifiDevices));	//this also sets interfaces up
			   wifiInterfaces = new Ipv6InterfaceContainer(addresses->Assign(*wifiDevices, std::vector<bool> (nWifi, false)));	//this also sets interfaces up


				internetv6->EnableAsciiIcmpv6All();


			  Simulator::Schedule(Seconds(15.0), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
			  Simulator::Schedule(Seconds(15.2), AddAddress, wifiInterfaces, wifiDevices, initiatorNr*j);
			  Simulator::Schedule(Seconds(30.0), SetInterfacesDown, wifiDevices);
			  Simulator::Stop (Seconds (33.0));

			  //main run
			  Simulator::Run ();

			  results.UpdateHopLimitStatsSingleRun(internetv6);
			  internetv6->DisableIcmpv6AsciiTracing();

			  /*//remember topology in the hierarchical model form:
			  if(mobilityHierarchyInit)
				  for(MobilityHelpersI it = mobilities.begin(); it != mobilities.end(); it++)
					  it->PopReferenceMobilityModel();
			  else
				  mobilityHierarchyInit = true;
			  l=0;
			  for(MobilityHelpersI it = mobilities.begin(); it != mobilities.end(); it++)
			  {
				  it->PushReferenceMobilityModel(wifiNodes->Get(l));	//remember old topology info for this node
				  it->SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
						  "X", RandomVariableValue (ConstantVariable (0.0)),
						  "Y", RandomVariableValue (ConstantVariable (0.0)));
				  //TODO: when working with mobility - check if it shouldn't be reset here!
				  l++;
			  }*/

			  Simulator::Destroy ();
			  //cleanup before next run
			  delete wifiInterfaces;
			  wifiInterfaces = NULL;
			  delete internetv6;
			  internetv6 = NULL;
			  delete wifiDevices;
			  wifiDevices = NULL;
			  delete wifiNodes;
			  wifiNodes = NULL;
		  }

		  results.UpdateHopLimitStats();
		  results.ResetAverageStatsSingleRun();
		  //internetv6->Reset();
		//  SetInterfacesDown(wifiDevices);

		  //update stats
		  //internetv6->ResetDadStats();
	  }
	  //Print results - one line for this hop limit
	  results.PrintResultHopLimit(inputCommand, hopLimit);
	  results.ResetStats();
  }

  //delete mobility;
  //mobility = NULL;
  delete mac;
  mac = NULL;
  delete phy;
  phy = NULL;
  delete channel;
  channel = NULL;
  delete wifi;
  wifi = NULL;
  delete addresses;
 // Simulator::Destroy ();

  return 0;
}
