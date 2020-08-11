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

NS_LOG_COMPONENT_DEFINE ("Ipv6PingManet");
//IPv6 ping is not working, because there is no echo reply handling implemented in ns-3!!!
int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 6;

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
	  LogComponentEnable("Ipv6PingManet", LOG_LEVEL_INFO);
	  LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_INFO);
	  LogComponentEnable ("Ipv6Interface", LOG_LEVEL_INFO);
	  LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_INFO);
	  LogComponentEnable ("Ipv6RawSocketImpl", LOG_LEVEL_INFO);
	  LogComponentEnable ("Ping6Application", LOG_LEVEL_INFO);
      //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }


  //create nodes:
  NodeContainer wifiNodes;
  wifiNodes.Create (nWifi);	//creates nWifi+1 nodes


 //create wifi network:
  WifiHelper wifi = WifiHelper::Default ();

  //specify network details:
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
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

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiNodes);


  InternetStackHelper internetv6;
  internetv6.SetIpv4StackInstall(false);
  internetv6.Install (wifiNodes);


  Ipv6AddressHelper address;
  Ipv6InterfaceContainer WifiInterfaces;
  //WifiInterfaces = address.Assign(wifiDevices, std::vector<bool> (nWifi, true));
  WifiInterfaces = address.Assign(wifiDevices);

  //create Ping6 app. to send ping6 from node nr 0 to all-nodes-multicast
  Ping6Helper ping6;

  ping6.SetIfIndex (WifiInterfaces.GetInterfaceIndex(0));
  ping6.SetLocal(WifiInterfaces.GetAddress(0, 0));
  ping6.SetRemote (Ipv6Address::GetAllNodesMulticast ());

  ping6.SetAttribute ("MaxPackets", UintegerValue (5));
  ping6.SetAttribute ("Interval", TimeValue (Seconds(1.0)));
  ping6.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer apps = ping6.Install(wifiNodes.Get(0));

  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));


  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (11.0));

  phy.EnablePcap ("/home/monika/Workspaces/COST_workspace/ns-3.11/pcaps/ipv6-ping", wifiDevices.Get (0), false, true);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
