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
#include "ns3/uinteger.h"
#include "ns3/vector.h"
#include "ns3/callback.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/object-vector.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"

#include "loopback-net-device.h"
#include "ipv6-l3-protocol.h"
#include "ip-l4-protocol.h"
#include "ipv6-interface.h"
#include "ipv6-raw-socket-impl.h"
#include "ipv6-autoconfigured-prefix.h"
#include "ipv6-extension-demux.h"
#include "ipv6-extension.h"
#include "ipv6-extension-header.h"
#include "ipv6-option-demux.h"
#include "ipv6-option.h"
#include "icmpv6-l4-protocol.h"
#include "ndisc-cache.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6L3Protocol_Ndpp);

NS_LOG_COMPONENT_DEFINE ("Ipv6L3Protocol_Ndpp");


TypeId Ipv6L3Protocol_Ndpp::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6L3Protocol_Ndpp")
    .SetParent<Ipv6L3Protocol> ()
    .AddConstructor<Ipv6L3Protocol_Ndpp> ()
  ;
  return tid;
}

Ipv6L3Protocol_Ndpp::Ipv6L3Protocol_Ndpp()
{
  NS_LOG_FUNCTION_NOARGS ();
}

Ipv6L3Protocol_Ndpp::~Ipv6L3Protocol_Ndpp()
{
  NS_LOG_FUNCTION_NOARGS ();
}


uint32_t Ipv6L3Protocol_Ndpp::AddInterface (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  Ptr<Node> node = GetObject<Node> ();
  Ptr<Ipv6Interface> interface = CreateObject<Ipv6Interface_Ndpp> ();

  node->RegisterProtocolHandler (MakeCallback (&Ipv6L3Protocol::Receive, this), Ipv6L3Protocol::PROT_NUMBER, device);
  interface->SetNode (m_node);
  interface->SetDevice (device);
  interface->SetForwarding (m_ipForward);
  return AddIpv6Interface (interface);
}

} /* namespace ns3 */

