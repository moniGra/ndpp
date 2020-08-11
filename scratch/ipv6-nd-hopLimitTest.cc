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

//#include "ipv6-ndpp-resultsCollection.h"
#include "ipv6-ndpp-experiments.h"


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
  bool m_verbose = false;
  int m_nWifi = 30;
  int m_initiatorNr = 2;
//  int targetNr = 5;
  enum resultTypeId
  {
	  SingleRunFullTxRxTables = 0,
	  HopLimitTest
  };
  int m_resultType = HopLimitTest;


	enum ndTypeId
	{
		ND_BASIC = 0,
		ND_SIMPLE = 1,
		ND_NDPP = 2
	};
	int m_ndType = ND_NDPP;

	std::string m_resultFileSuffix = "";

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
	int m_topologyType = Grid;

	enum phyWifiStdId
	{
	  Standard_802_11_a = 0,
	  Standard_802_11_b,
	  Standard_802_11_g,
	  Standard_802_11_n
	};
	int m_wifiStd = Standard_802_11_g;

	enum wifiModeId
	{
		OfdmRate6Mbps,
		OfdmRate24Mbps,
		OfdmRate54Mbps,
		DsssRate2Mbps,
		DsssRate11Mbps
	};
	int m_wifiMode = OfdmRate54Mbps;

	uint32_t m_initialHopLimit = 4;
	int32_t m_maxHopLimit = 4;


  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi nodes", m_nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", m_verbose);
 // cmd.AddValue ("initiatorNr", "Selected node - DAD initiator", initiatorNr);
 // cmd.AddValue ("targetNr", "Select\"target\" of DAD - node nr from which we will assign the address to initiating node", targetNr);
  cmd.AddValue ("ndType", "Select ND mode (Basic=0, simple=1, ND++ = 2", m_ndType);
  cmd.AddValue ("m_initialHopLimit", "HopLimit value for ND++ to start with", m_initialHopLimit);
  cmd.AddValue ("m_maxHopLimit", "Max HopLimit value for simulation", m_maxHopLimit);
  cmd.AddValue ("resultType", "Type of expected result to be collected during simulation", m_resultType);
  cmd.AddValue ("resultFileSuffix", "Suffix to be added to result file name", m_resultFileSuffix);
  cmd.AddValue ("topology", "Topology type (0-Grid, 1-CrossGrid)", m_topologyType);
  cmd.AddValue ("wifiStd", "Wi-Fi standard (a,b,g,n))", m_wifiStd);
  cmd.AddValue ("m_wifiMode", "Wi-Fi Mode: modulation + bitrate", m_wifiMode);

  cmd.Parse (argc,argv);

  if (m_verbose)
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

