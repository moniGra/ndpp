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


//
// 6-node MANET, no AP
//

using namespace ns3;

/*template <class charT, charT sep>
class punct_facet: public std::numpunct<charT> {
protected:
    charT do_decimal_point() const { return sep; }
};*/


int 
main (int argc, char *argv[])
{
	//set the script to output comma-sperated data
	//std::locale mylocale("");   // get global locale
	//std::cout.imbue(std::locale("en_GB.utf8"));  // imbue defined locale
	// std::imbue(std::locale(std::cout.getloc(), new punct_facet<char, ','>));


	NdppExperiment experiment;
	experiment.CommandSetup(argc,argv);

	experiment.InitSetup();

	experiment.Run();

  return 0;
}