//  LogComponentEnable("Ipv6PingManet", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("Ipv6L3Protocol", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("Ipv6L3Protocol_Ndpp", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ERROR);

  LogComponentEnable ("Icmpv6L4Protocol", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));

  LogComponentEnable ("Ipv6RawSocketImpl", LOG_LEVEL_INFO);
  LogComponentEnable ("Ping6Application", LOG_LEVEL_INFO);
  LogComponentEnable ("InternetStackHelper", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("InternetStackHelper_Nd", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable("Ipv6Nd", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  LogComponentEnable ("InternetTraceHelper", LOG_LEVEL_INFO);
  //LogComponentEnable("Buffer", LogLevel(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  //LogComponentEnable("Packet", LogLevel(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
   //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
   //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  //LogComponentEnable("Ipv6Nd", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));



  m_maxHopLimit = std::min((int) (m_nWifi * 1.5), m_maxHopLimit);

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
  switch(m_wifiMode)
  {
  case OfdmRate6Mbps:
	  if(m_wifiStd == Standard_802_11_a)
		  phyMode = "OfdmRate6Mbps";
	  else
		  phyMode = "ErpOfdmRate6Mbps";
	  break;
  case OfdmRate24Mbps:
	  if(m_wifiStd == Standard_802_11_a)
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
	  if(m_wifiStd == Standard_802_11_a)
		  phyMode = "OfdmRate54Mbps";
	  else
		  phyMode = "ErpOfdmRate54Mbps";
  	  break;
  }

  switch(m_wifiStd)
  {
  case Standard_802_11_a:
	  wifi->SetStandard(WIFI_PHY_STANDARD_80211a);
	  break;
  case Standard_802_11_b:
  	  wifi->SetStandard(WIFI_PHY_STANDARD_80211b);
  	  break;
  case Standard_802_11_n:
	  wifi->SetStandard(WIFI_PHY_STANDARD_80211n_2_4GHZ);
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

  Config::SetDefault ("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue (1000));	//default - 400

  //*****************specify network details:
  double maxRange = 100.0;

  //do not use default here! - this results in having 2 propagation loss models acting simultaneously (3.11)!
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

  ObjectFactory pos;
  Ptr<PositionAllocator> positionAlloc;
 /* for(int i=0; i<nWifi; i++)
  {
	  mobilities.push_back(MobilityHelper());	//in general we need one helper per node to allow re-creating
	  //topology in each run from hierarchical mobility model (will be different for each node - associated
	  //with different position*/

  switch(m_topologyType)
  {
	case CrossGrid:
	  NS_ASSERT_MSG(distance>maxRange/2, "CrossGrid is wrongly created!, maxRAnge value too small");
	  pos.SetTypeId("ns3::GridPositionAllocator");
	  pos.Set("MinX", DoubleValue (0.0));
	  pos.Set("MinY", DoubleValue (0.0));
	  pos.Set("DeltaX", DoubleValue (distance));
	  pos.Set("DeltaY", DoubleValue (distance));
	  pos.Set("GridWidth", UintegerValue ((int) ceil(sqrt(m_nWifi))));
	  pos.Set("LayoutType", StringValue ("RowFirst"));
	  positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
	  mobility.SetPositionAllocator (positionAlloc);

	  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  break;
	case RandomRectangle:
		pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
		pos.Set("X",  RandomVariableValue (UniformVariable (0.0, 350.0)));
		pos.Set( "Y",  RandomVariableValue (UniformVariable (0.0, 350.0)));
		positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
		mobility.SetPositionAllocator (positionAlloc);

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  break;
	case RandomDisc:
		pos.SetTypeId("ns3::RandomDiscPositionAllocator");
		pos.Set("X",  DoubleValue (0.0));
		pos.Set( "Y",  DoubleValue (0.0));
		pos.Set("Theta",  RandomVariableValue (UniformVariable (0.0, 6.2830)));
		pos.Set("Rho",  RandomVariableValue (UniformVariable (0.0, 200.0)));
		positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
		mobility.SetPositionAllocator (positionAlloc);

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		break;
	case UniformDisc:
		pos.SetTypeId("ns3::UniformDiscPositionAllocator");
		pos.Set("X",  DoubleValue (0.0));
		pos.Set("Y",  DoubleValue (0.0));
		pos.Set( "rho",  DoubleValue (200.0));
		positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
		mobility.SetPositionAllocator (positionAlloc);

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		break;
	case RandomWalk:
		mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
								   "Bounds", RectangleValue (Rectangle (0, 0, 200, 200)));
		//TODO: some additonal params may need to be specified
		break;
	case RandomWaypoint:
		mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel");
		//TODO: this has to be updated to specify position allocator for this mobility model
		 /*ObjectFactory pos;
		  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
		  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
		  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

		  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
		  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

		  std::stringstream ssSpeed;
		  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
		  std::stringstream ssPause;
		  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
		  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
		                                  "Speed", StringValue (ssSpeed.str ()),
		                                  "Pause", StringValue (ssPause.str ()),
		                                  "PositionAllocator", PointerValue (taPositionAlloc));
		  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);	//to jest chyab niepotrzebne
		  */
		break;
	case Grid:
	default:
		pos.SetTypeId("ns3::GridPositionAllocator");
		pos.Set("MinX", DoubleValue (0.0));
		pos.Set("MinY", DoubleValue (0.0));
		pos.Set("DeltaX", DoubleValue (maxRange-5));
		pos.Set( "DeltaY", DoubleValue (maxRange-5));
		pos.Set("GridWidth", UintegerValue ((int) ceil(sqrt(m_nWifi))));
		pos.Set("LayoutType", StringValue ("RowFirst"));
		positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
		mobility.SetPositionAllocator (positionAlloc);

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  break;
  }

  //TODO: sprawdzic, czy kazda symulacja ma ta sama topologie! -> trzeba o to zadbac
  //TODO: srawdzic, czy wiadomosci multicast nie sa wysylane z wieksza moca!!! - done - ustawienie remotae station manager
  //TODO: sprawdzic dropy w warstwie phy

  Ipv6AddressHelper* addresses = new Ipv6AddressHelper();
 // addresses->NewNetwork(Ipv6Address("fe80::"), Ipv6Prefix(64));

  ResultsCollection results;
  results.EnableResultsCollection("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/simul_results/ipv6-nd"
		  , m_resultFileSuffix);


  //get input command for printing:
  std::string inputCommand = "";
  for(int i=0; i<argc; i++)
  {
	  inputCommand +=*argv;
	  argv++;
  }

  //control RNG streams:
  int64_t streamsUsed;

	//.............................................................................
	//Simulation:
	//.............................................................................

  NS_LOG_UNCOND("Start ND demo");

  if (m_ndType == 0)
	  return -1;
  for(int hopLimit = m_initialHopLimit; hopLimit <= m_maxHopLimit; hopLimit += 2)
  {
	  NS_LOG_UNCOND("Run with HopLimit = " << hopLimit);

	  //RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
	  	  	  	  	  	  	  	  	 //puli niezaleznych liczb losowych

	  for(int j=0; j < m_nWifi/2; j++)
	  {
		  NS_LOG_UNCOND("Run no " << j << " with HopLimit = " << hopLimit);
		  for(int k=0; k<5; k++)
		  {
			  NS_LOG_UNCOND("Run no " << k << " with HopLimit = " << hopLimit << "for node: "
					  << m_initiatorNr*j);

			  //set HopLimit for this run:
			  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (hopLimit));

			  //reset streams
			  streamsUsed = 0;

			  //*********************create nodes*********************:
			   wifiNodes = new NodeContainer();
			   wifiNodes->Create (m_nWifi);	//creates nWifi+1 nodes (??? - nwifi nodes)


			   //*********************** create channel:
			   Ptr<YansWifiChannel> chan = channel->Create ();
			   phy->SetChannel(chan);
			   streamsUsed += channel->AssignStreams (chan, streamsUsed);


			   //********create interfaces and link the interfaces for given nodes to the wifi network:
			   wifiDevices = new NetDeviceContainer(wifi->Install (*phy, *mac, *wifiNodes));

			   // Assign fixed stream numbers to wifi and channel random variables
			  streamsUsed += wifi->AssignStreams (*wifiDevices, streamsUsed);	//blocks PHY, Station managers, MAC

			  /* //trace topology changes - to checkif topology is the same for each run:
			   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
				   (*it)->TraceConnectWithoutContext("CourseChange", MakeCallback (&CourseChange));*/


			  //**********************mobility & topology:
			   streamsUsed += positionAlloc->AssignStreams (streamsUsed);
			   mobility.Install(*wifiNodes);
			   //// Assign fixed stream numbers to mobility model, position allocator has to be handled separately
			   streamsUsed += mobility.AssignStreams (*wifiNodes, streamsUsed);

			   Ptr<MobilityModel> model;
			   TopologyI it_top;
			   if(mobilityHierarchyInit)
			   {
				   NS_ASSERT_MSG(topology.size() == wifiNodes->GetN(), "Topology vector size different "
						   "from node container size!");
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


			   //*********************internet stack:
			   internetv6 = new InternetStackHelper_Nd (m_ndType);
			   internetv6->SetIpv4StackInstall(false);
			   internetv6->InstallAll();
			  // internetv6->Install(*wifiNodes);	///moze tak by bylo lepiej???

			   //fix IPv6EXtensions and ICMPv6L4Protocol streams and IPv6Interface (dadDelay and MPR Start delay)
			   //(plus others most likely not used in my simulations)
			   streamsUsed += internetv6->AssignStreams (*wifiNodes, streamsUsed);

			   internetv6->EnableAsciiIcmpv6All();	//this has to be done before address assignment
			   	   	   	   	   	   	   	   	   	   //otherwise first MPR messages are not counted

			   wifiInterfaces = new Ipv6InterfaceContainer(addresses->Assign(*wifiDevices, std::vector<bool> (m_nWifi, false)));	//this also sets interfaces up
			   //UWAGA!!! - po wywolaniu powyzej startuja MPRy (MPRsInit) - to jest ok, schedule wysylanai odpalany po Simulator::Run


			   //monitor if random numbers are deterministic throughout simulation
			  NS_ASSERT_MSG(streamsUsed < ((1LL)<<62)-100LL, "No. of RNG streams close to end, fix RNs!!!!");

			   //****************************simulator setup:
			  Simulator::Schedule(Seconds(15.0), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
			  Simulator::Schedule(Seconds(15.2), AddAddress, wifiInterfaces, wifiDevices, m_initiatorNr*j);
			  Simulator::Schedule(Seconds(30.0), SetInterfacesDown, wifiDevices);
			  Simulator::Stop (Seconds (33.0));

			  //*********************main run:
			  Simulator::Run ();

			  //*************************results collection:
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
			  //*********************cleanup before next run:
			  delete wifiInterfaces;
			  wifiInterfaces = NULL;
			  delete internetv6;
			  internetv6 = NULL;
			  delete wifiDevices;
			  wifiDevices = NULL;
			  delete wifiNodes;
			  wifiNodes = NULL;
		  }

		  //***********************cumulative results collection:
		  results.UpdateHopLimitStats();
		  results.ResetAverageStatsSingleRun();
		  //internetv6->Reset();
		//  SetInterfacesDown(wifiDevices);

		  //update stats
		  //internetv6->ResetDadStats();
	  }
	  //***************************Print results - one line for this hop limit
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
