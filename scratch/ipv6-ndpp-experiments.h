/*
 * ipv6-ndpp-experiments.h
 *
 *  Created on: May 22, 2014
 *      Author: monika
 */
#ifndef IPV6_NDPP_EXPERIMENTS_H_
#define IPV6_NDPP_EXPERIMENTS_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include <string>
#include <iostream>
//#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("Ipv6Nd");

class InternetStackHelper_Nd;
typedef std::list<int>::const_iterator ListIntCI;

class DuplInfo
{
public:
	DuplInfo(Ipv6Address addr, int target) : m_addr(addr), m_target(target)	{}
	~DuplInfo(){}

	Ipv6Address m_addr;
	int m_target;	//node number for which the address belongs to
};


class ResultsCollection
{
public:
	ResultsCollection();
	~ResultsCollection();

	void PrintResult(int resultType, std::string additionalInfo, uint32_t hopLimit, int noOfRuns);

	/*
	 * this function is assumed to print several points of data for the hop limit experiments
	 * in per network set-ups
	 */
	 void PrintResultHopLimit(std::string additionalInfo, uint32_t hopLimit, int noOfRuns = -1, bool printTotal = false);
	 /*
	 * this function is assumed to print several (or one) points of data for the hop limit experiments
	 * in per node set-ups
	 */
	 void PrintIntroMultiExp(std::string additionalInfo);

	 void PrintPartialResultHLmNSSingleRun(std::string additionalInfo, uint32_t hopLimit, int noOfRuns=-1, std::string lineEnd = "\n");
	 void PrintPartialResultHLTotalSingleRun(std::string additionalInfo, uint32_t hopLimit, int noOfRuns=-1, std::string lineEnd = "\n");

	 void PrintPartialResultDupProbability(std::string additionalInfo, uint32_t hopLimit, int count, std::string lineEnd = "\n");
	 void PrintResultDupProbability(std::string additionalInfo, uint32_t hopLimit);
	 void PrintPartialResultLatencyRN(std::string additionalInfo, std::string lineEnd = "\n");
 	 void PrintResultLatencyRN(std::string additionalInfo);
	 void PrintPartialResultMlifeSingleRun(std::string additionalInfo, uint32_t hopLimit, int noOfRuns,
			 std::string lineEnd = "\n", bool printTotal = false, std::string midSign = "");
	 void PrintResultMessageLifetime(std::string additionalInfo, uint32_t hopLimit, int noOfRuns=-1, bool printTotal=false);
	 void UpdateHopLimitStats();
	 void UpdateHopLimitStatsSingleRun(InternetStackHelper_Nd* internetHelper, bool updatemNS=true, bool updateTot=false);
	 void UpdateMLifeStatsSingleRun(InternetStackHelper_Nd* internetHelper, bool updatemNS=true, bool updateTot=false);
	 void UpdateMessageLifetimeStats();
	 void UpdateLatencyStatsRNSingleRun(InternetStackHelper_Nd* internetHelper);
	 void UpdateLatencyStatsRN();
	 void UpdateDupProbStats(int noOfRuns);
	 void UpdateDupDetectCount(InternetStackHelper_Nd* internetHelper, uint32_t nodeNo);
	 void UpdateDupDetectCountRandomNodes(InternetStackHelper_Nd* internetHelper, int noOfDuplNodes);
	 int GetDupDetectCount();
	 void ResetStats();
	 void ResetAverageStatsSingleRun();
	 void EnableResultsCollection(std::string prefix, std::string sufix);

	 void SetConfidenceIntHL(double confidenceInt);
	 void SetAccuracyHL(double currAccuracy);
	 void SetConfidenceIntHLTot(double confidenceInt);
	 void SetAccuracyHLTot(double currAccuracy);
	 void SetConfidenceIntML(double confidenceInt);
	 void SetAccuracyML(double currAccuracy);
	 void SetConfidenceIntMLTot(double confidenceInt);
	 void SetAccuracyMLTot(double currAccuracy);
	 void SetRandomNodes (std::list<std::pair<int, Ipv6Address>> & selectedNodes, std::list<std::pair<int, DuplInfo>> & selectedDuplNodes);	//keep track of selected nodes)

	 void ResetResultPrintInit();
	 Average<int> m_average_mNSolCount_singleRun;
	 Average<int> m_average_mNDMessCount_singleRun;	//NS + NA
	 Average<int64_t> m_average_mNSlife_singleRun;	//in milliseconds!!!
	 Average<int64_t> m_average_mNDMessLife_singleRun;	//in milliseconds!!!

	 int m_noOfRunsDup;
	 int m_noOfRunsMLmNS;
	 int m_noOfRunsMLTot;
	 int m_noOfRunsHLmNS;
	 int m_noOfRunsHLTot;
	 //int noOfRunsLat;


	 class DupProbInfo
	 {
	 public:
		 DupProbInfo(Ipv6Address addr, int target, int dupCount) :
			 m_addr(addr), m_target(target), m_dupCount(dupCount){}
	 	~DupProbInfo(){}

	 	Ipv6Address m_addr;
	 	int m_target;	//node number for which the address belongs to
	 	int m_dupCount;	//how many times there was a hit for this target-initiator pair (duplication detected)
	 };

	 class AvgMinMax
	 {
	 public:
		 AvgMinMax(){}
		 ~AvgMinMax(){}

		 Average<double> m_averageResult;
		 Average<double> m_minResult;
		 Average<double> m_maxResult;

		 void Reset()
		 {
			 m_averageResult.Reset();
			 m_minResult.Reset();
			 m_maxResult.Reset();
		 }
		 double Min() {
			 return (m_minResult.Count())? m_minResult.Avg() : m_averageResult.Min();}
		 double Max() {
		 	return (m_maxResult.Count())? m_maxResult.Avg() : m_averageResult.Max();}
		 double Avg() {
		 	return m_averageResult.Avg();}
		 double MinMin() {
			 return m_minResult.Min();}
		 double MaxMax() {
			 return m_maxResult.Max();}
	  };

	 class LatencyInfo
	 {
	 public:
		 LatencyInfo() : m_duplAddress(-1) {}

		 bool m_duplAddress;
		 AvgMinMax m_averageLatencyINV;	//in case of address query resolved as INVALID
		 	 	 	 	 	 	 	 	 	 //NOTE: same node can have value for latency INV or
		 	 	 	 	 	 	 	 	 	 //latency PREF - but for DIFFERENT ADDRESSES (new auto address is on)
		 AvgMinMax m_averageLatencyPREF;	//in case of address query resolved as PREFERRED
		 AvgMinMax m_averageLatency;	//in case of address query resolved as either PREFERRED (if finished this way)
		 	 	 	 	 	 	 	 	 	 //OR INVALID if preferred state was never reached
		 AvgMinMax m_averageLatencyDupl;	//in case of address query for duplicated address
		 AvgMinMax m_averageLatencyNoDupl;	//in case of address query for not duplicated address
	 };

private:
	bool m_resultPrintInit;
	Ptr<OutputStreamWrapper> m_outputStream;

	AvgMinMax m_averageResult_mNSCount;
	AvgMinMax m_averageResult_mNDMessCount;
	AvgMinMax m_averageResult_mNSLife;
	AvgMinMax m_averageResult_mNDMessLife;

	Average<double> m_averageDupProbAll;
	Average<double> m_averageDupProb90;
	Average<double> m_averageDupProb75;
	Average<double> m_averageDupProb50;

	AvgMinMax m_averageDupProbPerNode;

	int m_dupDetectCount;	//for the specified target node only!!!
	int m_dupDetectCountAll;	//for simulations with random nodes - number of times all new nodes detected a dupication
	int m_dupDetectCount90;	//for simulations with random nodes - number of times at least 90% of new nodes detected a dupication
	int m_dupDetectCount75;	//for simulations with random nodes - number of times at least 75% of new nodes detected a dupication
	int m_dupDetectCount50;	//for simulations with random nodes - number of times at least 50% of new nodes detected a dupication
	double m_currAccuracyHLmNS;
	double m_currAccuracyHLTot;
	double m_currAccuracyMLmNS;
	double m_currAccuracyMLTot;
	double m_confidenceIntHLmNS;
	double m_confidenceIntHLTot;
	double m_confidenceIntMLmNS;
	double m_confidenceIntMLTot;
	std::list<int> m_selectedRandomNodes;	//keep track of selected nodes
	std::map<int,DupProbInfo> m_selectedDuplNodes;	//keep track of selected duplicated nodes

	typedef std::map<int, LatencyInfo >::iterator LatencyI;
	std::map<int, LatencyInfo > m_averageLatencyPerNode;	//in microseconds!!!; int to nr kolejnych wezlow w ramach jednego losowania
	std::list<LatencyInfo > m_averageLatencyPerRNSelection;	//in microseconds!!!; int to nr kolejnych zestawow losowanych wezlow
	LatencyInfo m_averageLatencyTotal;	//srednia po wszystkich losowaniach

};

class NdppExperiment
{
public:
  NdppExperiment ();
  ~NdppExperiment ();

  void CommandSetup (int argc, char **argv);
  void InitSetup();
  void Run ();
  void DoRunHopLimit();
  void DoRunHopLimitSelectedNode();
  void DoRunHopLimitSelectedNodeLong();
  void DoRunRandomNodes();
  void DoRunHopLimitRandomNodes();
  //for dup detection experiment specify target node no, otherwise use -1
  void DoRunSelectedNodeSingleRun(uint32_t currentNode, int32_t targetNode, bool resetStreams);
  int DoRunSelectedNodeSingleRunLong(uint32_t currentNode, int32_t targetNode, bool resetStreams,
		  bool controlNoOfRuns = false);
 // void DoRunRandomNodesSingleRun(int32_t targetNode, bool resetStreams, bool controlNoOfRuns = false);
  int DoRunRandomNodesSingleRun(bool resetStreams, bool controlNoOfRuns = false);
  int DoRunRNSingleRunMultiExp(bool resetStreams);
  void DoRunSelectedNodeSingleRunExp();
  void DoRunDupDetectionExp();
  void DoRunDupProbability();
  void DoRunDupProbRandomNode();
  void DoRunMultiExpRandomNodes();


private:
   void SelectRandomNodes(NetDeviceContainer* NetDevices);
   int SelectTargetNode(int possibleTarget);
//   int SelectRandomTargetNode();
   Ipv6Address SetDuplTargetAddr(NetDeviceContainer* NetDevices, std::list<std::pair<int, DuplInfo>>::iterator it2, uint32_t initiator, uint32_t target);
   void AddDuplicatedAddress (Ipv6InterfaceContainer* Ipv6Interfaces, NetDeviceContainer* NetDevices, uint32_t initiator, uint32_t target);
   Ipv6Address AddDuplAddressNewNode (NetDeviceContainer* NetDevices, uint32_t initiator, uint32_t target);
   Ipv6Address GetTargetAddress (NetDeviceContainer* NetDevices, uint32_t target);
   void AddAddress (Ipv6InterfaceContainer* Ipv6Interfaces, NetDeviceContainer* NetDevices, uint32_t initiator);
   void AddAddressRandomNodes(NetDeviceContainer* NetDevices);
   //void AddAddressNewRandomNodes(Ipv6InterfaceContainer* Ipv6Interfaces, NetDeviceContainer* NetDevices);
   void AddDuplAddressRandomNodes(Ipv6InterfaceContainer* Ipv6Interfaces, NetDeviceContainer* NetDevices, bool sameToAll);
   void RemoveAddress (NetDeviceContainer* NetDevices, uint32_t initiator);
   void RemoveAddressRandomNodes(NetDeviceContainer* NetDevices);
   void RemoveDuplicatedAddress (NetDeviceContainer* NetDevices, uint32_t initiator, uint32_t target);
   void SetInterfacesUp (NetDeviceContainer* NetDevices);
   void SetInterfacesUpNewNodes (NetDeviceContainer* NetDevices, bool newNodes);	//sets up only teh part of nodes which are NOT selected in m_selectedNodes
   void SetInterfacesDown (NetDeviceContainer* NetDevices);
   void SetInterfacesDownNewNodes (NetDeviceContainer* NetDevices);
   double SelectTparam(int noOfRuns);
   void SingleRunInit(NodeContainer* &wifiNodes,	NetDeviceContainer* &wifiDevices,
		   InternetStackHelper_Nd* &internetv6, Ipv6InterfaceContainer* &wifiInterfaces);
   void NetAnimSetup(NodeContainer* &wifiNodes);

   //constants:
   static const double m_t9 = 2.26216;
   static const double m_t19 = 2.093;	//z rozkladu t Studenta o 19 stopniach swobody na poziomie ufnosci 95%
   static const double m_t24 =  2.0639;
   static const double m_t29 =  2.04523;
   static const double m_t44 =  2.01537;
   static const double m_t39 =  2.02269;
   static const double m_t49 =  2.00958;
   static const double m_t59 =  2.001;
   static const double m_t69 =  1.995;
   static const double m_accuracyHL =10.0;	//1/10 czyli 10% dopuszczone
   static const double m_accuracyML = 10.0; //1 / 0.15;	//15% dopuszczone

   static const double m_IDEAL_CHANNEL_MAX_RANGE = 100.0;
   static const double m_REAL_CHANNEL_MAX_RANGE = 50.0;

  //initial variables:
  int m_verbose;
  int m_nWifi;
  int m_initiatorNo;
  int m_targetNo;
  int m_noOfRandomNodes;	//new nodes to start operation
  int m_noOfDuplications;	//no. of nodes (out of random nodes) which have duplicated addresses
  int m_noOfSameDupl;	//number of nodes that have the same duplicated address assigned

  int m_resultType;
  enum resultTypeId
  {
	  SingleRunFullTxRxTables = 0,
	  SingleRun = 1,
  	  HopLimitTest = 2,
  	  HopLimitSelectedNode = 3,
  	  DupDetection = 4,
  	  DupProbability = 5,
  	  mNSMessageLifetime = 6,
  	  HopLimitRandomNodes = 7,
  	  DupProbRandomNodes = 8,
  	  mNSMessageCountRandomNodes = 9,
  	  mNSMessageLifetimeRandomNodes = 10,
  	  MultiExpNoDupRN = 11,
  	  MultiExpRN = 12
   };

  int m_ndType;
  //do not needed here - defined in InternetStackHelper_Nd
  /*enum ndTypeId
  {
	ND_BASIC = 0,
	ND_SIMPLE = 1,
	ND_NDPP = 2,
	ND_NDPP_FRW = 3
  };*/

  std::string m_resultFileSuffix;
  std::string m_resultGroupName;

  int m_topologyType;
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

  int m_wifiStd;
  enum phyWifiStdId
  {
	  Standard_802_11_a = 0,
	  Standard_802_11_b,
	  Standard_802_11_g,
	  Standard_802_11_n
  };

  int m_wifiMode;
  enum wifiModeId
  {
	  OfdmRate6Mbps,
	  OfdmRate24Mbps,
	  OfdmRate54Mbps,
	  DsssRate2Mbps,
	  DsssRate11Mbps
  };

  int m_channelType;
  enum channelTypeId
  {
	  IDEAL = 0,
	  REAL = 1,	//log-distance
	  REAL2 = 2	//log-distance + fading
  };

  uint32_t m_initialHopLimit;
  int32_t m_maxHopLimit;
  int32_t m_hlStep;

  int m_frwCountLimit;
  int m_dadReplyCountLimit;
  int m_nDADcountLimit;
  int m_retransTimerNDad;	//(in ms!!!)
  double m_mprpDelay;
  double m_mprStartDelay;
  double m_dadDelay;
  double m_frwDelay;
  bool m_frwUseSrcAddr;

  int m_noOfRuns;
  int m_multiExpDupNoOfRuns;
  int m_maxNoOfRuns;

  std::string m_inputCommand;

 //simulation environment elements, helpers:

  WifiHelper* m_wifi;
  YansWifiChannelHelper* m_channel;
  YansWifiPhyHelper* m_phy;
  NqosWifiMacHelper* m_mac;
  // std::list<MobilityHelper> mobilities;
  std::list<Vector> m_topology;
  MobilityHelper m_mobility;
  Ptr<PositionAllocator> m_positionAlloc;
  typedef std::list<Vector>::iterator TopologyI;
  typedef std::list<MobilityHelper>::iterator MobilityHelpersI;

  bool m_mobilityHierarchyInit;

  Ipv6AddressHelper* m_addresses;

  bool m_resultPrintInit;
  ResultsCollection m_results;

  uint64_t m_streamsUsed;
  uint64_t m_extraStreamNo;
  uint64_t m_extraStreamNoRandomNodes;

  Ptr<UniformRandomVariable>  m_randomNodesSelector;
  int m_randomNodesNoOfRuns;
  bool m_randomNodesInit;
  std::list<std::pair<int, Ipv6Address>> m_selectedNodes;	//keep track of selected nodes
  typedef std::list<std::pair<int, Ipv6Address>>::iterator ListIntAddrI;

  std::list<std::list<int>> m_randNodesSets;	//store (ordered) sets of random nodes selected for each long simulation run
  typedef std::list<std::list<int>>::const_iterator ListListIntCI;

  //-> unordered lists used since when duplications are present target addresses are assigned sequentially - to avoid correlations
  //between node numbers and their positions
  std::list<std::pair<int, DuplInfo>> m_selectedNodesDupl;	//keep track of selected duplicated nodes
  std::list<std::pair<int, DuplInfo>> m_selectedNodesSameDupl;	//keep track of selected duplicated nodes
  bool m_newNodes;
  bool m_newAutoAddresses;

  bool m_controlNoOfRuns;

  bool m_useNetAnim;
  AnimationInterface* m_anim;
  int m_animFileNo;
 // std::string m_animFilesFolder;

  bool m_withMobility;
  double m_pauseTimeMax;	//in sec.
  double m_pauseTimeMin;	//in sec.
  int m_speed;	//in m/s
  enum speedId
   {
 	  Walking = 0,
 	  Running = 1
   };


  /*the stream pull is devided into two parts. Teh smaller one - between MID_STREAM_NO and MAX_STREAM_NO
   * is for channel assignments in REAL models -otherwise the topology would have changed comparing to IDEAL case
   */
  const static uint64_t MID_STREAM_NO = ((1ULL)<<63)-1000ULL;
  const static uint64_t SECOND_MID_STREAM_NO = ((1ULL)<<63)-150ULL;
  const static uint64_t MAX_STREAM_NO = ((1ULL)<<63)-100ULL;

  //std::string m_outFilename;

  friend class ResultsCollection;
};



NdppExperiment::NdppExperiment()
: m_verbose(false),
  m_nWifi(30),
  m_initiatorNo(2),
  m_targetNo(-1),
  m_noOfRandomNodes(0),
  m_noOfDuplications(0),
  m_noOfSameDupl(0),
  m_resultType(HopLimitTest),
  m_ndType(InternetStackHelper_Nd::ND_NDPP),
  m_resultFileSuffix(""),
  m_resultGroupName ("noname"),
  m_topologyType(Grid),
  m_wifiStd(Standard_802_11_g),
  m_wifiMode(OfdmRate54Mbps),
  m_channelType(IDEAL),
  m_initialHopLimit(4),
  m_maxHopLimit(4),
  m_hlStep(1),
  m_frwCountLimit(1),
  m_dadReplyCountLimit(1),
  m_nDADcountLimit(1),
  m_retransTimerNDad(1000),
  m_mprpDelay(1.0),
  m_mprStartDelay(1.0),
  m_dadDelay(1.0),
  m_frwDelay(100.0),
  m_frwUseSrcAddr(0),
  m_noOfRuns(10),
  m_multiExpDupNoOfRuns(50),
  m_maxNoOfRuns(70),
  m_inputCommand(""),
  m_mobilityHierarchyInit(false),
  m_resultPrintInit(false),
  m_streamsUsed (0),
  m_extraStreamNo(MID_STREAM_NO),
  m_extraStreamNoRandomNodes(SECOND_MID_STREAM_NO),
  m_randomNodesSelector(0),
  m_randomNodesNoOfRuns(5),
  m_randomNodesInit(false),
  m_newNodes(true),
  m_newAutoAddresses(false),
  m_controlNoOfRuns(true),
  m_useNetAnim(false),
  m_anim(0),
  m_animFileNo(1),
  m_withMobility(0),
  m_pauseTimeMax(0.0),
  m_pauseTimeMin(0.0),
  m_speed(0)
{
}

NdppExperiment::~NdppExperiment()
{
	delete m_wifi;
	delete m_channel;
	delete m_phy;
	delete m_mac;
	delete m_addresses;
	m_topology.clear();
	m_selectedNodes.clear();
	m_randNodesSets.clear();
}

/*
 * UWAGA!!! ta funkcja zmienia baze Ipv6AddressHelper (jest 1 Ipv6AddressGenerator na symulacje, z niego korzysta
 * tez helper). Nie powinien to byc problme, bo helper tutaj i tak nei jest wykorzystywany, bo generowane sa  tylko
 * adresy link-local, co sie odbywa automatycznie. W innych konfiguracjach symulatora trzeba uwazac.
 */
void NdppExperiment::SelectRandomNodes(NetDeviceContainer* NetDevices)
{
	NS_ASSERT_MSG(m_noOfRandomNodes <= m_nWifi, "AddAddress RandomNodes: noOfNodes exceeds network size!!!");
	NS_ASSERT_MSG(m_selectedNodes.empty(), "Selecting new nodes, but old ones still valid");
	NS_ASSERT_MSG(m_selectedNodesDupl.empty(), "Selecting new nodes, but old duplicated ones still valid");
	NS_ASSERT_MSG(m_selectedNodesSameDupl.empty(), "Selecting new nodes, but old duplicated ones (same address) still valid");

	//set RNG
	if(!m_randomNodesInit)
	{
		//reset generator to initial value
		m_randomNodesSelector->SetAttribute("Stream", IntegerValue(m_extraStreamNoRandomNodes));
		//m_extraStreamNoRandomNodes++;
		m_randomNodesInit = true;
		NS_ASSERT_MSG(m_extraStreamNoRandomNodes < MAX_STREAM_NO, "No. of RNG streams close to end, fix RNs (extraStreamNo overflow)!!!!");
	}

	int currentInitiator = 0;
	Ipv6Address addr_target;
	//std::map<int, Ipv6Address>::const_iterator it;
	ListIntAddrI it;
	std::list<std::pair<int, DuplInfo>>::iterator it2;
	std::list<std::pair<int, DuplInfo>>::iterator it3;


	bool needSearchAgain = false;
	bool found = false;
	m_randNodesSets.push_front(std::list<int> ());	//create empty entry for new set to be selected

	do
	{
		needSearchAgain = false;

		for(int i=0; i < m_noOfRandomNodes; i++)
		{
			do
			{
				found = false;
				//select nodeNo
				currentInitiator = m_randomNodesSelector->GetInteger(0, m_nWifi-1);
				//check if not selected before
				for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
					if(it->first == currentInitiator)
					{
						found = true;
						break;
					}
			}
			while (found);

			m_selectedNodes.push_back(std::make_pair(currentInitiator, "::"));
			m_randNodesSets.begin()->push_front(currentInitiator);
		}
		//check if this set was not selected previously - if yes, select again!!!!

		m_randNodesSets.begin()->sort();
		if(m_randNodesSets.size() > 1)
		{
			ListListIntCI it = m_randNodesSets.begin();
			it++;	//skip first element since new data are there!!!
			ListIntCI it_selN = m_randNodesSets.begin()->begin();	//new set stored here

			do
			{
				uint counter = 0;
				NS_ASSERT_MSG(m_randNodesSets.begin()->size() == it->size(), "m_selectedNodes size != previously stored selection size");
				for(ListIntCI it2 = it->begin(); it2 != it->end(); it2++)
				{
					if(*it2 == *it_selN)
						counter++;
					else
						break;
					it_selN++;
				}
				if(counter == m_selectedNodes.size())
				{
					needSearchAgain = true;
					//clear previously selected set since it is duplicated
					m_randNodesSets.begin()->clear();
					m_selectedNodes.clear();
				}
				it++;
			}
			while(!needSearchAgain && it != m_randNodesSets.end());
		}
		else
			break;
	}
	while (needSearchAgain);


	if(m_noOfDuplications)
	{
		NS_ASSERT_MSG(m_noOfDuplications <= m_nWifi/2.0, "No. of duplications is more than 50% of all nodes");
		NS_ASSERT_MSG(m_noOfSameDupl <= m_noOfDuplications, "No. of same duplications greater than no. of duplications!");
		int currentNoInSelectedSet;
		int target = 0;


		//select nodes with duplicated addresses out of random nodes
		//TODO: mozna toz robic efektywniej - wybierac zduplikowane, albo te bez duplikacji jesli noOfDupl > noOfRandom/2
		if(m_noOfDuplications == m_noOfRandomNodes)
		{
			for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
				m_selectedNodesDupl.push_back(std::make_pair(it->first, DuplInfo("::",-1)));
		}
		else
		{
			for(int i=0; i<m_noOfDuplications; i++)
			{
				do
				{
					found = false;
					//select nodeNo
					currentNoInSelectedSet = m_randomNodesSelector->GetInteger(0, m_selectedNodes.size()-1);
					it = m_selectedNodes.begin();
					for(int j=0; j<currentNoInSelectedSet; j++)
						it++;
					currentInitiator = it->first;

					//check if not selected before
					for(it2 = m_selectedNodesDupl.begin(); it2!= m_selectedNodesDupl.end(); it2++)
						if(it2->first == currentInitiator)
						{
							found = true;
							break;
						}
				}
				while (found);
				m_selectedNodesDupl.push_back(std::make_pair(currentInitiator, DuplInfo("::",-1)));
			}
		}

		//select nodes with the same duplicated addresses out of random nodes
		if(m_noOfSameDupl)
		{
			//select target node
			target = SelectTargetNode(target);


			if(m_noOfDuplications == m_noOfSameDupl)	//select all nodes
			{
				for(it2 = m_selectedNodesDupl.begin(); it2!= m_selectedNodesDupl.end(); it2++)
				{
					addr_target = SetDuplTargetAddr(NetDevices,it2,it2->first,target);
					m_selectedNodesSameDupl.push_back(std::make_pair(it2->first, DuplInfo(addr_target, target)));
				}
			}
			else
			{
				for(int i=0; i<m_noOfSameDupl; i++)
				{
					do
					{
						//select nodeNo
						found = false;
						currentNoInSelectedSet = m_randomNodesSelector->GetInteger(0, m_selectedNodesDupl.size()-1);
						it2 = m_selectedNodesDupl.begin();
						for(int j=0; j < currentNoInSelectedSet; j++)
							it2++;
						currentInitiator = it2->first;

						//check if not selected before
						for(it3 = m_selectedNodesSameDupl.begin(); it3!= m_selectedNodesSameDupl.end(); it3++)
							if(it3->first == currentInitiator)
							{
								found = true;
								break;
							}
					}
					while (found);

					addr_target = SetDuplTargetAddr(NetDevices,it2,currentInitiator,target);
					m_selectedNodesSameDupl.push_back(std::make_pair(currentInitiator, DuplInfo(addr_target, target)));
				}
			}

			//increment target;
			target++;
		}

		//select addresses for remaining nodes
		if(m_noOfDuplications != m_noOfSameDupl)
			for(it2 = m_selectedNodesDupl.begin(); it2!= m_selectedNodesDupl.end(); it2++)
			{
				if(it2->second.m_addr.IsAny())	//otherwise address assigned above
				{
					target = SelectTargetNode(target);
					SetDuplTargetAddr(NetDevices,it2,it2->first,target);
					target++;
				}
			}
	}

	//store/assign addresses for remaining random nodes which are not duplicated
	if(!m_newNodes)
	{

		Ipv6AddressGenerator::Init(Ipv6Address ("2002::0"), Ipv6Prefix (64), Ipv6Address ("0::0200:00ff:fe00:0001"));

		//assign new addresses to remaining nodes from selectedNodes list

		if(m_noOfDuplications != m_noOfRandomNodes)
			for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
			{
				if(it->second.IsAny())	//otherwise address assigned above
				{
					//set address:
					addr_target = Ipv6AddressGenerator::NextAddress(Ipv6Prefix(64));
					it->second = addr_target;
				}
			}
	}
	else if(m_noOfDuplications != m_noOfRandomNodes)
	{
		//remember link-local addresses:
		for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
		{
			if(it->second.IsAny())	//otherwise address assigned above
			{
				//set address:
				addr_target = GetTargetAddress(NetDevices,it->first);
				it->second = addr_target;
			}
		}
	}
}

int NdppExperiment::SelectTargetNode(int possibleTarget)
{
	ListIntAddrI it;
	int target = 0;
	bool found = false;

	if(m_noOfRandomNodes != m_nWifi)
	{
		target = possibleTarget;
		do
		{
			found = false;
			for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
				if(it->first == target)
				{
					found = true;
					target++;
					break;
				}
		}
		while(found && target<=m_nWifi);
	}
	return target;
}

/*int NdppExperiment::SelectRandomTargetNode()
{
	int target = 0;
	if(m_noOfRandomNodes != m_nWifi)
	{
		target = possibleTarget;
		do
		{
			if(m_selectedNodes.find(target) != m_selectedNodes.end())
				target++;
			else
				break;
		}
		while(target<=m_nWifi);
	}
	return target;
}*/

Ipv6Address NdppExperiment::SetDuplTargetAddr(NetDeviceContainer* NetDevices, std::list<std::pair<int, DuplInfo>>::iterator it2, uint32_t initiator, uint32_t target)
{
	//set address:
	/*Ipv6Address addr_target = (m_newNodes)? AddDuplAddressNewNode(NetDevices, initiator, target)
			: GetTargetAddress(NetDevices, target);*/
	Ipv6Address addr_target = GetTargetAddress(NetDevices, target);
	it2->second.m_addr = addr_target;
	it2->second.m_target = target;
	//remember this address also on the selectedNodes list
	for(ListIntAddrI it = m_selectedNodes.begin();
			it!= m_selectedNodes.end(); it++)
		if(it->first == (int)initiator)	//currentInitiator is the same as it2->first
		{
			it->second = addr_target;
			break;
		}

	return addr_target;
}

void NdppExperiment::AddAddress(Ipv6InterfaceContainer* Ipv6Interfaces, NetDeviceContainer* NetDevices, uint32_t initiator)
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

void NdppExperiment::AddAddressRandomNodes(NetDeviceContainer* NetDevices)
{
	NS_ASSERT_MSG(m_noOfRandomNodes <= m_nWifi, "AddAddress RandomNodes: noOfNodes exceeds network size!!!");

	Ptr<NetDevice> device_init;
	Ptr<Ipv6> ipv6_init;
	int32_t ifIndex_init = 0;
	std::list<std::pair<int, Ipv6Address>>::const_iterator it;

	for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
	{
		device_init = NetDevices->Get(it->first);
		ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
		ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
		NS_ASSERT_MSG(ifIndex_init != -1, "The interface index should be known at this point already!!!");
		ipv6_init->SetMetric (ifIndex_init, 1);
		ipv6_init->SetUp (ifIndex_init);
		ipv6_init->AddAddress (ifIndex_init, Ipv6InterfaceAddress(it->second));
		NS_LOG_INFO ("Assign new IPv6 address " << it->second << " to node " << it->first << ".");
	}
}

Ipv6Address NdppExperiment::AddDuplAddressNewNode (NetDeviceContainer* NetDevices, uint32_t initiator, uint32_t target)
{
	Ptr<NetDevice> device_init;
	Ptr<Ipv6L3Protocol> ipv6_init;
	int32_t ifIndex_init = 0;
	Ptr<Ipv6Interface> interface_init;
	Ipv6Address addr;
	Ipv6Address addr_target;

	addr_target = GetTargetAddress(NetDevices, target);

	if(initiator != target)
	{
		//set address:
		//step 1 - remove automatically assigned address
		device_init = NetDevices->Get(initiator);
		ipv6_init = device_init->GetNode()->GetObject<Ipv6L3Protocol> ();
		ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
		NS_ASSERT(ifIndex_init != -1);
		interface_init = ipv6_init->GetInterface (ifIndex_init);
		addr = (interface_init->GetAddress(0)).GetAddress();
		NS_ASSERT_MSG(addr.IsLinkLocal(), "The link local address should have been selected here, but it is not");
		ipv6_init->RemoveAddress(ifIndex_init, addr);

		NS_LOG_INFO ("Address " << addr << " removed from node " << initiator << ".");

		//step 2 - assign new one - address of the first node, that is not in random set
		ipv6_init->AddAddress(ifIndex_init, Ipv6InterfaceAddress(addr_target));
		NS_LOG_INFO ("Assign IPv6 addresses of node " << target << " to node " << initiator << ".");
		NS_LOG_INFO ("New duplicated IPv6 address: " << addr_target << " assigned to node " << initiator << ".");
	}

	return addr_target;
}

Ipv6Address NdppExperiment::GetTargetAddress (NetDeviceContainer* NetDevices, uint32_t target)
{
	Ptr<NetDevice> device_target;
	Ptr<Ipv6> ipv6_target;
	int32_t ifIndex_target;
	Ipv6InterfaceAddress ipv6IntAddr_target;
	Ipv6Address addr_target;

	//get the target address
	device_target = NetDevices->Get(target);
	ipv6_target = device_target->GetNode()->GetObject<Ipv6> ();
	ifIndex_target = ipv6_target->GetInterfaceForDevice (device_target);
	NS_ASSERT_MSG(ifIndex_target != -1, "Adding duplicated address - the interface index of target node should be known at this point already!!!");
	ipv6IntAddr_target = ipv6_target->GetAddress(ifIndex_target, 0);
	//create new interface address from Ipv6Address of a target (otherwise other interface params are copied as well:( )
	addr_target = ipv6IntAddr_target.GetAddress();

	return addr_target;
}

/*
 * UWAGA!!! ta funkcja zmienia baze Ipv6AddressHelper (jest 1 Ipv6AddressGenerator na symulacje, z niego korzysta
 * tez helper). Nie powinien to byc problme, bo helper tutaj i tak nei jest wykorzystywany, bo generowane sa  tylko
 * adresy link-local, co sie odbywa automatycznie. W innych konfiguracjach symulatora trzeba uwazac.
 */
/*void NdppExperiment::AddAddressNewRandomNodes(Ipv6InterfaceContainer* Ipv6Interfaces,
		NetDeviceContainer* NetDevices)
{
	uint32_t currentInitiator = 0;
	Ptr<NetDevice> device_init;
	Ptr<Ipv6> ipv6_init;
	int32_t ifIndex_init = 0;
	std::map<int, Ipv6Address>::const_iterator it;
	bool nodeNoOk = false;

	if(m_selectedNodes.empty()) 	//select new nodes and assign addresses
	{

		SelectRandomNodes();

		Ipv6AddressGenerator::Init(Ipv6Address ("2002::0"), Ipv6Prefix (64), Ipv6Address ("0::0200:00ff:fe00:0001"));

		for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
		{


			device_init = NetDevices->Get(currentInitiator);
			ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
			ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
			NS_ASSERT_MSG(ifIndex_init != -1, "Adding second address - the interface index should be known at this point already!!!");
			if (ifIndex_init == -1)
			{
			  ifIndex_init = ipv6_init->AddInterface (device_init);
			}

			//uWAGA!!! assign address - Ipv6AddressGenerator ma f. statyczne i jedna  "baze"
			//dla calej symulacji

			Ipv6Address ipv6Addr = Ipv6AddressGenerator::NextAddress(Ipv6Prefix(64));
			ipv6_init->SetMetric (ifIndex_init, 1);
			ipv6_init->SetUp (ifIndex_init);
			ipv6_init->AddAddress (ifIndex_init, Ipv6InterfaceAddress(ipv6Addr));
			//Simulator::Schedule(Seconds(0.0), &Ipv6::AddAddress, ipv6_init, ifIndex_init, Ipv6InterfaceAddress (ipv6Addr));

			m_selectedNodes.insert(std::make_pair(currentInitiator, ipv6Addr));

			//to chyba nie jest potzrebne, jelsi interfejs juz istnial??? TODO
			//Ipv6Interfaces->Add (ipv6_init, ifIndex_init);
			//Simulator::Schedule(Seconds(0.0), &Ipv6InterfaceContainer::Add, Ipv6Interfaces, ipv6_init, ifIndex_init);
			NS_LOG_INFO ("Assign new IPv6 address " << ipv6Addr<< " to node " << currentInitiator << ".");
		}
	}
	else	//reassign previously selected addresses
		for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
		{
			device_init = NetDevices->Get(it->first);
			ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
			ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
			NS_ASSERT_MSG(ifIndex_init != -1, "The interface index should be known at this point already!!!");
			ipv6_init->SetMetric (ifIndex_init, 1);
			ipv6_init->SetUp (ifIndex_init);
			Simulator::Schedule(Seconds(0.0), &Ipv6::AddAddress, ipv6_init, ifIndex_init, Ipv6InterfaceAddress(it->second));
			NS_LOG_INFO ("Reassign new IPv6 address " << it->second << " to node " << it->first << ".");
		}
}*/

//this function sets node no. 1 arbitrary as target
//TODO - finish this impelemnetation if needed
void NdppExperiment::AddDuplAddressRandomNodes(Ipv6InterfaceContainer* Ipv6Interfaces,
		NetDeviceContainer* NetDevices, bool sameToAll)
{
	NS_ASSERT_MSG(m_noOfRandomNodes <= m_nWifi, "AddAddress RandomNodes: noOfNodes exceeds network size!!!");

	int target = 0;
	int currentInitiator = 0;
	Ptr<NetDevice> device_init;
	Ptr<Ipv6> ipv6_init;
	int32_t ifIndex_init = 0;
	Ipv6Address ipv6Addr;
	std::list<std::pair<int, Ipv6Address>>::const_iterator it;
	bool nodeNoOk = false;

	Ptr<NetDevice> device_target;
	Ptr<Ipv6> ipv6_target;
	int32_t ifIndex_target;
	Ipv6InterfaceAddress ipv6IntAddr_target;

	if(sameToAll)
	{
		device_target = NetDevices->Get(target);
		ipv6_target = device_target->GetNode()->GetObject<Ipv6> ();
		ifIndex_target = ipv6_target->GetInterfaceForDevice (device_target);
		NS_ASSERT_MSG(ifIndex_target != -1, "Adding duplicated address - the interface index of target node should be known at this point already!!!");

		ipv6IntAddr_target = ipv6_target->GetAddress(ifIndex_target, 0);
		//create new interface address from Ipv6Address of a target (otherwise other interface params are copied as well:( )
		ipv6Addr = ipv6IntAddr_target.GetAddress();
	}

	if(m_selectedNodes.empty()) 	//select new nodes and assign addresses
	{

		//set RNG
		if(!m_randomNodesInit)
		{
			m_randomNodesSelector->SetAttribute("Stream", IntegerValue(m_streamsUsed));
			m_streamsUsed++;
			m_randomNodesInit = true;
		}

		for(int i = 0; i < m_noOfRandomNodes; i++)
		{
			do
			{
				//select nodeNo
				currentInitiator = m_randomNodesSelector->GetInteger(1, m_nWifi-1);	//node no. 0 excluded!!!
				nodeNoOk = false;

				//check if not selected before
				for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
					if(it->first == currentInitiator)
					{
						nodeNoOk = true;
						break;
					}
			}
			while (nodeNoOk);

			NS_LOG_INFO ("Assign IPv6 addresses of node " << target << " to node " << currentInitiator << ".");

			device_init = NetDevices->Get(currentInitiator);
			ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
			ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
			NS_ASSERT_MSG(ifIndex_init != -1, "Adding duplicated address - the interface index of initiator node should be known at this point already!!!");

			if(!sameToAll)
			{
				//TODO: choose address here - trzeba dodac sprawdzanie, czy
				if(i != it->first)
					target = i;
				else if(i != m_nWifi-1)
					target = i+1;
				else
					target = i-1;

				device_target = NetDevices->Get(target);
				ipv6_target = device_target->GetNode()->GetObject<Ipv6> ();
				ifIndex_target = ipv6_target->GetInterfaceForDevice (device_target);
				NS_ASSERT_MSG(ifIndex_target != -1, "Adding duplicated address - the interface index of target node should be known at this point already!!!");

				ipv6IntAddr_target = ipv6_target->GetAddress(ifIndex_target, 0);
				//create new interface address from Ipv6Address of a target (otherwise other interface params are copied as well:( )
				ipv6Addr = ipv6IntAddr_target.GetAddress();
			}
			ipv6_init->SetMetric (ifIndex_init, 1);
			ipv6_init->SetUp (ifIndex_init);
			ipv6_init->AddAddress (ifIndex_init, Ipv6InterfaceAddress(ipv6Addr));
			//Simulator::Schedule(Seconds(0.0), &Ipv6::AddAddress, ipv6_init, ifIndex_init, Ipv6InterfaceAddress (ipv6Addr));

			m_selectedNodes.push_back(std::make_pair(currentInitiator, ipv6Addr));

			//to chyba nie jest potzrebne, jelsi interfejs juz istnial??? TODO
			//Ipv6Interfaces->Add (ipv6_init, ifIndex_init);
			//Simulator::Schedule(Seconds(0.0), &Ipv6InterfaceContainer::Add, Ipv6Interfaces, ipv6_init, ifIndex_init);
			NS_LOG_INFO ("New IPv6 address: " << ipv6Addr << " assigned to node " << currentInitiator << ".");
		}
	}
	else	//reassign previously selected addresses TODO
		for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
		{
			device_init = NetDevices->Get(it->first);
			ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
			ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
			NS_ASSERT_MSG(ifIndex_init != -1, "The interface index should be known at this point already!!!");
			ipv6_init->SetMetric (ifIndex_init, 1);
			ipv6_init->SetUp (ifIndex_init);

			Simulator::Schedule(Seconds(0.0), &Ipv6::AddAddress, ipv6_init, ifIndex_init, Ipv6InterfaceAddress(it->second));
			NS_LOG_INFO ("Reassign new IPv6 address " << it->second << " to node " << it->first << ".");
		}
}

void NdppExperiment::RemoveAddress(NetDeviceContainer* NetDevices, uint32_t initiator)
{
	Ptr<NetDevice> device_init = NetDevices->Get(initiator);
	Ptr<Ipv6> ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();

	int32_t ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
	NS_ASSERT(ifIndex_init != -1);

	ipv6_init->RemoveAddress(ifIndex_init, Ipv6Address("2002:0000:0000:0000:0200:00ff:fe00:000e"));

	NS_LOG_INFO ("Address 2002:0000:0000:0000:0200:00ff:fe00:000e removed from node " << initiator << ".");
}

void NdppExperiment::RemoveAddressRandomNodes(NetDeviceContainer* NetDevices)
{
	Ptr<NetDevice> device_init;
	Ptr<Ipv6> ipv6_init;
	int32_t ifIndex_init = 0;
	std::list<std::pair<int, Ipv6Address>>::const_iterator it;
	bool removeOK = false;


	for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
	{
		device_init = NetDevices->Get(it->first);
		ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();

		ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
		NS_ASSERT(ifIndex_init != -1);

		removeOK = ipv6_init->RemoveAddress(ifIndex_init, it->second);
		NS_ASSERT_MSG(removeOK, "RemoveAddressRandomNodes: Address cannot be removed.(node= " << it->first << ", address= " << it->second << ")");

		NS_LOG_INFO ("Address " << it->second << " removed from node " << it->first << ".");
	}
}

void NdppExperiment::RemoveDuplicatedAddress(NetDeviceContainer* NetDevices, uint32_t initiator, uint32_t target)
{
	Ptr<NetDevice> device_init = NetDevices->Get(initiator);
	Ptr<NetDevice> device_target = NetDevices->Get(target);
	Ptr<Ipv6> ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
	Ptr<Ipv6> ipv6_target = device_target->GetNode()->GetObject<Ipv6> ();

	int32_t ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
	int32_t ifIndex_target = ipv6_target->GetInterfaceForDevice (device_target);
	NS_ASSERT(ifIndex_init != -1);
	NS_ASSERT(ifIndex_target != -1);

	Ipv6InterfaceAddress ipv6IntAddr_target = ipv6_target->GetAddress(ifIndex_target, 0);
	Ipv6Address target_addr = ipv6IntAddr_target.GetAddress();

	ipv6_init->RemoveAddress(ifIndex_init, target_addr);

	NS_LOG_INFO ("Address " << target_addr << " removed from node " << initiator << ".");
}

void NdppExperiment::AddDuplicatedAddress(Ipv6InterfaceContainer* Ipv6Interfaces, NetDeviceContainer* NetDevices, uint32_t initiator, uint32_t target)
{
	NS_LOG_INFO ("Assign IPv6 addresses of node " << target << " to node " << initiator << ".");
	Ptr<NetDevice> device_init = NetDevices->Get(initiator);
	Ptr<NetDevice> device_target = NetDevices->Get(target);

	Ptr<Ipv6> ipv6_init = device_init->GetNode()->GetObject<Ipv6> ();
	Ptr<Ipv6> ipv6_target = device_target->GetNode()->GetObject<Ipv6> ();

	int32_t ifIndex_init = ipv6_init->GetInterfaceForDevice (device_init);
	if (ifIndex_init == -1)
	{
	  ifIndex_init = ipv6_init->AddInterface (device_init);
	}
	int32_t ifIndex_target = ipv6_target->GetInterfaceForDevice (device_target);
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

	Ipv6Interfaces->Add (ipv6_init, ifIndex_init);
	NS_LOG_INFO ("New IPv6 address: " << ipv6Addr<< " assigned to node " << initiator << ".");
}

void NdppExperiment::SetInterfacesUp (NetDeviceContainer* NetDevices)
{
	Ptr<NetDevice> device;
	Ptr<Ipv6L3Protocol> ipv6;
	int32_t ifIndex = 0;
	NS_ASSERT_MSG (ifIndex >= 0, "SetInterfacesUp(): Interface index not found");

	for (u_int32_t i = 0; i < NetDevices->GetN(); i++)
	{
		device = NetDevices->Get(i);
		ipv6 = device->GetNode()->GetObject<Ipv6L3Protocol> ();
		ifIndex = ipv6->GetInterfaceForDevice (device);
		ipv6->SetUp(ifIndex);
		//start MPR processes - this is usually done after addAdress but to keep simulations concise
		//we do it here as well (for Single RunLong this is important)
		if(m_ndType != InternetStackHelper_Nd::ND_BASIC && m_ndType != InternetStackHelper_Nd::ND_SIMPLE)
			(DynamicCast<Ipv6Interface_Ndpp>(ipv6->GetInterface(ifIndex)))->MprsInit();
	}
}


void NdppExperiment::SetInterfacesUpNewNodes(NetDeviceContainer* NetDevices, bool newNodes)
{
	Ptr<NetDevice> device;
	Ptr<Ipv6L3Protocol> ipv6;
	int32_t ifIndex = 0;
	std::list<std::pair<int, Ipv6Address>>::const_iterator it;


	if (newNodes)	//set up only new nodes
	{
		Ptr<Ipv6Interface> interface;
		Ipv6Address addr, target_addr;
		bool removeOK = false;

		for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
		{
			device = NetDevices->Get(it->first);
			ipv6 = device->GetNode()->GetObject<Ipv6L3Protocol> ();
			ifIndex = ipv6->GetInterfaceForDevice (device);
			NS_ASSERT_MSG (ifIndex >= 0, "SetInterfacesUpNewNodes(): Interface index not found");
			interface = ipv6->GetInterface (ifIndex);

			//set address:
			//step 1 - remove automatically assigned address and all possible other addresses
						//(may be more than one if automatic AAC is being done)
			while (interface->GetNAddresses() > 0)
			{
		    	addr = (interface->GetAddress(0)).GetAddress();
		    	removeOK = ipv6->RemoveAddress(ifIndex, 0);
		    	NS_ASSERT_MSG(removeOK, "SetInterfacesUpNewNodes: Address cannot be removed.");
		    	NS_LOG_INFO ("Address " << addr << " removed from node " << (double)it->first << ".");
			}

			//step 2 - set interface up (VERY IMPORTANT to do this before AddAddress (MPR processess init issue)
			ipv6->SetUp(ifIndex);

			//step 3 - add new address (as indicated on teh list)
			//setInterfacesDown stopped DAD and this is the only way to re-initiate it (AddAddress is the only function that is doing that)
			target_addr = it->second;
			ipv6->AddAddress(ifIndex, Ipv6InterfaceAddress(target_addr));
			NS_LOG_INFO ("Node: " << (double)it->first << " set up with address: " << target_addr);
		}
	}
	else	//set up "baseline" -> "old" nodes
	{
		bool found = false;
		for (u_int32_t i = 0; i < NetDevices->GetN(); i++)
		{
			found = false;
			if(m_newNodes)
				for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
					if(it->first == (int) i)
					{
						found = true;
						break;
					}
			if(found)
				continue;	//this node is selected and should not be set up
			device = NetDevices->Get(i);
			ipv6 = device->GetNode()->GetObject<Ipv6L3Protocol> ();
			ifIndex = ipv6->GetInterfaceForDevice (device);
			NS_ASSERT_MSG (ifIndex >= 0, "SetInterfacesUp(): Interface index not found");
			ipv6->SetUp(ifIndex);
			//start MPR processes - this is usually done after addAdress but to keep simulations concise
			//we do it here as well (for Single RunLong this is important)
			if(m_ndType != InternetStackHelper_Nd::ND_BASIC && m_ndType != InternetStackHelper_Nd::ND_SIMPLE)
				(DynamicCast<Ipv6Interface_Ndpp>(ipv6->GetInterface(ifIndex)))->MprsInit();
		}
	}
}

//UWAGA! ta funkcja przerywa MPRy tylko, jelsi jest wywyolana w trakcie symulacji (scheduled)!!!
void NdppExperiment::SetInterfacesDown (NetDeviceContainer* NetDevices)
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

		/*przeneisione do SetDown if(m_ndType == InternetStackHelper_Nd::ND_NDPP_FRW)
		{
			 Ptr<Icmpv6L4Protocol_NdppFrw> icmpv6 = ipv6->GetObject<Icmpv6L4Protocol_NdppFrw> ();
			 NS_ASSERT_MSG(icmpv6, "Couldn't get icmpv6 ndppFrw object");
			 icmpv6->ClearFrwSet();
			 icmpv6->ClearDADReplySet();
		}*/
	}


}

void NdppExperiment::SetInterfacesDownNewNodes (NetDeviceContainer* NetDevices)
{
	if(!m_newNodes)
		return;
	Ptr<NetDevice> device;
	Ptr<Ipv6> ipv6;
	int32_t ifIndex = 0;

	std::list<std::pair<int, Ipv6Address>>::const_iterator it;

	for(it = m_selectedNodes.begin(); it!= m_selectedNodes.end(); it++)
	{
		device = NetDevices->Get(it->first);
		ipv6 = device->GetNode()->GetObject<Ipv6> ();
		ifIndex = ipv6->GetInterfaceForDevice (device);
		NS_ASSERT_MSG (ifIndex >= 0, "SetInterfacesDownNewNodes(): Interface index not found");
		ipv6->SetDown (ifIndex);
		NS_LOG_INFO ("Node: " << (double)it->first << " set down.");
	}
}

double NdppExperiment::SelectTparam(int noOfRuns)
{
	double t;
	switch(noOfRuns)
	{
	case 10:
		t = m_t9;
		break;
	case 20:
		t = m_t19;
		break;
	case 25:
		t = m_t24;
		break;
	case 30:
		t = m_t29;
		break;
	case 40:
		t = m_t39;
		break;
	case 45:
		t = m_t44;
		break;
	case 50:
		t = m_t49;
		break;
	case 60:
		t = m_t59;
		break;
	case 70:
		t = m_t69;
		break;
	default:
		NS_LOG_UNCOND("selected noOfRuns not predefined - check confidence levels manually"
				" (estimation sharper than real values");
		t = m_t9;
				break;
	}
	return t;
}

void NdppExperiment::SingleRunInit(NodeContainer* &wifiNodes,	NetDeviceContainer* &wifiDevices,
		   InternetStackHelper_Nd* &internetv6, Ipv6InterfaceContainer* &wifiInterfaces)
{
	//*********************create nodes*********************:
	wifiNodes = new NodeContainer();
	wifiNodes->Create (m_nWifi);	//creates nWifi+1 nodes (??? - nwifi nodes)


	//*********************** create m_channel:
	Ptr<YansWifiChannel> chan = m_channel->Create ();
	m_phy->SetChannel(chan);
	/*in JAkes/nakagami/other models there are additional  streams used and this changes topology, since different strams are used to
	 * create it in position allocator. The aim here is to assign some large stream number to this model and keep
	 * the same numbering for all other streams in the simulation
	 */
	if(m_channelType != IDEAL)
		m_extraStreamNo += m_channel->AssignStreams (chan, m_extraStreamNo);	//quick solver of topology issue
	else
		m_streamsUsed += m_channel->AssignStreams (chan, m_streamsUsed);


	//********create interfaces and link the interfaces for given nodes to the wifi network:
	wifiDevices = new NetDeviceContainer(m_wifi->Install (*m_phy, *m_mac, *wifiNodes));

	// Assign fixed stream numbers to wifi and m_channel random variables
	m_streamsUsed += m_wifi->AssignStreams (*wifiDevices, m_streamsUsed);	//blocks PHY, Station managers, MAC

	/* //trace topology changes - to checkif topology is the same for each run:
	for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
	   (*it)->TraceConnectWithoutContext("CourseChange", MakeCallback (&CourseChange));*/


	//**********************mobility & topology:
	m_streamsUsed += m_positionAlloc->AssignStreams (m_streamsUsed);
	m_mobility.Install(*wifiNodes);
	//// Assign fixed stream numbers to mobility model, position allocator has to be handled separately
	m_streamsUsed += m_mobility.AssignStreams (*wifiNodes, m_streamsUsed);

	Ptr<MobilityModel> model;
	TopologyI it_top;
	if(m_mobilityHierarchyInit)
	{
	   NS_ASSERT_MSG(m_topology.size() == wifiNodes->GetN(), "Topology vector size different "
			   "from node container size!");
	   it_top = m_topology.begin();
	   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
	   {
		   model = (*it)->GetObject<MobilityModel> ();
		   model->SetPosition(*it_top);
		   it_top++;
	   }
	}
	else
	{
	   m_mobilityHierarchyInit = true;
	   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
	   {
		   //remember initial topology setup for each node
		   model = (*it)->GetObject<MobilityModel> ();
		   m_topology.push_back(model->GetPosition());
	   }
	}


	//*********************internet stack:
	internetv6 = new InternetStackHelper_Nd (m_ndType);
	internetv6->SetIpv4StackInstall(false);
	internetv6->InstallAll();
	// internetv6->Install(*wifiNodes);	///moze tak by bylo lepiej???

	//fix IPv6EXtensions and ICMPv6L4Protocol streams and IPv6Interface (dadDelay and MPR Start delay)
	//(plus others most likely not used in my simulations)
	m_streamsUsed += internetv6->AssignStreams (*wifiNodes, m_streamsUsed);

	if(m_resultType == SingleRunFullTxRxTables || m_resultType == DupDetection)
	{
		internetv6->EnableAsciiIcmpv6All("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/simul_results/ipv6-nd-RxTxStats");
		m_phy->EnablePcapAll ("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/pcaps/ipv6-nd", true);
	}
	else
	{
		internetv6->EnableAsciiIcmpv6All();	//this has to be done before address assignment
									   //otherwise first MPR messages are not counted
		//m_phy->EnablePcapAll ("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/pcaps/ipv6-nd", true);
	}

	wifiInterfaces = new Ipv6InterfaceContainer(m_addresses->Assign(*wifiDevices, std::vector<bool> (m_nWifi, false)));	//this also sets interfaces up
	//UWAGA!!! - po wywolaniu powyzej startuja MPRy (MPRsInit) - to jest ok, schedule wysylanai odpalany po Simulator::Run


	//monitor if random numbers are independent throughout simulation
	NS_ASSERT_MSG(m_streamsUsed < MID_STREAM_NO, "No. of RNG streams close to end, fix RNs!!!!");
	NS_ASSERT_MSG(m_extraStreamNo < SECOND_MID_STREAM_NO, "No. of RNG streams close to end, fix RNs (extraStreamNo overflow)!!!!");
}

void NdppExperiment::NetAnimSetup(NodeContainer* &wifiNodes)
{
	mkdir(("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/simul_results/netAnim/" + m_resultGroupName).c_str(),
			S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	m_anim = new AnimationInterface_Ndpp("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/simul_results/netAnim/" +
			m_resultGroupName + "/netanim-" +
			 m_resultFileSuffix + "_" + std::to_string((long long int)m_animFileNo) + ".xml");
	if(m_withMobility)
		m_anim->SetMobilityPollInterval (Seconds (0.5));
	else
		m_anim->SetMobilityPollInterval (Seconds (5));	//for mobility this needs to be changed
	m_anim->EnablePacketMetadata (false);
	//UWAGA! tu tez jets AssignStreams, ale strumien wykorzystywany przy losowym rozmieszczaniuwezlow przez animatot - u mnei neiwykorzystywany
	//a nawet gdyby - grunt, ze f. init powyzej ma zresetowane strumienie, tu one sa bez znaczenia (w oecnym zastosowaniu)

	ListIntAddrI it;
	std::list<std::pair<int, DuplInfo>>::iterator it2, it3;
	Ptr<Node> node;
	bool found = false;


	//TODO
	//ustaw polozenie wezlow - zapisz topologie
	/*uint i;
	for(TopologyI it_top = m_topology.begin(), i = 0; it_top != m_topology.end(); it_top++, i++)
		AnimationInterface::SetConstantPosition (wifiNodes->Get(i), it_top->x, it_top->y, it_top->z);
*/
	//AnimationInterface::SetLinkDescription(0,1,"LINK", "NODE0", "NODE1");
		/*uWAGA!!!!
		zeby rysowac topologie, trzebaby zmienic kod w void AnimationInterface::StartAnimation ( - na razie tylko p2p links sa rysowane
	*/


	//set initail values - important when one wants to re-launch netAnim
	for (uint32_t i = 0; i < wifiNodes->GetN (); i++)
	{
		node = wifiNodes->Get(i);
		m_anim->UpdateNodeColor( node, 0, 0, 0);	//black
		m_anim->UpdateNodeDescription(node, std::to_string((long long int) node->GetId()));
	}

	//oznacz wezly wybrane
	for(it = m_selectedNodes.begin(); it != m_selectedNodes.end(); it++)
	{
		found = false;
		for(it2 = m_selectedNodesDupl.begin(); it2!= m_selectedNodesDupl.end(); it2++)
			if(it2->first == it->first)
			{
				found = true;
				break;
			}
		if(!found)
		{
			node = wifiNodes->Get(it->first);
			m_anim->UpdateNodeDescription(node, std::to_string((long long int) node->GetId()) + "_NEW");
			m_anim->UpdateNodeColor(node, 0, 0, 255);	//blue
		}
	}

	//oznacz wezly zduplikowane
	for(it2 = m_selectedNodesDupl.begin(); it2 != m_selectedNodesDupl.end(); it2++)
	{
		found = false;
		for(it3 = m_selectedNodesSameDupl.begin(); it3!= m_selectedNodesSameDupl.end(); it3++)
			if(it3->first == it2->first)
			{
				found = true;
				break;
			}
		if(!found)
		{
			node = wifiNodes->Get(it2->first);
			m_anim->UpdateNodeDescription(node, std::to_string((long long int) node->GetId()) + "_DUP("
					+ std::to_string((long long int)it2->second.m_target) + ")");
			m_anim->UpdateNodeColor(node, 255, 0, 0);	//red
		}
	}

	for(it2 = m_selectedNodesSameDupl.begin(); it2 != m_selectedNodesSameDupl.end(); it2++)
	{
		node = wifiNodes->Get(it2->first);
		m_anim->UpdateNodeDescription(node, std::to_string((long long int) node->GetId()) + "_SAME_DUP("
				+ std::to_string((long long int)it2->second.m_target) + ")");
		m_anim->UpdateNodeColor(node, 0, 255, 0);	//green
	}

}

void NdppExperiment::CommandSetup (int argc, char **argv)
{
	CommandLine cmd;
	cmd.AddValue ("nWifi", "Number of wifi nodes", m_nWifi);
	cmd.AddValue ("verbose", "Tell echo applications to log if true", m_verbose);
	cmd.AddValue ("initiatorNo", "Selected node - DAD initiator", m_initiatorNo);
	cmd.AddValue ("targetNo", "Select\"target\" of DAD - node nr from which we will assign the address to initiating node", m_targetNo);
	cmd.AddValue ("randomNodesNo", "Number of nodes to be randomly selected for the experiment", m_noOfRandomNodes);
	cmd.AddValue ("noOfDupl", "Number of nodes having duplicated address", m_noOfDuplications);
	cmd.AddValue ("noOfSameDupl", "Number of nodes having the same address as duplicated", m_noOfSameDupl);
	cmd.AddValue ("ndType", "Select ND mode (Basic=0, simple=1, ND++ = 2, ND++ FRW = 3", m_ndType);
	cmd.AddValue ("initialHopLimit", "HopLimit value for ND++ to start with", m_initialHopLimit);
	cmd.AddValue ("maxHopLimit", "Max HopLimit value for simulation", m_maxHopLimit);
	cmd.AddValue ("hlStep", "Hop Limit step for simulation", m_hlStep);
	cmd.AddValue ("noOfRuns", "No. of similar runs for one simulation data point", m_noOfRuns);
	cmd.AddValue ("resultType", "Type of expected result to be collected during simulation", m_resultType);
	cmd.AddValue ("resultFileSuffix", "Suffix to be added to result file name", m_resultFileSuffix);
	cmd.AddValue ("resultGroupName", "Folder name for teh group of results", m_resultGroupName);
	cmd.AddValue ("topology", "Topology type (0-Grid, 1-CrossGrid, ...)", m_topologyType);
	cmd.AddValue ("wifiStd", "Wi-Fi standard (a,b,g,n))", m_wifiStd);
	cmd.AddValue ("wifiMode", "Wi-Fi Mode: modulation + bitrate", m_wifiMode);
	cmd.AddValue ("channelType", "Channel type: ideal (0) or realistic (1) or realistic with fading (2)", m_channelType);
	cmd.AddValue ("frwCount", "the value of forward count limit - no. of times the same message can be forwarded",
			m_frwCountLimit);
	cmd.AddValue ("dadReplyCount", "the value of dad reply count limit - no. of times a reply to the same n-DAD query can be sent",
			m_dadReplyCountLimit);
	cmd.AddValue ("nDADcount", "the no of cycles for an n-DAD query",
			m_nDADcountLimit);
	cmd.AddValue ("RETRANS_TIMER_NDAD", "time to wait for a reply to n-DAD query (in ms!!!)",
			m_retransTimerNDad);
	cmd.AddValue ("MPRP_DELAY", "Delay between consecutive MPR opts announcement (in sec.!!!)",
				m_mprpDelay);
	cmd.AddValue ("MprStartDelay", "Delay before sending first MPR Parameters option after add "
			"address (in sec.!!!)", m_mprStartDelay);
	cmd.AddValue ("FrwDelay", "A random delay before forwarding a mNS or mNA message (in milisec.!!!)",
			m_frwDelay);
	cmd.AddValue ("frwUseSrcAddr", "Whether to use src address differentiation during forwarding packet filtering",
				m_frwUseSrcAddr);
	cmd.AddValue ("ctrlNoOfRuns", "Whether to control number of runs to assess measurement accuracy - for HopLimi experiments",
					m_controlNoOfRuns);
	cmd.AddValue ("rnNoOfRuns", "No. of runs during experiments with random nodes selection",
						m_randomNodesNoOfRuns);
	cmd.AddValue ("newNodes", "Whether to use new nodes or old nodes with new addresses in random nodes exp.",
							m_newNodes);
	cmd.AddValue ("newAutoAddresses", "Whether to auto-assign a new address in case there is no valid address on the interface",
								m_newAutoAddresses);
	cmd.AddValue ("netAnim", "Whether to auto-assign a new address in case there is no valid address on the interface",
									m_useNetAnim);
	cmd.AddValue ("DadDelay", "Max. delay before DAD start (in seconds)", m_dadDelay);
	cmd.AddValue ("multiExpDupNoOfRuns", "Fixed no. of  runs in duplication experiment for one simulation data point", m_multiExpDupNoOfRuns);
	cmd.AddValue ("pauseTimeMax", "In mobility: max. time to wait before next run", m_pauseTimeMax);
	cmd.AddValue ("pauseTimeMin", "In mobility: min. time to wait before next run", m_pauseTimeMin);
	cmd.AddValue ("speed", "In mobility: speed (0 - walk, 1-run", m_speed);

	cmd.Parse (argc,argv);

	NS_ASSERT_MSG(m_noOfRuns%10 == 0 && m_noOfRuns <= m_maxNoOfRuns && m_multiExpDupNoOfRuns%10 ==0
			&& m_multiExpDupNoOfRuns <= m_maxNoOfRuns, "NUmber of runs must be the factor of "
			"10 and not greater than the specified max value");

	if(m_verbose != -1)
	{
		if (m_verbose)
		{
			//cos takeigo moze byc tez pomocne:
			//WifiHelper::EnableLogComponents
		  LogComponentEnable ("Ipv6Interface_Ndpp", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		  LogComponentEnable ("Icmpv6L4Protocol_Ndpp", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		  LogComponentEnable ("Icmpv6L4Protocol_NdSimple", (LogLevel)(LOG_LEVEL_LOGIC|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		  LogComponentEnable ("AnimationInterface", LOG_LEVEL_INFO);
		}
		else if (m_verbose == 0)
		{
		//	LogComponentEnable ("PropagationLossModel", (LogLevel)(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));

		  LogComponentEnable ("Ipv6Interface_Ndpp", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		  LogComponentEnable ("Icmpv6L4Protocol_Ndpp",(LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		  LogComponentEnable ("Icmpv6L4Protocol_NdSimple", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		  LogComponentEnable ("AnimationInterface", LOG_LEVEL_ERROR);
		}

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
	}
	else
	{
		//disable all logging
		LogComponentEnable("Ipv6Nd", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));

		LogComponentEnable ("Ipv6Interface_Ndpp", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		LogComponentEnable ("Icmpv6L4Protocol_Ndpp", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		LogComponentEnable ("Icmpv6L4Protocol_NdSimple", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		LogComponentEnable ("Ipv6L3Protocol", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		LogComponentEnable ("Ipv6L3Protocol_Ndpp", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ERROR);

		LogComponentEnable ("Icmpv6L4Protocol", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));

		LogComponentEnable ("Ipv6RawSocketImpl", LOG_LEVEL_ERROR);
		LogComponentEnable ("Ping6Application", LOG_LEVEL_ERROR);
		LogComponentEnable ("InternetStackHelper", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		LogComponentEnable ("InternetStackHelper_Nd", (LogLevel)(LOG_LEVEL_ERROR|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
		LogComponentEnable ("InternetTraceHelper", LOG_LEVEL_ERROR);
		LogComponentEnable ("AnimationInterface", LOG_LEVEL_ERROR);


	}

//	m_maxHopLimit = std::min((int) (m_nWifi * 1.5), m_maxHopLimit);

	//get input command for printing:
	for(int i=0; i<argc; i++)
	{
		m_inputCommand +=*argv;
		argv++;
	}
}

void NdppExperiment::InitSetup()
{
	 ////////////////////////////////////
	  //create wifi network:
	  ///////////////////////////////////////

	  //DONE: consider - to raczej nie ma wplywu, lepiej zostawic wieksze
	  // disable fragmentation for frames below 2200 bytes
	  //Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
	  // turn on RTS/CTS for all frames - to avoid hidden terminal problem - this does nothing with broadcasting!!!
	   Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("1"));

	m_wifi = new WifiHelper();

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
	  m_wifi->SetStandard(WIFI_PHY_STANDARD_80211a);
	  break;
	case Standard_802_11_b:
	  m_wifi->SetStandard(WIFI_PHY_STANDARD_80211b);
	  break;
	case Standard_802_11_n:
	  m_wifi->SetStandard(WIFI_PHY_STANDARD_80211n_2_4GHZ);
	  break;
	case Standard_802_11_g:
	default:
	  m_wifi->SetStandard(WIFI_PHY_STANDARD_80211g);
	  break;
	}

	m_wifi->SetRemoteStationManager("ns3::ConstantRateWifiManager",
		  "NonUnicastMode", StringValue(phyMode),
		  "DataMode", StringValue(phyMode),
		  "ControlMode", StringValue(phyMode)/*,
		  "MaxSsrc", UintegerValue(20),
		  "MaxSlrc", UintegerValue(20)*/);

	Config::SetDefault ("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue (1000));	//default - 400

	////////////////////////////////////////////////////////////////////////
	//*****************specify network details: *****************
	////////////////////////////////////////////////////////////////////////

	//double maxRange = (m_channelType == IDEAL)? 100 : 50;
	double maxRange = 50;
	//good for grid and cross-grid with log distance propagation model
	//for nakagami + log-distance probably some other value would be needed since nakagami brings random gain!!!

	//do not use default here! - this results in having 2 propagation loss models acting simultaneously (3.11)!
	m_channel = new YansWifiChannelHelper();
	if(m_channelType == IDEAL)
		m_channel->AddPropagationLoss("ns3::RangePropagationLossModel",
			  "MaxRange", DoubleValue(maxRange));
	else if(m_channelType == REAL)
	{
		/*m_channel->AddPropagationLoss("ns3::JakesPropagationLossModel");
		Config::SetDefault ("ns3::JakesProcess::NumberOfOscillators", UintegerValue (100));	//default 20
		Config::SetDefault ("ns3::JakesProcess::DopplerFrequencyHz", DoubleValue (200.0));	//default 80*/
		/*m_channel->AddPropagationLoss("ns3::NakagamiPropagationLossModel");
		Config::SetDefault ("ns3::NakagamiPropagationLossModel::m0", DoubleValue (1.0));
		Config::SetDefault ("ns3::NakagamiPropagationLossModel::m1", DoubleValue (1.0));
		Config::SetDefault ("ns3::NakagamiPropagationLossModel::m2", DoubleValue (1.0));*/

		/*m_channel->AddPropagationLoss("ns3::RangePropagationLossModel",
				"MaxRange", DoubleValue(maxRange));*/
		m_channel->AddPropagationLoss("ns3::LogDistancePropagationLossModel");
		// reference loss at 2.4 GHz is 40.045997
		Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceLoss", DoubleValue (40.045997));
	}
	else if(m_channelType == REAL2)
	{
		/*m_channel->AddPropagationLoss("ns3::LogDistancePropagationLossModel");
		// reference loss at 2.4 GHz is 40.045997
		Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceLoss", DoubleValue (40.045997));
*/
		/*m_channel->AddPropagationLoss("ns3::JakesPropagationLossModel");
		Config::SetDefault ("ns3::JakesProcess::NumberOfOscillators", UintegerValue (100));	//default 20
		Config::SetDefault ("ns3::JakesProcess::DopplerFrequencyHz", DoubleValue (200.0));	//default 80*/
		m_channel->AddPropagationLoss("ns3::NakagamiPropagationLossModel");
		Config::SetDefault ("ns3::NakagamiPropagationLossModel::m0", DoubleValue (1.0));
		Config::SetDefault ("ns3::NakagamiPropagationLossModel::m1", DoubleValue (1.0));
		Config::SetDefault ("ns3::NakagamiPropagationLossModel::m2", DoubleValue (1.0));

		m_channel->AddPropagationLoss("ns3::RangePropagationLossModel",
				"MaxRange", DoubleValue(maxRange));
	}

	m_channel->SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	m_phy = new YansWifiPhyHelper(YansWifiPhyHelper::Default ());	//this sets NistErrorRateModel
	// wifiPhy.Set ("RxGain", DoubleValue (0) );
	// wifiPhy.Set ("TxGain", DoubleValue (0) );

	Ssid ssid = Ssid ("monika");
	m_mac = new NqosWifiMacHelper(NqosWifiMacHelper::Default ());
	m_mac->SetType ("ns3::AdhocWifiMac",
			"Ssid", SsidValue (ssid));

	//////////////////////////////////////////////////////////
	//************** Mobility **************
	//////////////////////////////////////////////////////////

	double distance = maxRange/std::sqrt(2.0) - maxRange/20.0;
	int gridWidth = (int) ceil(sqrt(m_nWifi));
	double scaleFactor = 0.0;

	ObjectFactory pos, pos2;
	Ptr<PositionAllocator> posAlloc_moveDirection = 0;
	Ptr<UniformRandomVariable>  speed = 0;
	Ptr<UniformRandomVariable>  pause = 0;
	/* for(int i=0; i<nWifi; i++)
	{
	  mobilities.push_back(MobilityHelper());	//in general we need one helper per node to allow re-creating
	  //topology in each run from hierarchical mobility model (will be different for each node - associated
	  //with different position*/

	  switch(m_topologyType)
	  {
		case CrossGrid:
			NS_ASSERT_MSG(distance > maxRange/2, "CrossGrid is wrongly created!, maxRAnge value too small");
			pos.SetTypeId("ns3::GridPositionAllocator");
			pos.Set("MinX", DoubleValue (0.0));
			pos.Set("MinY", DoubleValue (0.0));
			pos.Set("DeltaX", DoubleValue (distance));
			pos.Set("DeltaY", DoubleValue (distance));
			pos.Set("GridWidth", UintegerValue (gridWidth));
			pos.Set("LayoutType", StringValue ("RowFirst"));
			m_positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
			m_mobility.SetPositionAllocator (m_positionAlloc);
			/* m_mobility.SetPositionAllocator("ns3::GridPositionAllocator",
				  "MinX", DoubleValue (0.0),
				  "MinY", DoubleValue (0.0),
				  "DeltaX", DoubleValue (distance),
				  "DeltaY", DoubleValue (distance),
				  "GridWidth", UintegerValue (gridWidth),
				  "LayoutType", StringValue ("RowFirst"));*/

			m_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
			break;
		case RandomRectangle:
			pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
			m_positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
			//set the rect size to max. probability of obtaining a connected graph (based on the cross-grid set-up)
			Names::Add("rRectPosAll", m_positionAlloc);
			Config::Set ("/Names/rRectPosAll/X/$ns3::UniformRandomVariable/Min",DoubleValue (0.0));
			Config::Set ("/Names/rRectPosAll/X/$ns3::UniformRandomVariable/Max",DoubleValue (gridWidth * distance));
			Config::Set ("/Names/rRectPosAll/Y/$ns3::UniformRandomVariable/Min",DoubleValue (0.0));
			Config::Set ("/Names/rRectPosAll/Y/$ns3::UniformRandomVariable/Max",DoubleValue (gridWidth * distance));
			m_mobility.SetPositionAllocator (m_positionAlloc);

			m_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
			break;
		case RandomDisc:
			pos.SetTypeId("ns3::RandomDiscPositionAllocator");
			pos.Set("X",  DoubleValue (0.0));
			pos.Set( "Y",  DoubleValue (0.0));
			pos.Set("Theta",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.2830]"));
			m_positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
			//set the disc size to max. probability of obtaining a connected graph (based on the cross-grid set-up)
			Names::Add("rDiscPosAll", m_positionAlloc);
			Config::Set ("/Names/rDiscPosAll/Rho/$ns3::UniformRandomVariable/Min",DoubleValue (0.0));
			Config::Set ("/Names/rDiscPosAll/Rho/$ns3::UniformRandomVariable/Max",
					DoubleValue (gridWidth * distance / 2.2));
			//pos.Set("Rho", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"));
			m_mobility.SetPositionAllocator (m_positionAlloc);

			m_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
			break;
		case UniformDisc:
			pos.SetTypeId("ns3::UniformDiscPositionAllocator");
			pos.Set("X",  DoubleValue (0.0));
			pos.Set("Y",  DoubleValue (0.0));
			scaleFactor = (m_channelType == IDEAL)? 2.5 : 2.5; //some approx. to estimate the disc size
			//to max. the probability of getting a connected graph
			pos.Set( "rho",  DoubleValue (gridWidth * distance / scaleFactor));
			m_positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
			m_mobility.SetPositionAllocator (m_positionAlloc);

			m_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
			break;
		case RandomWalk:
			m_mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
									   "Bounds", RectangleValue (Rectangle (0, 0, 200, 200)));
			//TODO: some additonal params may need to be specified
			break;
		case RandomWaypoint:
			m_withMobility = true;
			//pa for setting node direction (next position after move)
			pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
			posAlloc_moveDirection = pos.Create ()->GetObject<PositionAllocator> ();
			Names::Add("rRectPosAll", posAlloc_moveDirection);
			Config::Set ("/Names/rRectPosAll/X/$ns3::UniformRandomVariable/Min",DoubleValue (0.0));
			Config::Set ("/Names/rRectPosAll/X/$ns3::UniformRandomVariable/Max",DoubleValue (gridWidth * distance));
			Config::Set ("/Names/rRectPosAll/Y/$ns3::UniformRandomVariable/Min",DoubleValue (0.0));
			Config::Set ("/Names/rRectPosAll/Y/$ns3::UniformRandomVariable/Max",DoubleValue (gridWidth * distance));

			//pa for setting initial node layout
			pos2.SetTypeId("ns3::GridPositionAllocator");
			pos2.Set("MinX", DoubleValue (0.0));
			pos2.Set("MinY", DoubleValue (0.0));
			pos2.Set("DeltaX", DoubleValue (distance));
			pos2.Set("DeltaY", DoubleValue (distance));
			pos2.Set("GridWidth", UintegerValue (gridWidth));
			pos2.Set("LayoutType", StringValue ("RowFirst"));
			m_positionAlloc = pos2.Create ()->GetObject<PositionAllocator> ();

			speed = CreateObject<UniformRandomVariable> ();
			if(m_speed == Walking)
			{
				speed->SetAttribute("Min", DoubleValue(0.83));
				speed->SetAttribute("Max", DoubleValue(1.94));
			}
			else
			{
				speed->SetAttribute("Min", DoubleValue(2.78));
				speed->SetAttribute("Max", DoubleValue(9.72));
			}

			pause = CreateObject<UniformRandomVariable> ();	//can be constant as well!!!
			pause->SetAttribute("Min", DoubleValue(m_pauseTimeMin));
			pause->SetAttribute("Max", DoubleValue(m_pauseTimeMax));

			m_mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
										  "Speed", PointerValue(speed),
										  "Pause", PointerValue(pause),
										  "PositionAllocator", PointerValue (posAlloc_moveDirection));	//set node destinations after move
			m_mobility.SetPositionAllocator (m_positionAlloc);	//set initial node position
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
			m_positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
			m_mobility.SetPositionAllocator (m_positionAlloc);

			m_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		  break;
	  }

	  ///////////////////////////
	  //***** address helper:
	  ///////////////////////////
	  m_addresses = new Ipv6AddressHelper();


	  ///////////////////////////
	  //***** NDPP_FRW count limits:
	  ///////////////////////////
	  if(m_ndType == InternetStackHelper_Nd::ND_NDPP_FRW ||
			  m_ndType == InternetStackHelper_Nd::ND_NDPP_MPR || m_ndType == InternetStackHelper_Nd::ND_NDPP_MPR2)
	  {
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdppFrw::FrwCountLimit",
				  UintegerValue (m_frwCountLimit));
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdppFrw::DADReplyCountLimit",
		 			  UintegerValue (m_dadReplyCountLimit));
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdppFrw::frwUseSrcAddr",
		  		 			  BooleanValue (m_frwUseSrcAddr));
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdppFrw::autoAssignNewAddr",
		  		  		 			  BooleanValue (m_newAutoAddresses));
	  }
	  if(m_ndType != InternetStackHelper_Nd::ND_BASIC && m_ndType != InternetStackHelper_Nd::ND_SIMPLE)
	  {
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_Ndpp::nDADcount",
				  UintegerValue (m_nDADcountLimit));

		  NS_ASSERT_MSG(m_mprStartDelay <= m_mprpDelay, "MprStratDelay is bigger than MprDelay - "
				  "this does not have practical sense, please verify!");

		  //remember to set this value in icmpv6l4protocol too!!!!
		  Config::SetDefault ("ns3::Ipv6Interface_Ndpp::MPRP_DELAY",
		  				  DoubleValue (m_mprpDelay));
		  Ptr<UniformRandomVariable>  mprStartDelay = CreateObject<UniformRandomVariable> ();
		  mprStartDelay->SetAttribute("Min", DoubleValue(0.0));
		  mprStartDelay->SetAttribute("Max", DoubleValue(m_mprStartDelay));
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_Ndpp::MprStartDelay", PointerValue(mprStartDelay));

		  Ptr<UniformRandomVariable> dadDelay = CreateObject<UniformRandomVariable> ();
		  dadDelay->SetAttribute ("Min", DoubleValue (0.0));
		  dadDelay->SetAttribute ("Max", DoubleValue (m_dadDelay));
		  Config::SetDefault ("ns3::Icmpv6L4Protocol::DadDelay", PointerValue(dadDelay));
	  }
	  if(m_ndType != InternetStackHelper_Nd::ND_BASIC)
	  {
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::RETRANS_TIMER_NDAD",
				  UintegerValue (m_retransTimerNDad));
		  Ptr<UniformRandomVariable>  frwDelay = CreateObject<UniformRandomVariable> ();
		  frwDelay->SetAttribute("Min", DoubleValue(0.0));
		  frwDelay->SetAttribute("Max", DoubleValue(m_frwDelay));
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::FrwDelay",
		  				  PointerValue(frwDelay));
	  }

	  ///////////////////////////
	  //***** experiments with random no of initiating nodes:
	  ///////////////////////////

	  if(m_noOfRandomNodes != 0)
	  {
		  m_randomNodesSelector = CreateObject<UniformRandomVariable> ();
		  //m_selectedNodes = new std::list<int> ();
	  }


	  ///////////////////////////
	  //***** results collection:
	  ///////////////////////////

	  mkdir(("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/simul_results/results/" + m_resultGroupName).c_str(),
	  			S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	  m_results.EnableResultsCollection("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/simul_results/results/"
			  + m_resultGroupName + "/ndpp", m_resultFileSuffix);

	  ///////////////////////////
	 	  //***** Net Anim:
	 	  ///////////////////////////

	  AnimationInterface_Ndpp::SetMaxRangeForDrawingTopology(maxRange);
	  //AnimationInterface_Ndpp::SetMaxRange(79.1528882);
	  AnimationInterface::SetBoundary(0, 0, 200, 200);

}

void NdppExperiment::Run()
{
	//.............................................................................
	//Simulation:
	//.............................................................................

	NS_LOG_UNCOND("Start ND demo");

	if (m_ndType == InternetStackHelper_Nd::ND_BASIC)
	{
		NS_LOG_UNCOND("standard ND chosen for simulation - no experiments available, QUIT");
		return;
	}

	switch(m_resultType)
	{
	case SingleRunFullTxRxTables:
	case SingleRun:
		DoRunSelectedNodeSingleRunExp();
		break;
	case HopLimitTest:
		DoRunHopLimit();
		break;
	case mNSMessageLifetime:
	case HopLimitSelectedNode:
		DoRunHopLimitSelectedNodeLong();
		break;
	case DupDetection:
		DoRunDupDetectionExp();
		break;
	case DupProbability:
		DoRunDupProbability();
		break;
	case HopLimitRandomNodes:
	//	DoRunHopLimitRandomNodes();
		break;
	case mNSMessageLifetimeRandomNodes:
	case mNSMessageCountRandomNodes:
		DoRunRandomNodes();
		break;
	case DupProbRandomNodes:
		DoRunDupProbRandomNode();
		break;
	case MultiExpRN:
	case MultiExpNoDupRN:
		DoRunMultiExpRandomNodes();
		break;
	default:
		break;
	}

	AnimationInterface_Ndpp::Cleanup();
}

void NdppExperiment::DoRunHopLimit()
{

	double t = SelectTparam(m_noOfRuns);

	double accuracy = 0;
	if(m_resultType == mNSMessageLifetime)
		accuracy = m_accuracyML;
	else
		accuracy = m_accuracyHL;
	double currAccuracy = 0.0;
	double confidenceInt = 0.0;

	for(int hopLimit = m_initialHopLimit; hopLimit <= m_maxHopLimit; hopLimit += m_hlStep)
	{
	  NS_LOG_UNCOND("Run with HopLimit = " << hopLimit);

	  //RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
									 //puli niezaleznych liczb losowych

	  for(int j=0; j < m_nWifi; j++)
	  {
		  NS_LOG_UNCOND("Run for node " << j << " with HopLimit = " << hopLimit);
		  for(int k=0; k < m_noOfRuns; k++)
		  {
			  NS_LOG_UNCOND("Run no " << k << " for node " << j << " with HopLimit = " << hopLimit);

			  //set HopLimit for this run:
			  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (hopLimit));

			  DoRunSelectedNodeSingleRun(j, -1, false);
		  }

		  //check if we are still in a reasonable "point of operation"
		  //zalozenie: dokladnosc 10% na poziomie ufnosci 95%
		  confidenceInt = t * m_results.m_average_mNSolCount_singleRun.Stddev() / std::sqrt((double)m_noOfRuns);
		  currAccuracy = (confidenceInt > 0)? m_results.m_average_mNSolCount_singleRun.Avg() / confidenceInt
		  			  : 1000.0;	//1000 is supposed to be greater than any accuracy that can be assumed

		  if(m_resultType == mNSMessageLifetime)
		  {
			  m_results.SetConfidenceIntML(confidenceInt);
			  m_results.SetAccuracyML(currAccuracy);
		  }
		  else
		  {
			  m_results.SetConfidenceIntHL(confidenceInt);
			  m_results.SetAccuracyHL(currAccuracy);
		  }


		  if(currAccuracy < accuracy)
		  {
			  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! - stop here:" << std::endl
					  << "Node " << j << " with HopLimit = " << hopLimit);
			  //reset just for safety - we quit anyway
			  m_results.ResetAverageStatsSingleRun();
			  m_results.ResetStats();
			  return;
		  }

		  //***********************cumulative results collection:
		  m_results.UpdateHopLimitStats();
		  //reset before next run:
		  m_results.ResetAverageStatsSingleRun();

		  //internetv6->Reset();
		//  SetInterfacesDown(wifiDevices);

		  //update stats
		  //internetv6->ResetDadStats();
	  }
	  //***************************Print results - one line for this hop limit
	  m_results.PrintResultHopLimit(m_inputCommand, hopLimit, m_noOfRuns);
	  m_results.ResetStats();
	}
}

void NdppExperiment::DoRunHopLimitSelectedNode()
{

	double t = SelectTparam(m_noOfRuns);
	double accuracy = 0;
	if(m_resultType == mNSMessageLifetime)
		accuracy = m_accuracyML;
	else
		accuracy = m_accuracyHL;
	double currAccuracy = 0.0;
	double confidenceInt = 0.0;
	bool resetStreams;

	for(int hopLimit = m_initialHopLimit; hopLimit <= m_maxHopLimit; hopLimit += m_hlStep)
	{
	  //RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
									 //puli niezaleznych liczb losowych

	  //set node to run experiment for:
	  int j = 8; // czyli 9 liczac od 1
	  resetStreams = true;

	  NS_LOG_UNCOND("Run with HopLimit = " << hopLimit << " (for node " << j << ")" << std::endl);

	  for(int k = 0; k < m_noOfRuns; k++)
	  {
		  NS_LOG_UNCOND("Run no " << k << " for node " << j << " with HopLimit = " << hopLimit);

		  //set HopLimit for this run:
		  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (hopLimit));
		  DoRunSelectedNodeSingleRun(j, -1, resetStreams);
		  resetStreams = false;
	  }

	  //check if we are still in a reasonable "point of operation"
	  //zalozenie: dokladnosc 10% na poziomie ufnosci 95%
	  confidenceInt = t * m_results.m_average_mNSolCount_singleRun.Stddev() / std::sqrt((double)m_noOfRuns);
	  currAccuracy = (confidenceInt > 0)? m_results.m_average_mNSolCount_singleRun.Avg() / confidenceInt
			  : 1000.0;	//1000 is supposed to be greater than any accuracy that can be assumed

	  if(m_resultType == mNSMessageLifetime)
	  {
		  m_results.SetConfidenceIntML(confidenceInt);
		  m_results.SetAccuracyML(currAccuracy);
	  }
	  else
	  {
		  m_results.SetConfidenceIntHL(confidenceInt);
		  m_results.SetAccuracyHL(currAccuracy);
	  }

	  if(currAccuracy < accuracy)
	  {
		  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! - stop here:" << std::endl
				  << "Node " << j << " with HopLimit = " << hopLimit << " (currAccuracy = " << currAccuracy
				  << ", accuracy = " << accuracy << ")" << std::endl);
		  //reset just for safety - we quit anyway
		  m_results.ResetAverageStatsSingleRun();
		  return;
	  }

	  //***************************Print results - one line for this hop limit
	  m_results.PrintPartialResultHLmNSSingleRun(m_inputCommand, hopLimit, m_noOfRuns);

	  //reset before next run:
	  m_results.ResetAverageStatsSingleRun();

	  //internetv6->Reset();
	  //SetInterfacesDown(wifiDevices);
	}
}

void NdppExperiment::DoRunHopLimitSelectedNodeLong()
{
	int hopLimit = m_initialHopLimit;
	int noOfRuns;

	do
	{
	  //RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
									 //puli niezaleznych liczb losowych

	  //set node to run experiment for:
	  int j = m_initiatorNo; // bylo tu 8 czyli 9 liczac od 1 -> tu byla zmiana!!!! (10.03.2015)

	  NS_LOG_UNCOND("Run with HopLimit = " << hopLimit << " (for node " << j << ")" << std::endl);

	  //set HopLimit for this run set:
	  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (hopLimit));
	  noOfRuns = DoRunSelectedNodeSingleRunLong(j, m_targetNo, true, m_controlNoOfRuns);



	  ////////////////////****************************************//////////////////////////////
	  //***************************Print results - one line for this hop limit
	  ////////////////////****************************************//////////////////////////////

	  if(m_resultType == mNSMessageLifetime)
	  {
		  m_results.PrintPartialResultMlifeSingleRun(m_inputCommand, hopLimit, noOfRuns);

		  //reset before next run:
		  m_results.ResetAverageStatsSingleRun();
	  }
	  else
	  {
		  m_results.PrintPartialResultHLmNSSingleRun(m_inputCommand, hopLimit, noOfRuns);

		  //reset before next run:
		  m_results.ResetAverageStatsSingleRun();
	  }

	  //internetv6->Reset();
	  //SetInterfacesDown(wifiDevices);
	  hopLimit += m_hlStep;
	}
	while (hopLimit <= m_maxHopLimit);
}

/*void NdppExperiment::DoRunHopLimitRandomNodes()
{
	int hopLimit = m_initialHopLimit;
	bool resetStreams = true;
	//int noOfRuns;

	do
	{
	  //RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
									 //puli niezaleznych liczb losowych

	  //set HopLimit for this run set:
	  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (hopLimit));

	  for(int i = 0; i < m_randomNodesNoOfRuns; i++)
	  {
		  DoRunRandomNodesSingleRun(m_targetNo, resetStreams, m_controlNoOfRuns);

		  if(m_resultType == mNSMessageLifetimeRandomNodes)
			  m_results.UpdateMessageLifetimeStats();
		  else
			  m_results.UpdateHopLimitStats();
		  //reset before next run:
		  m_results.ResetAverageStatsSingleRun();

		  //TODO - this changes topology!!!!!
		  resetStreams = false;	//the streams are not reset to the initial state within this loop
		  	  	  	  	  	  	//BUT new streams are chosen - the ones following in order the previous ones.
		  	  	  	  	  	  	// this is to avoid looping of a stream during long single simulation run, there are enough streams to handle that
	  }

*/
	  ////////////////////****************************************//////////////////////////////
	  //***************************Print results - one line for this hop limit
	  ////////////////////****************************************//////////////////////////////
/*
	  if(m_resultType == mNSMessageLifetimeRandomNodes)
		  m_results.PrintResultMessageLifetime(m_inputCommand, hopLimit);
	  else
		  m_results.PrintResultHopLimit(m_inputCommand, hopLimit);

	  //reset before next run:
	  m_results.ResetStats();

	  //internetv6->Reset();
	  //SetInterfacesDown(wifiDevices);
	  resetStreams = true;

	  hopLimit += m_hlStep;
	}
	while (hopLimit <= m_maxHopLimit);
}*/

void NdppExperiment::DoRunRandomNodes()
{
	//RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
									 //puli niezaleznych liczb losowych

	int noOfRuns = 0;
	  //set HopLimit :
	  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (m_initialHopLimit));

	  for(int i = 0; i < m_randomNodesNoOfRuns; i++)
	  {
		  noOfRuns = DoRunRandomNodesSingleRun(true, m_controlNoOfRuns);

		  if(m_resultType == mNSMessageLifetimeRandomNodes)
		  {
			  m_results.UpdateMessageLifetimeStats();
			  m_results.PrintPartialResultMlifeSingleRun(m_inputCommand,m_initialHopLimit, noOfRuns);
		  }
		  else
		  {
			  m_results.UpdateHopLimitStats();
			  m_results.PrintPartialResultHLmNSSingleRun(m_inputCommand,m_initialHopLimit, noOfRuns);
		  }
		  //reset before next run:
		  m_results.ResetAverageStatsSingleRun();
	  }


	  ////////////////////****************************************//////////////////////////////
	  //***************************Print results - summary from all variants of node selection
	  ////////////////////****************************************//////////////////////////////

	  m_results.ResetResultPrintInit();
	  if(m_resultType == mNSMessageLifetimeRandomNodes)
		  m_results.PrintResultMessageLifetime("", m_initialHopLimit);
	  else
		  m_results.PrintResultHopLimit("", m_initialHopLimit);

	  //reset before next run:
	  m_results.ResetStats();
}

void NdppExperiment::DoRunSelectedNodeSingleRunExp()
{
	m_noOfRuns = 1;	//just for safety
	//set HopLimit for this run:
	Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (m_initialHopLimit));

	DoRunSelectedNodeSingleRun(m_initiatorNo, -1, false);

	//***************************Print results - one line for this hop limit
	m_results.PrintPartialResultHLmNSSingleRun(m_inputCommand, m_initialHopLimit);
	m_results.ResetAverageStatsSingleRun();
}

void NdppExperiment::DoRunSelectedNodeSingleRun(uint32_t currentNode, int32_t targetNode, bool resetStreams)
{
	NodeContainer* wifiNodes;
	NetDeviceContainer* wifiDevices;
	InternetStackHelper_Nd* internetv6;
	Ipv6InterfaceContainer* wifiInterfaces;

	if(resetStreams)
	{
		m_streamsUsed = 0;
		m_extraStreamNo = MID_STREAM_NO;
	}


	//*********************create nodes*********************:
	wifiNodes = new NodeContainer();
	wifiNodes->Create (m_nWifi);	//creates nWifi+1 nodes (??? - nwifi nodes)


	//*********************** create m_channel:
	Ptr<YansWifiChannel> chan = m_channel->Create ();
	m_phy->SetChannel(chan);
	/*in JAkes/nakagami/other models there are additional  streams used and this changes topology, since different strams are used to
	 * create it in position allocator. The aim here is to assign some large stream number to this model and keep
	 * the same numbering for all other streams in the simulation
	 */
	if(m_channelType != IDEAL)
		m_extraStreamNo += m_channel->AssignStreams (chan, m_extraStreamNo);	//quick solver of topology issue
	else
		m_streamsUsed += m_channel->AssignStreams (chan, m_streamsUsed);


	//********create interfaces and link the interfaces for given nodes to the wifi network:
	wifiDevices = new NetDeviceContainer(m_wifi->Install (*m_phy, *m_mac, *wifiNodes));

	// Assign fixed stream numbers to wifi and m_channel random variables
	m_streamsUsed += m_wifi->AssignStreams (*wifiDevices, m_streamsUsed);	//blocks PHY, Station managers, MAC

	/* //trace topology changes - to checkif topology is the same for each run:
	for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
	   (*it)->TraceConnectWithoutContext("CourseChange", MakeCallback (&CourseChange));*/


	//**********************mobility & topology:
	m_streamsUsed += m_positionAlloc->AssignStreams (m_streamsUsed);
	m_mobility.Install(*wifiNodes);
	//// Assign fixed stream numbers to mobility model, position allocator has to be handled separately
	m_streamsUsed += m_mobility.AssignStreams (*wifiNodes, m_streamsUsed);

	Ptr<MobilityModel> model;
	TopologyI it_top;
	if(m_mobilityHierarchyInit)
	{
	   NS_ASSERT_MSG(m_topology.size() == wifiNodes->GetN(), "Topology vector size different "
			   "from node container size!");
	   it_top = m_topology.begin();
	   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
	   {
		   model = (*it)->GetObject<MobilityModel> ();
		   model->SetPosition(*it_top);
		   it_top++;
	   }
	}
	else
	{
	   m_mobilityHierarchyInit = true;
	   for(NodeContainer::Iterator it = wifiNodes->Begin(); it != wifiNodes->End(); ++it)
	   {
		   //remember initial topology setup for each node
		   model = (*it)->GetObject<MobilityModel> ();
		   m_topology.push_back(model->GetPosition());
	   }
	}


	//*********************internet stack:
	internetv6 = new InternetStackHelper_Nd (m_ndType);
	internetv6->SetIpv4StackInstall(false);
	internetv6->InstallAll();
	// internetv6->Install(*wifiNodes);	///moze tak by bylo lepiej???

	//fix IPv6EXtensions and ICMPv6L4Protocol streams and IPv6Interface (dadDelay and MPR Start delay)
	//(plus others most likely not used in my simulations)
	m_streamsUsed += internetv6->AssignStreams (*wifiNodes, m_streamsUsed);

	if(m_resultType == SingleRunFullTxRxTables || m_resultType == DupDetection)
	{
		internetv6->EnableAsciiIcmpv6All("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/simul_results/ipv6-nd-RxTxStats");
		m_phy->EnablePcapAll ("/home/monika/Workspaces/ns-3-update-nd++/ns-3.18/pcaps/ipv6-nd", true);
	}
	else
		internetv6->EnableAsciiIcmpv6All();	//this has to be done before address assignment
									   //otherwise first MPR messages are not counted

	wifiInterfaces = new Ipv6InterfaceContainer(m_addresses->Assign(*wifiDevices, std::vector<bool> (m_nWifi, false)));	//this also sets interfaces up
	//UWAGA!!! - po wywolaniu powyzej startuja MPRy (MPRsInit) - to jest ok, schedule wysylanai odpalany po Simulator::Run


	//monitor if random numbers are independent throughout simulation
	NS_ASSERT_MSG(m_streamsUsed < MID_STREAM_NO, "No. of RNG streams close to end, fix RNs!!!!");
	NS_ASSERT_MSG(m_extraStreamNo < MAX_STREAM_NO, "No. of RNG streams close to end, fix RNs (extraStreamNo overflow)!!!!");

	//****************************simulator setup:
	//for large networks and big HL the messages form the bootstrapping phase are in a network for a long time -
	//the new address has to be assigned late, for safety reasons!!!!
	Simulator::Schedule(Seconds(60.0), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
	if(targetNode == -1)
		Simulator::Schedule(Seconds(60.5), &NdppExperiment::AddAddress, this, wifiInterfaces, wifiDevices, currentNode);
	else
		Simulator::Schedule(Seconds(60.5), &NdppExperiment::AddDuplicatedAddress, this,
				wifiInterfaces, wifiDevices, currentNode, targetNode);

	Simulator::Schedule(Seconds(150.0), &NdppExperiment::SetInterfacesDown, this, wifiDevices);
	Simulator::Stop (Seconds (153.0));

	//*********************main run:
	Simulator::Run ();

	//*************************results collection:
	m_results.UpdateHopLimitStatsSingleRun(internetv6);
	if(m_resultType == SingleRunFullTxRxTables || m_resultType == DupDetection)
		internetv6->PrintResultsTxRxStats(m_inputCommand);

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
	delete internetv6;
	delete wifiDevices;
	delete wifiNodes;
}

int NdppExperiment::DoRunSelectedNodeSingleRunLong(uint32_t currentNode, int32_t targetNode,
		bool resetStreams, bool controlNoOfRuns)
{
	NodeContainer* wifiNodes = 0;
	NetDeviceContainer* wifiDevices = 0;
	InternetStackHelper_Nd* internetv6 = 0;
	Ipv6InterfaceContainer* wifiInterfaces = 0;

	if(resetStreams)
	{
		m_streamsUsed = 0;
		m_extraStreamNo = MID_STREAM_NO;
	}

	SingleRunInit(wifiNodes, wifiDevices, internetv6, wifiInterfaces);

	//****************************confidence intervals:*****************///
	double accuracy, currAccuracy, confidenceInt, t = 0.0;
	int noOfRuns = m_noOfRuns;
	if(controlNoOfRuns)
	{
		t = SelectTparam(noOfRuns);
		currAccuracy = confidenceInt = 0.0;
		if(m_resultType == mNSMessageLifetime)
			accuracy = m_accuracyML;
		else
			accuracy = m_accuracyHL;
	}

	//****************************simulator setup:
	//for large networks and big HL the messages form the bootstrapping phase are in a network for a long time -
	//the new address has to be assigned late, for safety reasons!!!!
	int totalExpCycleTime = 60;
	int initPhaseTime = 110;
	int timeAdv = 0;

	int kCounter = m_noOfRuns;
	bool needMoreRuns = false;

	do
	{
		//UWAGA!!!!!! musi byc >10s przerwy pomiedzy interface down i up -> f. WifiMacQueue::Cleanup czysci stare
		//pakiety, ktore pomimo zakolejkownia nei zostana wyslane. Czyszczenie po czasie m_maxDelay
		//ustawianymjako atrybut WifiMacQueue (defaultowow 10s)!!!!!!!!!!!!!!!!!!!!!!!!
		for(int k = 0; k < kCounter; k++)
		{
			if(k == 0 && !needMoreRuns)
			{
				//first round is longer, because of the bootstrapping phase
				timeAdv = initPhaseTime;
			}
			else
			{
				timeAdv = initPhaseTime + totalExpCycleTime * k;
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 1.0)), &NdppExperiment::SetInterfacesUp, this, wifiDevices);
			}

			Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 9.5)), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
			if(targetNode == -1)
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::AddAddress, this, wifiInterfaces, wifiDevices, currentNode);
			else
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::AddDuplicatedAddress, this,
						wifiInterfaces, wifiDevices, currentNode, targetNode);

			Simulator::Schedule(Seconds(timeAdv - 14.0), &InternetStackHelper_Nd::DisableIcmpv6AsciiTracing, internetv6);
			if (targetNode == -1)
				Simulator::Schedule(Seconds(timeAdv - 13.0), &NdppExperiment::RemoveAddress, this, wifiDevices, currentNode);
			else
				Simulator::Schedule(Seconds(timeAdv - 13.0), &NdppExperiment::RemoveDuplicatedAddress, this, wifiDevices, currentNode, targetNode);
			Simulator::Schedule(Seconds(timeAdv - 12.0), &NdppExperiment::SetInterfacesDown, this, wifiDevices);

			//*************************results collection:
			switch(m_resultType)
			{
			case DupProbability:
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateDupDetectCount,
							&m_results, internetv6, currentNode);
				break;
			case mNSMessageLifetime:
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateMLifeStatsSingleRun,
							&m_results, internetv6, true, false);
				break;
			case HopLimitSelectedNode:
			default:
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateHopLimitStatsSingleRun,
							&m_results, internetv6, true, false);
				break;
			}

			Simulator::Schedule(Seconds(timeAdv - 0.5), &InternetStackHelper_Nd::ResetDadStats, internetv6);

				 /* Simulator::Schedule(Seconds(30.0), &ns3::NS_LOG_UNCOND,
				  std::string("Run no " << k << " for node " << currentNode << std::endl);*/
		}

		Simulator::Stop (Seconds (timeAdv));

		//*********************main run:
		Simulator::Run ();

		 //////////////////////****************************************////////////////////////////////////////
		//we go here after the time set by simulator::Stop
		//check if we are in a reasonable "point of operation"
		//zalozenie: dokladnosc m_accuracy na poziomie ufnosci 95%,jesli mniejsza - > ilosc runow o 10
		if(controlNoOfRuns)
		{
			kCounter = 10;
			if(m_resultType == mNSMessageLifetime)
			{
			  confidenceInt = t * m_results.m_average_mNSlife_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			  currAccuracy = (confidenceInt > 0)? m_results.m_average_mNSlife_singleRun.Avg() / confidenceInt
						  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			}
			else
			{
			  confidenceInt = t * m_results.m_average_mNSolCount_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			  currAccuracy = (confidenceInt > 0)? m_results.m_average_mNSolCount_singleRun.Avg() / confidenceInt
						  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			}

			if(currAccuracy < accuracy)
			{
				noOfRuns += 10;
				if(noOfRuns <= m_maxNoOfRuns)
				{
				  //make another 10 runs and check again
				  t = SelectTparam(noOfRuns);
				  needMoreRuns = true;
				  initPhaseTime = totalExpCycleTime;	//init phase same as other cycles, note that Simualtor::Schedule schedules at
				  //the  given delay time after the CURRENT simulation time which is "remembered" always before Distroy!!!!

				  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! for noOfRuns: " << noOfRuns - 10
						  << ". Node " << currentNode << " (currAccuracy = " << currAccuracy
						  << ", accuracy = " << accuracy << "). Running another 10..." << std::endl);
				}
				else
				{
				  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! (m_noOfRuns > MAX) - stop here:" << std::endl
						  << "Node " << currentNode << " (noOfRuns = " << noOfRuns - 10 <<
						  ", currAccuracy = " << currAccuracy << ", accuracy = " << accuracy << ")" << std::endl);

				  needMoreRuns = false; 	//just for safety
				  break;
				}
			}
			else
			{
				//register conf interval and accuracy level
				if(m_resultType == mNSMessageLifetime)
				{
				  m_results.SetConfidenceIntML(confidenceInt);
				  m_results.SetAccuracyML(currAccuracy);
				}
				else
				{
				  m_results.SetConfidenceIntHL(confidenceInt);
				  m_results.SetAccuracyHL(currAccuracy);
				}

				needMoreRuns = false;
			}
		}
	}
	while(needMoreRuns);

	//*************************results collection:
	if((m_resultType == SingleRunFullTxRxTables || m_resultType == DupDetection) && m_noOfRuns == 1)
		internetv6->PrintResultsTxRxStats(m_inputCommand);


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
	delete internetv6;
	delete wifiDevices;
	delete wifiNodes;

	return noOfRuns;
}

/*void NdppExperiment::DoRunRandomNodesSingleRun(int32_t targetNode, bool resetStreams,  bool controlNoOfRuns)
{
	NodeContainer* wifiNodes = 0;
	NetDeviceContainer* wifiDevices = 0;
	InternetStackHelper_Nd* internetv6 = 0;
	Ipv6InterfaceContainer* wifiInterfaces = 0;

	if(resetStreams)
	{
		m_streamsUsed = 0;
		m_extraStreamNo = MID_STREAM_NO;
	}

	SingleRunInit(wifiNodes, wifiDevices, internetv6, wifiInterfaces);

*/	//****************************confidence intervals:*****************///
/*	double accuracy, currAccuracy, confidenceInt, t = 0.0;
	int noOfRuns = m_noOfRuns;
	if(controlNoOfRuns)
	{
		t = SelectTparam(noOfRuns);
		currAccuracy = confidenceInt = 0.0;
		if(m_resultType == mNSMessageLifetimeRandomNodes)
			accuracy = m_accuracyML;
		else
			accuracy = m_accuracyHL;
	}

*/	//****************************simulator setup:
	//for large networks and big HL the messages from the bootstrapping phase are in a network for a long time -
	//the new address has to be assigned late, for safety reasons!!!!
/*	int totalExpCycleTime = 60;
	int initPhaseTime = 110;
	int timeAdv = 0;

	int kCounter = m_noOfRuns;
	bool needMoreRuns = false;

	do
	{
		//UWAGA!!!!!! musi byc >10s przerwy pomiedzy interface down i up -> f. WifiMacQueue::Cleanup czysci stare
		//pakiety, ktore pomimo zakolejkownia nei zostana wyslane. Czyszczenie po czasie m_maxDelay
		//ustawianymjako atrybut WifiMacQueue (defaultowow 10s)!!!!!!!!!!!!!!!!!!!!!!!!
		for(int k = 0; k < kCounter; k++)
		{
			if(k == 0 && !needMoreRuns)
			{
				//first round is longer, because of the bootstrapping phase
				timeAdv = initPhaseTime;
			}
			else
			{
				timeAdv = initPhaseTime + totalExpCycleTime * k;
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 1.0)), &NdppExperiment::SetInterfacesUp, this, wifiDevices);
			}

			Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 9.5)), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
			if(targetNode == -1)
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::AddAddressRandomNodes, this,
						wifiInterfaces, wifiDevices);
			else
			{
				//TODO
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::AddDuplicatedAddress, this,
						wifiInterfaces, wifiDevices, currentNode, targetNode);
			}
			Simulator::Schedule(Seconds(timeAdv - 14.0), &InternetStackHelper_Nd::DisableIcmpv6AsciiTracing, internetv6);
			if (targetNode == -1)
				Simulator::Schedule(Seconds(timeAdv - 13.0), &NdppExperiment::RemoveAddressRandomNodes, this, wifiDevices);
			else
			{
				//TODO
				//Simulator::Schedule(Seconds(timeAdv - 13.0), &NdppExperiment::RemoveDuplicatedAddress, this, wifiDevices, currentNode, targetNode);
			}
			Simulator::Schedule(Seconds(timeAdv - 12.0), &NdppExperiment::SetInterfacesDown, this, wifiDevices);

	*/		//*************************results collection:
/*			switch(m_resultType)
			{
			case DupProbRandomNodes:
				//TODO
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateDupDetectCount,
							&m_results, internetv6, currentNode);
				break;
			case mNSMessageLifetimeRandomNodes:
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateMLifeStatsSingleRun,
							&m_results, internetv6);
				break;
			case HopLimitRandomNodes:
			default:
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateHopLimitStatsSingleRun,
							&m_results, internetv6);
				break;
			}

			Simulator::Schedule(Seconds(timeAdv - 0.5), &InternetStackHelper_Nd::ResetDadStats, internetv6);

				  Simulator::Schedule(Seconds(30.0), &ns3::NS_LOG_UNCOND,
				  std::string("Run no " << k << " for node " << currentNode << std::endl);
		}

		Simulator::Stop (Seconds (timeAdv));
*/
		//*********************main run:
/*		Simulator::Run ();
*/
		 //////////////////////****************************************////////////////////////////////////////
		//we go here after the time set by simulator::Stop
		//check if we are in a reasonable "point of operation"
		//zalozenie: dokladnosc m_accuracy na poziomie ufnosci 95%,jesli mniejsza - > ilosc runow o 10
/*		if(controlNoOfRuns)
		{
			kCounter = 10;
			if(m_resultType == mNSMessageLifetimeRandomNodes)
			{
			  confidenceInt = t * m_results.m_average_mNSlife_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			  currAccuracy = (confidenceInt > 0)? m_results.m_average_mNSlife_singleRun.Avg() / confidenceInt
						  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			}
			else
			{
			  confidenceInt = t * m_results.m_average_hopLimit_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			  currAccuracy = (confidenceInt > 0)? m_results.m_average_hopLimit_singleRun.Avg() / confidenceInt
						  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			}

			if(currAccuracy < accuracy)
			{
				noOfRuns += 10;
				if(noOfRuns <= m_maxNoOfRuns)
				{
				  //make another 10 runs and check again
				  t = SelectTparam(noOfRuns);
				  needMoreRuns = true;
				  initPhaseTime = totalExpCycleTime;	//init phase same as other cycles, note that Simualtor::Schedule schedules at
				  //the  given delay time after the CURRENT simulation time which is "remembered" always before Distroy!!!!

				  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! for noOfRuns: " << noOfRuns - 10
						  << " (currAccuracy = " << currAccuracy
						  << ", accuracy = " << accuracy << "). Running another 10..." << std::endl);
				}
				else
				{
				  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! (m_noOfRuns > MAX) - stop here:" << std::endl
						  << " (noOfRuns = " << noOfRuns - 10 <<
						  ", currAccuracy = " << currAccuracy << ", accuracy = " << accuracy << ")" << std::endl);

				  needMoreRuns = false; 	//just for safety
				  break;
				}
			}
			else
			{
				//register conf interval and accuracy level
				m_results.SetConfidenceInt(confidenceInt);
				m_results.SetAccuracy(currAccuracy);

				needMoreRuns = false;
			}
		}
	}
	while(needMoreRuns);
*/
	//*************************results collection:
/*	if((m_resultType == SingleRunFullTxRxTables || m_resultType == DupDetection) && m_noOfRuns == 1)
		internetv6->PrintResultsTxRxStats(m_inputCommand);


	Simulator::Destroy ();
*/
	//*********************cleanup before next run:
/*	delete wifiInterfaces;
	delete internetv6;
	delete wifiDevices;
	delete wifiNodes;

	//m_randomNodesInit = false;
	m_selectedNodes.clear();

	NS_LOG_INFO("No of runs for this single run with random nodes was: " << noOfRuns <<std::endl);
}*/

int NdppExperiment::DoRunRandomNodesSingleRun(bool resetStreams,  bool controlNoOfRuns)
{
	NodeContainer* wifiNodes = 0;
	NetDeviceContainer* wifiDevices = 0;
	InternetStackHelper_Nd* internetv6 = 0;
	Ipv6InterfaceContainer* wifiInterfaces = 0;

	if(resetStreams)
	{
		m_streamsUsed = 0;
		m_extraStreamNo = MID_STREAM_NO;
	}

	SingleRunInit(wifiNodes, wifiDevices, internetv6, wifiInterfaces);

	SelectRandomNodes(wifiDevices);
	SetInterfacesDownNewNodes(wifiDevices);

	m_results.SetRandomNodes(m_selectedNodes, m_selectedNodesDupl);


	//****************************simulator setup:
	//for large networks and big HL the messages from the bootstrapping phase are in a network for a long time -
	//the new address has to be assigned late, for safety reasons!!!!
	int totalExpCycleTime = 60;
	int initPhaseTime = 110;
	int timeAdv = 0;

	int kCounter = m_noOfRuns;
	bool needMoreRuns = false;

	//************* NetAnim setup:*************//
	if(m_useNetAnim)
	{
		NetAnimSetup(wifiNodes);
		if(m_withMobility)
		{
			m_anim->SetStartTime(Seconds(0));
			m_anim->SetStopTime(Seconds(initPhaseTime + totalExpCycleTime));
		}
		else
		{
			m_anim->SetStartTime(Seconds(initPhaseTime - totalExpCycleTime + 5));
			m_anim->SetStopTime(Seconds(initPhaseTime - totalExpCycleTime + 10));
		}
	}


	//****************************confidence intervals:*****************///
	double accuracy, currAccuracy, confidenceInt, t = 0.0;
	int noOfRuns = m_noOfRuns;
	if(controlNoOfRuns)
	{
		t = SelectTparam(noOfRuns);
		currAccuracy = confidenceInt = 0.0;
		if(m_resultType == mNSMessageLifetimeRandomNodes)
			accuracy = m_accuracyML;
		else
			accuracy = m_accuracyHL;
	}

	//****************************simulator run setup:


	do
	{
		//UWAGA!!!!!! musi byc >10s przerwy pomiedzy interface down i up -> f. WifiMacQueue::Cleanup czysci stare
		//pakiety, ktore pomimo zakolejkownia nei zostana wyslane. Czyszczenie po czasie m_maxDelay
		//ustawianymjako atrybut WifiMacQueue (defaultowow 10s)!!!!!!!!!!!!!!!!!!!!!!!!
		for(int k = 0; k < kCounter; k++)
		{
			if(k == 0 && !needMoreRuns)
			{
				//first round is longer, because of the bootstrapping phase
				timeAdv = initPhaseTime;
				//Simulator::Schedule(Seconds(0.0), &NdppExperiment::SetInterfacesUpNewNodes, this, wifiDevices, false);
			}
			else
			{
				timeAdv = initPhaseTime + totalExpCycleTime * k;
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 1.0)), &NdppExperiment::SetInterfacesUpNewNodes, this,
						wifiDevices, false);
			}

			Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 9.5)), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
			if(m_newNodes)
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::SetInterfacesUpNewNodes, this,
						wifiDevices, true);
			else
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::AddAddressRandomNodes, this,
									wifiDevices);

			Simulator::Schedule(Seconds(timeAdv - 14.0), &InternetStackHelper_Nd::DisableIcmpv6AsciiTracing, internetv6);
			if(!m_newNodes)
				Simulator::Schedule(Seconds(timeAdv - 13.0), &NdppExperiment::RemoveAddressRandomNodes, this, wifiDevices);
			Simulator::Schedule(Seconds(timeAdv - 12.0), &NdppExperiment::SetInterfacesDown, this, wifiDevices);

			//*************************results collection:
			switch(m_resultType)
			{
			case DupProbRandomNodes:
				//TODO
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateDupDetectCountRandomNodes,
							&m_results, internetv6, m_noOfDuplications);
				break;
			case mNSMessageLifetimeRandomNodes:
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateMLifeStatsSingleRun,
							&m_results, internetv6, true, false);
				break;
			case mNSMessageCountRandomNodes:
			default:
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateHopLimitStatsSingleRun,
							&m_results, internetv6, true, false);
				break;
			}

			Simulator::Schedule(Seconds(timeAdv - 0.5), &InternetStackHelper_Nd::ResetDadStats, internetv6);

				 /* Simulator::Schedule(Seconds(30.0), &ns3::NS_LOG_UNCOND,
				  std::string("Run no " << k << " for node " << currentNode << std::endl);*/
		}

		Simulator::Stop (Seconds (timeAdv));


		//*********************main run:
		Simulator::Run ();

		 //////////////////////****************************************////////////////////////////////////////
		//we go here after the time set by simulator::Stop
		//check if we are in a reasonable "point of operation"
		//zalozenie: dokladnosc m_accuracy na poziomie ufnosci 95%,jesli mniejsza - > ilosc runow o 10
		if(controlNoOfRuns)
		{
			kCounter = 10;
			if(m_resultType == mNSMessageLifetimeRandomNodes)
			{
			  confidenceInt = t * m_results.m_average_mNSlife_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			  currAccuracy = (confidenceInt > 0)? m_results.m_average_mNSlife_singleRun.Avg() / confidenceInt
						  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			}
			else
			{
			  confidenceInt = t * m_results.m_average_mNSolCount_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			  currAccuracy = (confidenceInt > 0)? m_results.m_average_mNSolCount_singleRun.Avg() / confidenceInt
						  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			}

			if(currAccuracy < accuracy)
			{
				noOfRuns += 10;
				if(noOfRuns <= m_maxNoOfRuns)
				{
				  //make another 10 runs and check again
				  t = SelectTparam(noOfRuns);
				  needMoreRuns = true;
				  initPhaseTime = totalExpCycleTime;	//init phase same as other cycles, note that Simualtor::Schedule schedules at
				  //the  given delay time after the CURRENT simulation time which is "remembered" always before Distroy!!!!

				  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! for noOfRuns: " << noOfRuns - 10
						  << " (currAccuracy = " << currAccuracy
						  << ", accuracy = " << accuracy << "). Running another 10..." << std::endl);
				}
				else
				{
				  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! (m_noOfRuns > MAX) - stop here:" << std::endl
						  << " (noOfRuns = " << noOfRuns - 10 <<
						  ", currAccuracy = " << currAccuracy << ", accuracy = " << accuracy << ")" << std::endl);

				  needMoreRuns = false; 	//just for safety
				  break;
				}
			}
			else
			{
				//register conf interval and accuracy level
				if(m_resultType == mNSMessageLifetimeRandomNodes)
				{
				  m_results.SetConfidenceIntML(confidenceInt);
				  m_results.SetAccuracyML(currAccuracy);
				}
				else
				{
				  m_results.SetConfidenceIntHL(confidenceInt);
				  m_results.SetAccuracyHL(currAccuracy);
				}

				needMoreRuns = false;
			}
		}
	}
	while(needMoreRuns);

	//*************************results collection:
	/*if((m_resultType == SingleRunFullTxRxTables || m_resultType == DupDetection) && m_noOfRuns == 1)
		internetv6->PrintResultsTxRxStats(m_inputCommand);*/

	Simulator::Destroy ();
	//*********************cleanup before next run:
	delete wifiInterfaces;
	delete internetv6;
	delete wifiDevices;
	delete wifiNodes;

//	m_randomNodesInit = false;
	m_selectedNodes.clear();
	m_selectedNodesDupl.clear();
	m_selectedNodesSameDupl.clear();

	if(m_useNetAnim)
	{
		delete m_anim;
		m_animFileNo++;
	}

	NS_LOG_INFO("No of runs for this single run with random nodes was: " << noOfRuns <<std::endl);
	return noOfRuns;
}

int NdppExperiment::DoRunRNSingleRunMultiExp(bool resetStreams)
{
	NodeContainer* wifiNodes = 0;
	NetDeviceContainer* wifiDevices = 0;
	InternetStackHelper_Nd* internetv6 = 0;
	Ipv6InterfaceContainer* wifiInterfaces = 0;

	if(resetStreams)
	{
		m_streamsUsed = 0;
		m_extraStreamNo = MID_STREAM_NO;
	}

	SingleRunInit(wifiNodes, wifiDevices, internetv6, wifiInterfaces);

	SelectRandomNodes(wifiDevices);
	SetInterfacesDownNewNodes(wifiDevices);

	m_results.SetRandomNodes(m_selectedNodes, m_selectedNodesDupl);


	//****************************simulator setup:
	//for large networks and big HL the messages from the bootstrapping phase are in a network for a long time -
	//the new address has to be assigned late, for safety reasons!!!!
	int totalExpCycleTime = 60;
	int initPhaseTime = 110;
	int timeAdv = 0;

	if(m_withMobility)
		m_maxNoOfRuns = m_multiExpDupNoOfRuns;
	int kCounter = m_noOfRuns;
	bool needMoreRuns, needMoreRunsHLmNS, needMoreRunsHLTot, needMoreRunsMLTot, needMoreRunsMLmNS, needMoreRunsDup, needMoreRunsLat;
	needMoreRuns = false;
	needMoreRunsHLmNS = needMoreRunsMLmNS = needMoreRunsHLTot = needMoreRunsMLTot = needMoreRunsDup = needMoreRunsLat = true;
	if(m_resultType == MultiExpNoDupRN)
		needMoreRunsDup = false;

	//************* NetAnim setup:*************//
	if(m_useNetAnim)
	{
		NetAnimSetup(wifiNodes);
		if(m_withMobility)
		{
			m_anim->SetStartTime(Seconds(0));
			m_anim->SetStopTime(Seconds(initPhaseTime + totalExpCycleTime));
		}
		else
		{
			m_anim->SetStartTime(Seconds(initPhaseTime - totalExpCycleTime + 5));
			m_anim->SetStopTime(Seconds(initPhaseTime - totalExpCycleTime + 10));
		}
	}


	//****************************confidence intervals:*****************///
	double currAccuracyHLmNS, confidenceIntHLmNS, currAccuracyHLTot, confidenceIntHLTot, t = 0.0;
	double currAccuracyMLmNS, confidenceIntMLmNS, currAccuracyMLTot, confidenceIntMLTot;
	int noOfRuns, noOfRunsHLmNS, noOfRunsMLmNS, noOfRunsHLTot, noOfRunsMLTot, noOfRunsDup, noOfRunsLat;
	noOfRuns = noOfRunsHLmNS = noOfRunsMLmNS = noOfRunsHLTot = noOfRunsMLTot = noOfRunsDup = noOfRunsLat = m_noOfRuns;

	currAccuracyHLmNS = confidenceIntHLmNS = currAccuracyHLTot = confidenceIntHLTot = 0.0;
	currAccuracyMLmNS = confidenceIntMLmNS = currAccuracyMLTot = confidenceIntMLTot =  0.0;

	//bool monitorHL, monitorML, monitorDup;
	//monitorHL = monitorML = monitorDup = true;

	//****************************simulator run setup:


	do
	{
		//UWAGA!!!!!! musi byc >10s przerwy pomiedzy interface down i up -> f. WifiMacQueue::Cleanup czysci stare
		//pakiety, ktore pomimo zakolejkownia nei zostana wyslane. Czyszczenie po czasie m_maxDelay
		//ustawianymjako atrybut WifiMacQueue (defaultowow 10s)!!!!!!!!!!!!!!!!!!!!!!!!
		for(int k = 0; k < kCounter; k++)
		{
			if(k == 0 && !needMoreRuns)
			{
				//first round is longer, because of the bootstrapping phase
				timeAdv = initPhaseTime;
				//Simulator::Schedule(Seconds(0.0), &NdppExperiment::SetInterfacesUpNewNodes, this, wifiDevices, false);
			}
			else
			{
				timeAdv = initPhaseTime + totalExpCycleTime * k;
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 1.0)), &NdppExperiment::SetInterfacesUpNewNodes, this,
						wifiDevices, false);
			}

			Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 9.5)), &InternetStackHelper_Nd::EnableIcmpv6AsciiTracing, internetv6);
			if(m_newNodes)
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::SetInterfacesUpNewNodes, this,
						wifiDevices, true);
			else
				Simulator::Schedule(Seconds(timeAdv - (totalExpCycleTime - 10.0)), &NdppExperiment::AddAddressRandomNodes, this,
									wifiDevices);

			Simulator::Schedule(Seconds(timeAdv - 14.0), &InternetStackHelper_Nd::DisableIcmpv6AsciiTracing, internetv6);
			if(!m_newNodes)
				Simulator::Schedule(Seconds(timeAdv - 13.0), &NdppExperiment::RemoveAddressRandomNodes, this, wifiDevices);
			Simulator::Schedule(Seconds(timeAdv - 12.0), &NdppExperiment::SetInterfacesDown, this, wifiDevices);

			//*************************results collection:
			if(m_resultType == MultiExpRN && needMoreRunsDup)
			{

				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateDupDetectCountRandomNodes,
							&m_results, internetv6, m_noOfDuplications);

			}
			if(needMoreRunsMLmNS || needMoreRunsMLTot)
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateMLifeStatsSingleRun,
						&m_results, internetv6, needMoreRunsMLmNS, needMoreRunsMLTot);
			if(needMoreRunsHLmNS || needMoreRunsHLTot)
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateHopLimitStatsSingleRun,
						&m_results, internetv6, needMoreRunsHLmNS, needMoreRunsHLTot);
			if(needMoreRunsLat)
				Simulator::Schedule(Seconds(timeAdv - 1.0), &ResultsCollection::UpdateLatencyStatsRNSingleRun,
						&m_results, internetv6);


			Simulator::Schedule(Seconds(timeAdv - 0.5), &InternetStackHelper_Nd::ResetDadStats, internetv6);

				 /* Simulator::Schedule(Seconds(30.0), &ns3::NS_LOG_UNCOND,
				  std::string("Run no " << k << " for node " << currentNode << std::endl);*/
		}

		Simulator::Stop (Seconds (timeAdv));


		//*********************main run:
		Simulator::Run ();

		 //////////////////////****************************************////////////////////////////////////////
		//we go here after the time set by simulator::Stop
		//check if we are in a reasonable "point of operation"
		//zalozenie: dokladnosc m_accuracy na poziomie ufnosci 95%,jesli mniejsza - > ilosc runow o 10
		//for duplication experiments - go teh specified amount of times (m_multiExpDupNoOfRuns)

		kCounter = 10;
		if(needMoreRunsHLmNS || needMoreRunsMLmNS || needMoreRunsHLTot || needMoreRunsMLTot)
			t = SelectTparam(noOfRuns);
		//ML - mNS
		if(needMoreRunsMLmNS)
		{
			confidenceIntMLmNS = t * m_results.m_average_mNSlife_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			currAccuracyMLmNS = (confidenceIntMLmNS > 0)? m_results.m_average_mNSlife_singleRun.Avg() / confidenceIntMLmNS
					  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			if(currAccuracyMLmNS >= m_accuracyML)
			{
				needMoreRunsMLmNS = false;

				m_results.m_noOfRunsMLmNS = noOfRuns;
				m_results.SetConfidenceIntML(confidenceIntMLmNS);
				m_results.SetAccuracyML(currAccuracyMLmNS);
			}
		}

		//ML - mNS
		if(needMoreRunsMLTot)
		{
			confidenceIntMLTot = t * m_results.m_average_mNDMessLife_singleRun.Stddev() / std::sqrt((double)noOfRuns);
			currAccuracyMLTot = (confidenceIntMLTot > 0)? m_results.m_average_mNDMessLife_singleRun.Avg() / confidenceIntMLTot
					  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
			if(currAccuracyMLTot >= m_accuracyML)
			{
				needMoreRunsMLTot = false;

				m_results.m_noOfRunsMLTot = noOfRuns;
				m_results.SetConfidenceIntMLTot(confidenceIntMLTot);
				m_results.SetAccuracyMLTot(currAccuracyMLTot);
			}
		}

		//HL - mNS
		if(needMoreRunsHLmNS)
		{
		  confidenceIntHLmNS = t * m_results.m_average_mNSolCount_singleRun.Stddev() / std::sqrt((double)noOfRuns);
		  currAccuracyHLmNS = (confidenceIntHLmNS > 0)? m_results.m_average_mNSolCount_singleRun.Avg() / confidenceIntHLmNS
					  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
		  if(currAccuracyHLmNS >= m_accuracyHL)
		  {
			  needMoreRunsHLmNS = false;

			  m_results.m_noOfRunsHLmNS = noOfRuns;
			  m_results.SetConfidenceIntHL(confidenceIntHLmNS);
			  m_results.SetAccuracyHL(currAccuracyHLmNS);
		  }
		}

		//HL - Tot
		if(needMoreRunsHLTot)
		{
		  confidenceIntHLTot = t * m_results.m_average_mNDMessCount_singleRun.Stddev() / std::sqrt((double)noOfRuns);
		  currAccuracyHLTot = (confidenceIntHLTot > 0)? m_results.m_average_mNDMessCount_singleRun.Avg() / confidenceIntHLTot
					  : 1000.0;	//1000 is supposed to be greater than any m_accuracy that can be assumed
		  if(currAccuracyHLTot >= m_accuracyHL)
		  {
			  needMoreRunsHLTot = false;

			  m_results.m_noOfRunsHLTot = noOfRuns;
			  m_results.SetConfidenceIntHLTot(confidenceIntHLTot);
			  m_results.SetAccuracyHLTot(currAccuracyHLTot);
		  }
		}

		//Dup
		if(m_resultType == MultiExpRN && needMoreRunsDup)
		{
			if(noOfRuns >= m_multiExpDupNoOfRuns)
			{
				//stop duplication experiments
				needMoreRunsDup = false;

				m_results.m_noOfRunsDup = noOfRuns;
				NS_ASSERT_MSG(noOfRuns == m_multiExpDupNoOfRuns, "Ndpp_Experiment: Sth. is wrong - no of runs is greater than max value for duplication exp.");
				noOfRunsDup = noOfRuns;
			}
		}

		//Latency
		if(needMoreRunsLat)
		{
			if(noOfRuns >= m_multiExpDupNoOfRuns)
			{
				//stop duplication experiments
				needMoreRunsLat = false;

				NS_ASSERT_MSG(noOfRuns == m_multiExpDupNoOfRuns, "Ndpp_Experiment: Sth. is wrong - no of runs is greater than max value for latency exp.");
				noOfRunsLat = noOfRuns;
			}
		}

		//specify next steps
		if(needMoreRunsHLmNS || needMoreRunsMLmNS || needMoreRunsHLTot || needMoreRunsMLTot || needMoreRunsDup || needMoreRunsLat)
		{
			noOfRuns += 10;
			if(noOfRuns <= m_maxNoOfRuns)
			{
			  //make another 10 runs and check again
			 // t = SelectTparam(noOfRuns);
			  needMoreRuns = true;
			  initPhaseTime = totalExpCycleTime;	//init phase same as other cycles, note that Simualtor::Schedule schedules at
			  //the  given delay time after the CURRENT simulation time which is "remembered" always before Distroy!!!!

			  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! for noOfRuns: " << noOfRuns - 10 << "\n"
					  << "needMoreRunsHLmNS = " << needMoreRunsHLmNS << ", (currAccuracyHLmNS = " << currAccuracyHLmNS
					  << ", accuracyHLmNS = " << m_accuracyHL << ")."  << "\n"
					  << "needMoreRunsMLmNS = " << needMoreRunsMLmNS << ", (currAccuracyMLmNS = " << currAccuracyMLmNS
					  << ", accuracyMLmNS = " << m_accuracyML << ")."  << "\n"
					  << "needMoreRunsHLTot = " << needMoreRunsHLTot << ", (currAccuracyHLTot = " << currAccuracyHLTot
					  << ", accuracyHLTot = " << m_accuracyHL << ")."  << "\n"
					  << "needMoreRunsMLTot = " << needMoreRunsMLTot << ", (currAccuracyMLTot = " << currAccuracyMLTot
					  << ", accuracyMLTot = " << m_accuracyML << ")."  << "\n"
					  << "needMoreRunsDup = " << needMoreRunsDup << "\n"
					  << "needMoreRunsLat = " << needMoreRunsLat << "\n"
					  << "Running another 10..." << std::endl);
			}
			else
			{
			  NS_LOG_UNCOND("Accuracy threshold exceeded!!!! (m_noOfRuns > MAX) - stop here:" << std::endl
					  << " (noOfRuns = " << noOfRuns - 10 << "\n"
					  << "needMoreRunsHL = " << needMoreRunsHLmNS << ", (currAccuracyHL = " << currAccuracyHLmNS
					  << ", accuracyHL = " << m_accuracyHL << ")."  << "\n"
					  << "needMoreRunsML = " << needMoreRunsMLmNS << ", (currAccuracyMLmNS = " << currAccuracyMLmNS
					  << ", accuracyML = " << m_accuracyML << ")."  << "\n"
					  << "needMoreRunsHLTot = " << needMoreRunsHLTot << ", (currAccuracyHLTot = " << currAccuracyHLTot
					  << ", accuracyHLTot = " << m_accuracyHL << ")."  << "\n"
					  << "needMoreRunsMLTot = " << needMoreRunsMLTot << ", (currAccuracyMLTot = " << currAccuracyMLTot
					  << ", accuracyMLTot = " << m_accuracyML << ")."  << "\n"
					  << "needMoreRunsDup = " << needMoreRunsDup << "\n"
					  << "needMoreRunsLat = " << needMoreRunsLat <<"\n");

			  needMoreRuns = false; 	//just for safety
			  //register conf interval and accuracy level
			  m_results.SetConfidenceIntML(confidenceIntMLmNS);
			  m_results.SetAccuracyML(currAccuracyMLmNS);
			  m_results.SetConfidenceIntHL(confidenceIntHLmNS);
			  m_results.SetAccuracyHL(currAccuracyHLmNS);
			  m_results.SetConfidenceIntMLTot(confidenceIntMLTot);
			  m_results.SetAccuracyMLTot(currAccuracyMLTot);
			  m_results.SetConfidenceIntHLTot(confidenceIntHLTot);
			  m_results.SetAccuracyHLTot(currAccuracyHLTot);
			  break;
			}
		}
		else
		{
			needMoreRuns = false;
			break;
		}
	}
	while(needMoreRuns);

	//*************************results collection:
	/*if((m_resultType == SingleRunFullTxRxTables || m_resultType == DupDetection) && m_noOfRuns == 1)
		internetv6->PrintResultsTxRxStats(m_inputCommand);*/

	Simulator::Destroy ();
	//*********************cleanup before next run:
	delete wifiInterfaces;
	delete internetv6;
	delete wifiDevices;
	delete wifiNodes;

//	m_randomNodesInit = false;
	m_selectedNodes.clear();
	m_selectedNodesDupl.clear();
	m_selectedNodesSameDupl.clear();

	if(m_useNetAnim)
	{
		delete m_anim;
		m_animFileNo++;
	}

	NS_LOG_INFO("No of runs for this single run with random nodes was: " << noOfRuns <<std::endl);
	return noOfRuns;
}


void NdppExperiment::DoRunDupDetectionExp()
{
	m_noOfRuns = 1;	//just for safety

	//set HopLimit for this run:
	Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (m_initialHopLimit));

	DoRunSelectedNodeSingleRun(m_initiatorNo, m_targetNo, false);

	m_results.ResetAverageStatsSingleRun();
}

void NdppExperiment::DoRunDupProbability()
{
	for(int hopLimit = m_initialHopLimit; hopLimit <= m_maxHopLimit; hopLimit += m_hlStep)
	{
	  //RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
									 //puli niezaleznych liczb losowych

	  NS_LOG_UNCOND("Run with HopLimit = " << hopLimit << std::endl);

	  //set HopLimit for this run set:
	  Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (hopLimit));
	  DoRunSelectedNodeSingleRunLong(m_initiatorNo, m_targetNo, true, false);


	  //***************************Print results - one line for this hop limit

	  m_results.PrintPartialResultDupProbability(m_inputCommand, hopLimit, m_noOfRuns);

	  //reset before next run:
	  m_results.ResetStats();
	}
}

void NdppExperiment::DoRunDupProbRandomNode()
{
	//RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
								 //puli niezaleznych liczb losowych

	//set HopLimit for this run set:
	Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (m_initialHopLimit));

	for(int i = 0; i < m_randomNodesNoOfRuns; i++)
	{
	  DoRunRandomNodesSingleRun(true, false);	//arbitrary selected target node

	  //***************************Print results - one line for this random node selection
	  m_results.UpdateDupProbStats(m_noOfRuns);
	  m_results.PrintPartialResultDupProbability(m_inputCommand, m_initialHopLimit, m_noOfRuns);

	  //reset before next run:
	  m_results.ResetAverageStatsSingleRun();
	}

	 ////////////////////****************************************//////////////////////////////
	  //***************************Print results - summary from all variants of node selection
	  ////////////////////****************************************//////////////////////////////

	  m_results.PrintResultDupProbability("", m_initialHopLimit);

	  //reset before next run:
	  m_results.ResetStats();
}

void NdppExperiment::DoRunMultiExpRandomNodes()
{
	//RngSeedManager::SetRun(1);	//do ew. rozwazenia, jesli symulacje beda bardzo dlugie i mogloby to grozic wyczerpaniem
								 //puli niezaleznych liczb losowych

	//int noOfRuns;
	//set HopLimit for this run set:
	Config::SetDefault ("ns3::Icmpv6L4Protocol_NdSimple::HopLimit", UintegerValue (m_initialHopLimit));

	for(int i = 0; i < m_randomNodesNoOfRuns; i++)
	{
	   DoRunRNSingleRunMultiExp(true);

	  //***************************Print results - one line for this random node selection
	  if(m_resultType == MultiExpRN)
		   m_results.UpdateDupProbStats(-1);
	  m_results.UpdateMessageLifetimeStats();
	  m_results.UpdateHopLimitStats();
	  m_results.UpdateLatencyStatsRN();

	  m_results.PrintIntroMultiExp(m_inputCommand);
	  if(m_resultType == MultiExpRN)
		  m_results.PrintPartialResultDupProbability("", m_initialHopLimit, -1, "\t\t");
	  m_results.PrintPartialResultHLmNSSingleRun("",m_initialHopLimit, -1, "\t\t");
	  m_results.PrintPartialResultHLTotalSingleRun("",m_initialHopLimit, -1, "\t\t");
	  m_results.PrintPartialResultMlifeSingleRun("",m_initialHopLimit, -1, "\t\t", true, "\t\t");
	  m_results.PrintPartialResultLatencyRN("", "\n");

	  //reset before next run:
	  m_results.ResetAverageStatsSingleRun();
	}

	 ////////////////////****************************************//////////////////////////////
	  //***************************Print results - summary from all variants of node selection
	  ////////////////////****************************************//////////////////////////////

	 if(m_resultType == MultiExpRN)
		 m_results.PrintResultDupProbability("", m_initialHopLimit);

	  m_results.ResetResultPrintInit();
	  m_results.PrintResultHopLimit("", m_initialHopLimit, -1, true);
	  m_results.ResetResultPrintInit();
	  m_results.PrintResultMessageLifetime("", m_initialHopLimit, -1, true);
	  m_results.ResetResultPrintInit();
	  m_results.PrintResultLatencyRN("");

	  //reset before next run:
	  m_results.ResetStats();
}



///////////////////////////////////////////////////////////
//Class ResultsCollection

ResultsCollection::ResultsCollection():
		m_noOfRunsDup(0),
		m_noOfRunsMLmNS(0),
		m_noOfRunsMLTot(0),
		m_noOfRunsHLmNS(0),
		m_noOfRunsHLTot(0),
		m_resultPrintInit(false),
		m_dupDetectCount(0),
		m_dupDetectCountAll (0),
		m_dupDetectCount90 (0),
		m_dupDetectCount75 (0),
		m_dupDetectCount50 (0),
		m_currAccuracyHLmNS (-1.0),
		m_currAccuracyHLTot (-1.0),
		m_currAccuracyMLmNS (-1.0),
		m_currAccuracyMLTot (-1.0),
		m_confidenceIntHLmNS (-1.0),
		m_confidenceIntHLTot(-1.0),
		m_confidenceIntMLmNS (-1.0),
		m_confidenceIntMLTot (-1.0)
{

}

ResultsCollection::~ResultsCollection()
{
	m_selectedRandomNodes.clear();
	m_selectedDuplNodes.clear();
	m_averageLatencyPerRNSelection.clear();
	m_averageLatencyPerNode.clear();
}

void ResultsCollection::EnableResultsCollection(std::string prefix, std::string sufix)
{
	AsciiTraceHelper asciiTraceHelper;

	std::string filename;
	filename = (sufix != "")? prefix+"-"+sufix : prefix;

	if(!m_outputStream)
		m_outputStream = asciiTraceHelper.CreateFileStream (filename);
}

void ResultsCollection::SetConfidenceIntHL(double confidenceInt)
{
	m_confidenceIntHLmNS = confidenceInt;
}

void ResultsCollection::SetAccuracyHL(double currAccuracy)
{
	m_currAccuracyHLmNS = currAccuracy;
}

void ResultsCollection::SetConfidenceIntHLTot(double confidenceInt)
{
	m_confidenceIntHLTot = confidenceInt;
}

void ResultsCollection::SetAccuracyHLTot(double currAccuracy)
{
	m_currAccuracyHLTot = currAccuracy;
}


void ResultsCollection::SetConfidenceIntML(double confidenceInt)
{
	m_confidenceIntMLmNS = confidenceInt;
}

void ResultsCollection::SetAccuracyML(double currAccuracy)
{
	m_currAccuracyMLmNS = currAccuracy;
}

void ResultsCollection::SetConfidenceIntMLTot(double confidenceInt)
{
	m_confidenceIntMLTot = confidenceInt;
}

void ResultsCollection::SetAccuracyMLTot(double currAccuracy)
{
	m_currAccuracyMLTot = currAccuracy;
}

void ResultsCollection::ResetResultPrintInit()
{
	m_resultPrintInit = false;
}

void ResultsCollection::SetRandomNodes (std::list<std::pair<int, Ipv6Address>> & selectedNodes,
		std::list<std::pair<int, DuplInfo>> & selectedDuplNodes)
{
	m_selectedRandomNodes.clear();
	m_selectedDuplNodes.clear();
	//std::map<int, Ipv6Address>::const_iterator it;
	std::list<std::pair<int, Ipv6Address>>::const_iterator it;
	std::list<std::pair<int, DuplInfo>>::const_iterator it2;

	for(it = selectedNodes.begin(); it!= selectedNodes.end(); it++)
		m_selectedRandomNodes.push_back(it->first);

	m_selectedRandomNodes.sort();

	for(it2 = selectedDuplNodes.begin(); it2!= selectedDuplNodes.end(); it2++)
		m_selectedDuplNodes.insert(std::make_pair(it2->first, DupProbInfo(it2->second.m_addr, it2->second.m_target, 0)));
}

void ResultsCollection::PrintResult(int resultType, std::string additionalInfo, uint32_t hopLimit,
		 			 int noOfRuns)
{
	switch(resultType)
	{
	case NdppExperiment::mNSMessageLifetime:
		PrintPartialResultMlifeSingleRun(additionalInfo, hopLimit, noOfRuns);
		break;
	case NdppExperiment::HopLimitTest:
		PrintResultHopLimit(additionalInfo, hopLimit, noOfRuns);
		break;
	case NdppExperiment::HopLimitSelectedNode:
		PrintPartialResultHLmNSSingleRun(additionalInfo, hopLimit, noOfRuns);
		break;
	}

}

void ResultsCollection::PrintIntroMultiExp(std::string additionalInfo)
{
	if(!m_resultPrintInit)
	{
		m_resultPrintInit = true;

		*m_outputStream->GetStream () << "Results for multi experiment with random nodes" << std::endl
				<< additionalInfo << std::endl << std::endl;
		if(!m_selectedDuplNodes.empty())
			*m_outputStream->GetStream () << "Duplication probability \t\t\t\t\t\t\t\t\t\t\t | \t";
		*m_outputStream->GetStream () << "mNS Message Count \t\t\t\t\t\t\t\t\t\t | \tmND Total Message Count" << " \t\t\t\t\t\t\t\t\t\t | \tmNS Message Lifetime"
					<< " \t\t\t\t\t\t\t\t\t\t | \tmND Total Message Lifetime" << " \t\t\t\t\t\t\t\t\t\t | \tLatency" << "\n" << std::endl;

		*m_outputStream->GetStream()<< std::endl;

		//Duplication probability
		//*m_outputStream->GetStream () << "Duplication probability:" << std::endl << std::endl << std::endl;

		//header:
		if(!m_selectedDuplNodes.empty())
			*m_outputStream->GetStream() << "HopLimit" << "\t" << "Prob. All"  << "\t" << "Prob. 90" << "\t" << "Prob. 75%"
			 << "\t" << "Prob. 50%" << "\t" << "Selected dupl. nodes" << "\t" << "Dupl. nodes with targets and dup. prob. per node"
			 << "\t" << "Avg. Dup. prob. per node" << "\t" << "Min" << "\t" << "Max." << "\t" << "Std. dev."
			 << "\t" << "|"  << "\t";

		//Message Count
	//	*m_outputStream->GetStream () << "Message Count:" << std::endl << std::endl << std::endl;

		//*m_outputStream->GetStream () << "(Single node run - hopLimit vs no of multihop NS messages)" <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average no of NS messages" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
				<< "\t" << "confidenceInt";
		if(!m_selectedRandomNodes.empty())
			*m_outputStream->GetStream() << "\t" << "selected Nodes";

		*m_outputStream->GetStream()<< "\t" << "|"  << "\t";

		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average no of ND++ messages (mNA+mNS+NS+NA)" << "\t" << "Min" << "\t"
						<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
						<< "\t" << "confidenceInt";
		*m_outputStream->GetStream()<< "\t" << "|"  << "\t";

		//Message Lifetime
		//*m_outputStream->GetStream () << "Message Lifetime:" << std::endl << std::endl << std::endl;

		//*m_outputStream->GetStream () << "(Single node run - hopLimit vs average message lifetime)" <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average message lifetime (millisec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
				<< "\t" << "confidenceInt";

		if(!m_selectedRandomNodes.empty())
			*m_outputStream->GetStream() << "\t" << "selected Nodes";

		*m_outputStream->GetStream()<< "\t" << "|"  << "\t";

		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average mND (mNS+mNA) message lifetime (millisec.)" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
			<< "\t" << "confidenceInt";
		*m_outputStream->GetStream()<< "\t" << "|"  << "\t";

		//Latency:
		*m_outputStream->GetStream() << "Avg.Latency (microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Avg.Latency-INV (microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Avg.Latency-PREF(microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t";

		if(m_selectedDuplNodes.size() != m_selectedRandomNodes.size())	//all nodes duplicated
		{
			*m_outputStream->GetStream() << "Avg.Latency Not Dupl. Nodes (microsec.)" << "\t" << "Min" << "\t"
					<< "Max" << "\t" << "StdDev" << "\t";
		}
		if(!m_selectedDuplNodes.empty())
		{
			*m_outputStream->GetStream() << "Avg.Latency Dupl. Nodes (microsec.)" << "\t" << "Min" << "\t"
					<< "Max" << "\t" << "StdDev" << "\t" ;
		}

		*m_outputStream->GetStream()<< "details per node: node no. (av.lat.tot., av.lat.inv, av.lat.pref)" << std::endl;


		*m_outputStream->GetStream()<< std::endl;
	}
}

void ResultsCollection::PrintResultHopLimit(std::string additionalInfo, uint32_t hopLimit, int noOfRuns, bool printTotal)
{
	if(!m_resultPrintInit)
	{
		*m_outputStream->GetStream () << std::endl << std::endl << "Results for Hop Limit experiment:" << std::endl;
		if(additionalInfo.compare(""))
			*m_outputStream->GetStream () << additionalInfo << std::endl;

		*m_outputStream->GetStream () << "(HopLimit vs no of multihop NS messages)" <<std::endl;

		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" << "\t"<< "noOfRuns" << std::endl;
	}

	*m_outputStream->GetStream() << hopLimit << "\t" << m_averageResult_mNSCount.m_averageResult.Avg() << "\t"
			<< m_averageResult_mNSCount.Min() << "\t"
			<< m_averageResult_mNSCount.Max() << "\t"
			<< m_averageResult_mNSCount.MinMin() << "\t"
			<< m_averageResult_mNSCount.MaxMax()  << "\t" << noOfRuns << std::endl;

	if(printTotal)
	{
		if(!m_resultPrintInit)
		{
			*m_outputStream->GetStream () << std::endl << std::endl << "Results for Hop Limit experiment:" << std::endl;
			if(additionalInfo.compare(""))
				*m_outputStream->GetStream () << additionalInfo << std::endl;

			*m_outputStream->GetStream () << "(HopLimit vs no of total ND++ messages (NS+NA+mNS+mNA))" <<std::endl;

			//header:
			*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average" << "\t" << "Min" << "\t"
					<< "Max" << "MinMin" << "\t" << "MaxMax" << "\t" << "noOfRuns" << std::endl;
		}

		*m_outputStream->GetStream() << hopLimit << "\t" << m_averageResult_mNDMessCount.m_averageResult.Avg() << "\t"
				<< m_averageResult_mNDMessCount.Min() << "\t"
				<< m_averageResult_mNDMessCount.Max() << "\t"
				<< m_averageResult_mNDMessCount.MinMin() << "\t"
				<< m_averageResult_mNDMessCount.MaxMax() << "\t" << noOfRuns << std::endl;
	}

	m_resultPrintInit = true;
}

void ResultsCollection::PrintPartialResultHLmNSSingleRun(std::string additionalInfo, uint32_t hopLimit,
		int noOfRuns, std::string lineEnd)
{
	if(noOfRuns == -1)
			noOfRuns = m_noOfRunsHLmNS;

	if(!m_resultPrintInit)
	{
		m_resultPrintInit = true;

		*m_outputStream->GetStream () << "Results for Single Node experiment:" << std::endl
				<< additionalInfo << std::endl << std::endl;

		*m_outputStream->GetStream () << "(Single node run - hopLimit vs no of multihop NS messages)" <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average no of NS messages" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
				<< "\t" << "confidenceInt";
		if(!m_selectedRandomNodes.empty())
			*m_outputStream->GetStream() << "\t" << "selected Nodes";

		*m_outputStream->GetStream()<< std::endl;
	}

	*m_outputStream->GetStream() << hopLimit << "\t" << m_average_mNSolCount_singleRun.Avg() << "\t" << m_average_mNSolCount_singleRun.Min()
			<< "\t" << m_average_mNSolCount_singleRun.Max() << "\t" << m_average_mNSolCount_singleRun.Stddev() << "\t"
			<< m_average_mNSolCount_singleRun.Var()  << "\t" << noOfRuns << "\t" << m_currAccuracyHLmNS
			<< "\t" << m_confidenceIntHLmNS;
	if(!m_selectedRandomNodes.empty())
	{
		*m_outputStream->GetStream() << "\t";

		for(ListIntCI it = m_selectedRandomNodes.begin(); it != m_selectedRandomNodes.end(); it++)
			*m_outputStream->GetStream() << *it << ", ";
	}
	*m_outputStream->GetStream()<< lineEnd << std::flush;
}

void ResultsCollection::PrintPartialResultHLTotalSingleRun(std::string additionalInfo, uint32_t hopLimit,
		int noOfRuns, std::string lineEnd)
{
	if(noOfRuns == -1)
			noOfRuns = m_noOfRunsHLTot;

	if(!m_resultPrintInit)
	{
		m_resultPrintInit = true;

		*m_outputStream->GetStream () << "Results for Single Node experiment:" << std::endl
				<< additionalInfo << std::endl << std::endl;

		*m_outputStream->GetStream () << "(Single node run - hopLimit vs no of multihop NS messages)" <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average no of ND++ messages (mNA+mNS+NS+NA)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
				<< "\t" << "confidenceInt";
		/*if(!m_selectedRandomNodes.empty())
			*m_outputStream->GetStream() << "\t" << "selected Nodes";*/

		*m_outputStream->GetStream()<< std::endl;
	}

	*m_outputStream->GetStream() << hopLimit << "\t" << m_average_mNDMessCount_singleRun.Avg() << "\t" << m_average_mNDMessCount_singleRun.Min()
			<< "\t" << m_average_mNDMessCount_singleRun.Max() << "\t" << m_average_mNDMessCount_singleRun.Stddev() << "\t"
			<< m_average_mNDMessCount_singleRun.Var()  << "\t" << noOfRuns << "\t" << m_currAccuracyHLTot
			<< "\t" << m_confidenceIntHLTot;
	/*if(!m_selectedRandomNodes.empty())
	{
		*m_outputStream->GetStream() << "\t";

		for(uint32_t i=0; i<m_selectedRandomNodes.size(); i++)
			*m_outputStream->GetStream() << m_selectedRandomNodes[i] << ", ";
	}*/
	*m_outputStream->GetStream()<< lineEnd << std::flush;
}

void ResultsCollection::PrintPartialResultDupProbability(std::string additionalInfo, uint32_t hopLimit, int count, std::string lineEnd)
{
	std::map<int, DupProbInfo>::const_iterator it;
	if(count == -1)
		count = m_noOfRunsDup;

	if(!m_resultPrintInit)
	{
		m_resultPrintInit = true;

		*m_outputStream->GetStream () << "Results for Single Run experiment (duplication probability:" << std::endl
				<< additionalInfo << std::endl << std::endl;

		*m_outputStream->GetStream () << "(Single run - hopLimit vs duplication probability)" <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t";
		if(!m_selectedDuplNodes.empty())
			*m_outputStream->GetStream() << "Prob. All"  << "\t" << "Prob. 90" << "\t" << "Prob. 75%"
			 << "\t" << "Prob. 50%" << "\t" << "Selected dupl. nodes" << "\t" << "Dupl. nodes with targets and dup. prob. per node"
			 << "\t" << "Avg. Dup. prob. per node" << "\t" << "Min" << "\t" << "Max." << "\t" << "Std. dev.";
		else
			*m_outputStream->GetStream() << "Duplication probability";


		*m_outputStream->GetStream()<< std::endl;
	}

	NS_ASSERT_MSG(count, "Trying to count duplication probability with zero runs");

	*m_outputStream->GetStream() << hopLimit << "\t";
	if(!m_selectedDuplNodes.empty())
	{
		Average<double> averageDupProbPerNode;
		double dupProb;
		*m_outputStream->GetStream() << ((double)m_dupDetectCountAll) / count << "\t" << ((double)m_dupDetectCount90) / count << "\t"
				<< ((double)m_dupDetectCount75) / count << "\t" << ((double)m_dupDetectCount50) / count << "\t";
		for(it = m_selectedDuplNodes.begin(); it!= m_selectedDuplNodes.end(); it++)
			*m_outputStream->GetStream() << it->first << ", ";

		*m_outputStream->GetStream()<< "\t";

		for(it = m_selectedDuplNodes.begin(); it!= m_selectedDuplNodes.end(); it++)
		{
			dupProb = ((double)it->second.m_dupCount) / count;
			averageDupProbPerNode.Update(dupProb);
			*m_outputStream->GetStream() << it->first << "->" << it->second.m_target
					<< "(" << dupProb << ")" << ", ";
		}
		//remember values:
		m_averageDupProbPerNode.m_averageResult.Update(averageDupProbPerNode.Avg());
		m_averageDupProbPerNode.m_minResult.Update(averageDupProbPerNode.Min());
		m_averageDupProbPerNode.m_maxResult.Update(averageDupProbPerNode.Max());
		*m_outputStream->GetStream()<< "\t" << averageDupProbPerNode.Avg() << "\t" <<averageDupProbPerNode.Min()
				<< "\t" << averageDupProbPerNode.Max() << "\t" << averageDupProbPerNode.Stddev();
	}
	else
	{
		*m_outputStream->GetStream() << ((double)m_dupDetectCount) / count;
	}

	*m_outputStream->GetStream()<< lineEnd << std::flush;
}

void ResultsCollection::PrintResultDupProbability(std::string additionalInfo, uint32_t hopLimit)
{
	*m_outputStream->GetStream () << std::endl << std::endl << "Results for Dup. Prob. experiment:" << std::endl;
	if(additionalInfo.compare(""))
		*m_outputStream->GetStream () << additionalInfo << std::endl;

	*m_outputStream->GetStream () << "(hopLimit vs duplication probability - avareged over several random node selections)" <<std::endl;

	//Dup. prob. per node
	//header:
	*m_outputStream->GetStream() << "HopLimit" << "\t" << "Duplication prob. per node (Avg.)" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" << std::endl;
	//value
	*m_outputStream->GetStream() << hopLimit << "\t" << m_averageDupProbPerNode.m_averageResult.Avg() << "\t"
			<< m_averageDupProbPerNode.Min() << "\t"
			<< m_averageDupProbPerNode.Max() << "\t"
			<< m_averageDupProbPerNode.MinMin() << "\t"
			<< m_averageDupProbPerNode.MaxMax() << std::endl << std::endl;


	//All
	//header:
	*m_outputStream->GetStream() << "HopLimit" << "\t" << "Total Dupl. Prob. All" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "StdDev" << "\t" << "Var" << std::endl;
	//value
	*m_outputStream->GetStream() << hopLimit << "\t" << m_averageDupProbAll.Avg() << "\t" << m_averageDupProbAll.Min()
			<< "\t" << m_averageDupProbAll.Max() << "\t" << m_averageDupProbAll.Stddev() << "\t"
			<< m_averageDupProbAll.Var() << std::endl << std::endl;

	//90%
	//header:
	*m_outputStream->GetStream() << "HopLimit" << "\t" << "Total Dupl. Prob. 90" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "StdDev" << "\t" << "Var" << std::endl;
	//value
	*m_outputStream->GetStream() << hopLimit << "\t" << m_averageDupProb90.Avg() << "\t" << m_averageDupProb90.Min()
			<< "\t" << m_averageDupProb90.Max() << "\t" << m_averageDupProb90.Stddev() << "\t"
			<< m_averageDupProb90.Var() << std::endl << std::endl;

	//75%
	//header:
	*m_outputStream->GetStream() << "HopLimit" << "\t" << "Total Dupl. Prob. 75" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "StdDev" << "\t" << "Var" << std::endl;
	//value
	*m_outputStream->GetStream() << hopLimit << "\t" << m_averageDupProb75.Avg() << "\t" << m_averageDupProb75.Min()
			<< "\t" << m_averageDupProb75.Max() << "\t" << m_averageDupProb75.Stddev() << "\t"
			<< m_averageDupProb75.Var() << std::endl << std::endl;

	//50%
	//header:
	*m_outputStream->GetStream() << "HopLimit" << "\t" << "Total Dupl. Prob. 50" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "StdDev" << "\t" << "Var" << std::endl;
	//value
	*m_outputStream->GetStream() << hopLimit << "\t" << m_averageDupProb50.Avg() << "\t" << m_averageDupProb50.Min()
			<< "\t" << m_averageDupProb50.Max() << "\t" << m_averageDupProb50.Stddev() << "\t"
			<< m_averageDupProb50.Var() << std::endl << std::endl;

}

void ResultsCollection::PrintPartialResultLatencyRN(std::string additionalInfo, std::string lineEnd)
{
	if(!m_resultPrintInit)
	{
		m_resultPrintInit = true;
		*m_outputStream->GetStream () << "Results for Latency experiment:" << std::endl
				<< additionalInfo << std::endl << std::endl;

		//header:
		*m_outputStream->GetStream() << "Avg.Latency (microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Avg.Latency-INV (microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Avg.Latency-PREF(microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t";
		if(m_selectedDuplNodes.size() != m_selectedRandomNodes.size())	//all nodes duplicated
		{
			*m_outputStream->GetStream() << "Avg.Latency Not Dupl. Nodes (microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" ;
		}

		if(!m_selectedDuplNodes.empty())
		{
			*m_outputStream->GetStream() << "Avg.Latency Dupl. Nodes (microsec.)" << "\t" << "Min" << "\t"
					<< "Max" << "\t" << "StdDev" << "\t" ;
		}

		*m_outputStream->GetStream()<< "details per node: node no. (av.lat.tot., av.lat.inv, av.lat.pref)" << std::endl;
	}

	//results:
	//latency total:
	*m_outputStream->GetStream() << m_averageLatencyPerRNSelection.back().m_averageLatency.Avg() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatency.Min() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatency.Max() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatency.m_averageResult.Stddev() << "\t";
	//latency INV:
	if(m_averageLatencyPerRNSelection.back().m_averageLatencyINV.m_averageResult.Count() != 0)
		*m_outputStream->GetStream() << m_averageLatencyPerRNSelection.back().m_averageLatencyINV.Avg() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyINV.Min() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyINV.Max() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyINV.m_averageResult.Stddev() << "\t";
	else
		*m_outputStream->GetStream() << "-1" << "\t" << "-1" << "\t" <<  "-1" << "\t" << "-1" << "\t";

	//latency PREF
	if(m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.m_averageResult.Count() != 0)
		*m_outputStream->GetStream() << m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.Avg() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.Min() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.Max() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.m_averageResult.Stddev() << "\t";
	else
		*m_outputStream->GetStream() << "-1" << "\t" << "-1" << "\t" <<  "-1" << "\t" << "-1" << "\t";

	//latency No Dupl
	if(m_selectedDuplNodes.size() != m_selectedRandomNodes.size())	//all nodes duplicated
	{
		*m_outputStream->GetStream() << m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.Avg() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.Min() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.Max() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.m_averageResult.Stddev() << "\t";
	}
	//laetncy Dupl
	if(!m_selectedDuplNodes.empty())
	{
		*m_outputStream->GetStream() << m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.Avg() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.Min() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.Max() << "\t"
			<<  m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.m_averageResult.Stddev() << "\t";
	}
	//details per node:

	for(LatencyI it = m_averageLatencyPerNode.begin(); it!= m_averageLatencyPerNode.end(); it++)
	{
		*m_outputStream->GetStream() << it->first << " (" << it->second.m_averageLatency.Avg() << ", ";
		if(it->second.m_averageLatencyINV.m_averageResult.Count())
			*m_outputStream->GetStream() << it->second.m_averageLatencyINV.Avg() << ", ";
		else
			*m_outputStream->GetStream() << "-1" << ", ";
		if(it->second.m_averageLatencyPREF.m_averageResult.Count())
			*m_outputStream->GetStream() << it->second.m_averageLatencyPREF.Avg() << "); ";
		else
			*m_outputStream->GetStream() << "-1" << "); ";
	}

	*m_outputStream->GetStream()<< lineEnd << std::flush;
}

void ResultsCollection::PrintResultLatencyRN(std::string additionalInfo)
{

	*m_outputStream->GetStream () << std::endl << std::endl << "Results for Latency experiment:" << std::endl;
	if(additionalInfo.compare(""))
		*m_outputStream->GetStream () << additionalInfo << std::endl;

	//total header :
	*m_outputStream->GetStream() << "Average latency total (microsec.)" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" << std::endl;
	//value:
	*m_outputStream->GetStream() << m_averageLatencyTotal.m_averageLatency.Avg() << "\t"
			<< m_averageLatencyTotal.m_averageLatency.Min() << "\t"
			<< m_averageLatencyTotal.m_averageLatency.Max() << "\t"
			<< m_averageLatencyTotal.m_averageLatency.MinMin() << "\t"
			<< m_averageLatencyTotal.m_averageLatency.MaxMax() /* << "\t"
			<< m_averageLatencyTotal.m_averageLatency.Stddev() << "\t"
			<< m_averageLatencyTotal.m_averageLatency.Var() << "\t" */<< std::endl;

	//INV header :
	*m_outputStream->GetStream() << "Average latency INV (microsec.)" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" /*<< "\t" << "StdDev" << "\t" << "Var"*/ << std::endl;
	//value:
	if(m_averageLatencyTotal.m_averageLatencyINV.m_averageResult.Count() != 0)
		*m_outputStream->GetStream() << m_averageLatencyTotal.m_averageLatencyINV.Avg() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyINV.Min() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyINV .Max() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyINV.MinMin() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyINV.MaxMax()/*<< "\t"
			<< m_averageLatencyTotal.m_averageLatencyINV.Stddev() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyINV.Var() << "\t" */<< std::endl;
	else
		*m_outputStream->GetStream() << "-1" << "\t" << "-1" << "\t" <<  "-1" << "\t" << "-1" << "\t" <<  "-1"/*<< "\t" << "-1"*/ << std::endl;

	//PREF header :
	*m_outputStream->GetStream() << "Average latency PREF (microsec.)" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax"  /*<< "\t" << "StdDev" << "\t" << "Var"*/ << std::endl;
	//value:
	if(m_averageLatencyTotal.m_averageLatencyPREF.m_averageResult.Count() != 0)
		*m_outputStream->GetStream() << m_averageLatencyTotal.m_averageLatencyPREF.Avg() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyPREF.Min() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyPREF.Max() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyPREF.MinMin() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyPREF.MaxMax() /*<< "\t"
			<< m_averageLatencyTotal.m_averageLatencyPREF.Stddev() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyPREF.Var() << "\t" */<< std::endl;
	else
		*m_outputStream->GetStream() << "-1" << "\t" << "-1" << "\t" <<  "-1" << "\t" << "-1" << "\t" <<  "-1" /*<< "\t" << "-1" */<< std::endl;

	//No Dupl. header :
	if(m_selectedDuplNodes.size() != m_selectedRandomNodes.size())	//all nodes duplicated
	{
		*m_outputStream->GetStream() << "Average latency No Dupl. (milli(microsec.)" << "\t" << "Min" << "\t"
			<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" /*<< "\t" << "StdDev" << "\t" << "Var" */<< std::endl;
	//value:
		*m_outputStream->GetStream() << m_averageLatencyTotal.m_averageLatencyNoDupl.Avg() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyNoDupl.Min() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyNoDupl.Max() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyNoDupl.MinMin() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyNoDupl.MaxMax()/*<< "\t"
			<< m_averageLatencyTotal.m_averageLatencyNoDupl.Stddev() << "\t"
			<< m_averageLatencyTotal.m_averageLatencyNoDupl.Var() << "\t" */<< std::endl;
	}

	//Dupl. header :
	if(!m_selectedDuplNodes.empty())
	{
		*m_outputStream->GetStream() << "Average latency Dupl (microsec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" /* << "\t" << "StdDev" << "\t" << "Var" */<< std::endl;
		//value:
		*m_outputStream->GetStream() << m_averageLatencyTotal.m_averageLatencyDupl.Avg() << "\t"
				<< m_averageLatencyTotal.m_averageLatencyDupl.Min() << "\t"
				<< m_averageLatencyTotal.m_averageLatencyDupl.Max() << "\t"
				<< m_averageLatencyTotal.m_averageLatencyDupl.MinMin() << "\t"
				<< m_averageLatencyTotal.m_averageLatencyDupl.MaxMax() /*<< "\t"
				<< m_averageLatencyTotal.m_averageLatencyDupl.Stddev() << "\t"
				<< m_averageLatencyTotal.m_averageLatencyDupl.Var() << "\t" */<< std::endl;
	}
}

void ResultsCollection::PrintPartialResultMlifeSingleRun(std::string additionalInfo, uint32_t hopLimit,
		int noOfRuns, std::string lineEnd, bool printTotal, std::string midSign)
{
	if(noOfRuns == -1)
			noOfRuns = m_noOfRunsMLmNS;

	if(!m_resultPrintInit)
	{
		m_resultPrintInit = true;
		*m_outputStream->GetStream () << "Results for Single Node experiment (Message lifetime):" << std::endl
				<< additionalInfo << std::endl << std::endl;

		*m_outputStream->GetStream () << "(Single node run - hopLimit vs average message lifetime)" <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average mNS message lifetime (millisec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
				<< "\t" << "confidenceInt";

		if(!m_selectedRandomNodes.empty())
			*m_outputStream->GetStream() << "\t" << "selected Nodes";

		if(printTotal)
		{
			//header:
			*m_outputStream->GetStream() << midSign << "HopLimit" << "\t" << "Average mND (mNS+mNA) message lifetime (millisec.)" << "\t" << "Min" << "\t"
					<< "Max" << "\t" << "StdDev" << "\t" << "Var" << "\t" << "noOfRuns" << "\t" << "currAccuracy"
					<< "\t" << "confidenceInt";
		}

		*m_outputStream->GetStream()<< std::endl;
	}

	*m_outputStream->GetStream() << hopLimit << "\t" << m_average_mNSlife_singleRun.Avg() << "\t" << m_average_mNSlife_singleRun.Min()
			<< "\t" << m_average_mNSlife_singleRun.Max() << "\t" << m_average_mNSlife_singleRun.Stddev() << "\t"
			<< m_average_mNSlife_singleRun.Var() << "\t" << noOfRuns << "\t" << m_currAccuracyMLmNS
			<< "\t" << m_confidenceIntMLmNS;

	if(!m_selectedRandomNodes.empty())
	{
		*m_outputStream->GetStream() << "\t";

		for(ListIntCI it = m_selectedRandomNodes.begin(); it != m_selectedRandomNodes.end(); it++)
			*m_outputStream->GetStream() << *it << ", ";
	}

	if(printTotal)
	{
		*m_outputStream->GetStream() << midSign << hopLimit << "\t" << m_average_mNDMessLife_singleRun.Avg() << "\t" << m_average_mNDMessLife_singleRun.Min()
			<< "\t" << m_average_mNDMessLife_singleRun.Max() << "\t" << m_average_mNDMessLife_singleRun.Stddev() << "\t"
			<< m_average_mNDMessLife_singleRun.Var() << "\t" << m_noOfRunsMLTot << "\t" << m_currAccuracyMLTot
			<< "\t" << m_confidenceIntMLTot;
	}

	*m_outputStream->GetStream()<< lineEnd << std::flush;
}

void ResultsCollection::PrintResultMessageLifetime(std::string additionalInfo, uint32_t hopLimit, int noOfRuns, bool printTotal)
{
	if(!m_resultPrintInit)
	{
		*m_outputStream->GetStream () << std::endl << std::endl << "Results for Message Lifetime experiment:" << std::endl;
		if(additionalInfo.compare(""))
			*m_outputStream->GetStream () << additionalInfo << std::endl;

		*m_outputStream->GetStream () << "(HopLimit vs. average mNS message lifetime)" <<std::endl;
		//header:
		*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average mNS message lifetime (millisec.)" << "\t" << "Min" << "\t"
				<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" << "\t" << "noOfRuns" << std::endl;
	}

	*m_outputStream->GetStream() << hopLimit << "\t" << m_averageResult_mNSLife.m_averageResult.Avg() << "\t"
			<< m_averageResult_mNSLife.Min() << "\t"
			<< m_averageResult_mNSLife.Max() << "\t"
			<< m_averageResult_mNSLife.MinMin() << "\t"
			<< m_averageResult_mNSLife.MaxMax() << "\t" << noOfRuns << std::endl;

	if(printTotal)
	{
		if(!m_resultPrintInit)
		{
			*m_outputStream->GetStream () << std::endl << std::endl << "Results for Message Lifetime experiment:" << std::endl;
			if(additionalInfo.compare(""))
				*m_outputStream->GetStream () << additionalInfo << std::endl;

			*m_outputStream->GetStream () << "(HopLimit vs. average mND message lifetime (mNS + mNA))" <<std::endl;
			//header:
			*m_outputStream->GetStream() << "HopLimit" << "\t" << "Average mND message lifetime (millisec.)" << "\t" << "Min" << "\t"
					<< "Max" << "\t" << "MinMin" << "\t" << "MaxMax" << "\t" << "noOfRuns" << std::endl;
		}

		*m_outputStream->GetStream() << hopLimit << "\t" << m_averageResult_mNDMessLife.m_averageResult.Avg() << "\t"
				<< m_averageResult_mNDMessLife.Min() << "\t"
				<< m_averageResult_mNDMessLife.Max() << "\t"
				<< m_averageResult_mNDMessLife.MinMin() << "\t"
				<< m_averageResult_mNDMessLife.MaxMax() << "\t"<< noOfRuns << std::endl;
	}

	m_resultPrintInit = true;
}

void ResultsCollection::UpdateHopLimitStatsSingleRun(InternetStackHelper_Nd* internetHelper, bool updatemNS, bool updateTot)
{
	int txTotalMultihopNS = 0;
	int txTotalMultihopNDpp = 0;
	for(InternetStackHelper_Nd::DadStatsMessageTypeI it = internetHelper->m_txDadStatsMessageType.begin();
			it != internetHelper->m_txDadStatsMessageType.end(); it++)
	{
		 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::NdppMessageTypeSize + 2,
				 "Assertion failed! - txMessages vector length not corresponding to expected size");

		 if(updatemNS)
			 txTotalMultihopNS += (*(*it))[Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP+2];
		 if(updateTot)
			 txTotalMultihopNDpp += (*(*it))[Icmpv6L4Protocol_NdSimple::NDPP_NS_MHOP+2] +
			 	 (*(*it))[Icmpv6L4Protocol_NdSimple::NDPP_NA_MHOP+2] +
			 	(*(*it))[Icmpv6L4Protocol_NdSimple::NDPP_NS_1HOP+2] +
			 	(*(*it))[Icmpv6L4Protocol_NdSimple::NDPP_NA_1HOP+2];
	}
	if(updatemNS)
	{
		m_average_mNSolCount_singleRun.Update(txTotalMultihopNS);
		NS_LOG_INFO("txTotalMultihopNS: " << txTotalMultihopNS);
	}
	if(updateTot)
	{
		m_average_mNDMessCount_singleRun.Update(txTotalMultihopNDpp);
		NS_LOG_INFO("txTotalMultihop ND++ (NS+NA): " << txTotalMultihopNDpp);
	}
}

void ResultsCollection::UpdateHopLimitStats()
{
	m_averageResult_mNSCount.m_averageResult.Update(m_average_mNSolCount_singleRun.Avg());
	m_averageResult_mNSCount.m_minResult.Update(m_average_mNSolCount_singleRun.Min());
	m_averageResult_mNSCount.m_maxResult.Update(m_average_mNSolCount_singleRun.Max());

	m_averageResult_mNDMessCount.m_averageResult.Update(m_average_mNDMessCount_singleRun.Avg());
	m_averageResult_mNDMessCount.m_minResult.Update(m_average_mNDMessCount_singleRun.Min());
	m_averageResult_mNDMessCount.m_maxResult.Update(m_average_mNDMessCount_singleRun.Max());

	NS_LOG_INFO("m_average_mNSolCount_singleRun: mean=" << m_average_mNSolCount_singleRun.Avg()
			<< "stdDev=" << m_average_mNSolCount_singleRun.Stddev());
	NS_LOG_INFO("m_average_mNDMessCount_singleRun: mean=" << m_average_mNDMessCount_singleRun.Avg()
				<< "stdDev=" << m_average_mNDMessCount_singleRun.Stddev());
}

void ResultsCollection::UpdateMLifeStatsSingleRun(InternetStackHelper_Nd* internetHelper, bool updatemNS, bool updateTot)
{
	if(updatemNS)
	{
		if(internetHelper->m_first_mNStime > 0 && internetHelper->m_last_mNStime < 0)
		{
			//the node could have been isolated from the network or the initial message could have been lost and couldn't reach any other nodes
			//set result to 0
			m_average_mNSlife_singleRun.Update(0);
		}
		else
		{
			NS_ASSERT_MSG(internetHelper->m_last_mNStime >= internetHelper->m_first_mNStime, "last_mNStime < "
					"internetHelper->m_first_mNStime - sth is wrong!!!");
			m_average_mNSlife_singleRun.Update((internetHelper->m_last_mNStime
					- internetHelper->m_first_mNStime).GetMilliSeconds());
		}
	}
	if(updateTot)
	{
		if(internetHelper->m_first_mNDMessTime > 0 && internetHelper->m_last_mNDTtime < 0)
		{
			//the node could have been isolated from the network or the initial message could have been lost and couldn't reach any other nodes
			//set result to 0
			m_average_mNDMessLife_singleRun.Update(0);
		}
		else
		{
			NS_ASSERT_MSG(internetHelper->m_last_mNDTtime >= internetHelper->m_first_mNDMessTime, "m_last_mNDTtime < "
					"internetHelper->m_first_mNDMessTime - sth is wrong!!!");
			m_average_mNDMessLife_singleRun.Update((internetHelper->m_last_mNDTtime
					- internetHelper->m_first_mNDMessTime).GetMilliSeconds());
		}
	}
}

void ResultsCollection::UpdateLatencyStatsRNSingleRun(InternetStackHelper_Nd* internetHelper)
{
	std::map<int,DupProbInfo>::iterator it2;
	std::pair<LatencyI, bool> insertResult;
	int64_t currLatency = 0;

	for(std::list<LatencyPerNode *>::iterator it = internetHelper->m_latencies.begin();
			it != internetHelper->m_latencies.end();it++)
	{
		if( (*it)->m_latencyInitTime >= Seconds(0.0))
		{
			if((*it)->m_latencyInvalidTime >= Seconds(0.0))
			{
				NS_ASSERT_MSG((*it)->m_latencyInvalidTime > (*it)->m_latencyInitTime, "Latency updates: init time <= "
							"invalid time - sth is wrong!!!");
				insertResult = m_averageLatencyPerNode.insert(std::make_pair((*it)->m_nodeNo, LatencyInfo()));
				currLatency = ((*it)->m_latencyInvalidTime - (*it)->m_latencyInitTime).GetMicroSeconds();
				insertResult.first->second.m_averageLatencyINV.m_averageResult.Update(currLatency);
			}
			if ((*it)->m_latencyPreferredTime >= Seconds(0.0))
			{
				NS_ASSERT_MSG((*it)->m_latencyPreferredTime > (*it)->m_latencyInitTime, "Latency updates: init time <= "
											"preferred time - sth is wrong!!!");
				insertResult = m_averageLatencyPerNode.insert(std::make_pair((*it)->m_nodeNo, LatencyInfo()));
				currLatency = ((*it)->m_latencyPreferredTime - (*it)->m_latencyInitTime).GetMicroSeconds();
				(insertResult.first)->second.m_averageLatencyPREF.m_averageResult.Update(currLatency);
			}
			if((*it)->m_latencyInvalidTime < Seconds(0.0) && (*it)->m_latencyPreferredTime < Seconds(0.0))
				NS_ASSERT_MSG(false, "LAtency Trace data collection: init time registered, but neither invalid nor preferred time recorded!!!");

			NS_ASSERT_MSG(currLatency > 0, "Latency Trace: Init time registered, but latency measured is <= 0 !!!!");
			(insertResult.first)->second.m_averageLatency.m_averageResult.Update(currLatency);

			if(insertResult.second)
			{
				it2 = m_selectedDuplNodes.find((*it)->m_nodeNo);
				(insertResult.first)->second.m_duplAddress = (it2 != m_selectedDuplNodes.end())? true : false;
			}
		}
	}
}

void ResultsCollection::UpdateLatencyStatsRN()
{
	m_averageLatencyPerRNSelection.push_back(LatencyInfo());

	//update PerRNSelection stats
	for(LatencyI it = m_averageLatencyPerNode.begin(); it != m_averageLatencyPerNode.end();it++)
	{
		if(it->second.m_averageLatencyINV.m_averageResult.Count())
			m_averageLatencyPerRNSelection.back().m_averageLatencyINV.m_averageResult.Update(it->second.m_averageLatencyINV.Avg());
		if(it->second.m_averageLatencyPREF.m_averageResult.Count())
			m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.m_averageResult.Update(it->second.m_averageLatencyPREF.Avg());
		m_averageLatencyPerRNSelection.back().m_averageLatency.m_averageResult.Update(it->second.m_averageLatency.Avg());

		if(it->second.m_duplAddress)
			m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.m_averageResult.Update(it->second.m_averageLatency.Avg());
		else
			m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.m_averageResult.Update(it->second.m_averageLatency.Avg());
	}

	//update final stats - building step by step an average over all positions in m_averageLatencyPerRNSelection:
	m_averageLatencyTotal.m_averageLatency.m_averageResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatency.Avg());
	m_averageLatencyTotal.m_averageLatency.m_minResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatency.Min());
	m_averageLatencyTotal.m_averageLatency.m_maxResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatency.Max());

	if(m_averageLatencyPerRNSelection.back().m_averageLatencyINV.m_averageResult.Count())
	{
		m_averageLatencyTotal.m_averageLatencyINV.m_averageResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyINV.Avg());
		m_averageLatencyTotal.m_averageLatencyINV.m_minResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyINV.Min());
		m_averageLatencyTotal.m_averageLatencyINV.m_maxResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyINV.Max());
	}
	if(m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.m_averageResult.Count())
	{
		m_averageLatencyTotal.m_averageLatencyPREF.m_averageResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.Avg());
		m_averageLatencyTotal.m_averageLatencyPREF.m_minResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.Min());
		m_averageLatencyTotal.m_averageLatencyPREF.m_maxResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyPREF.Max());
	}
	if(m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.m_averageResult.Count())
	{
		m_averageLatencyTotal.m_averageLatencyDupl.m_averageResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.Avg());
		m_averageLatencyTotal.m_averageLatencyDupl.m_minResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.Min());
		m_averageLatencyTotal.m_averageLatencyDupl.m_maxResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyDupl.Max());
	}
	if(m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.m_averageResult.Count())
	{
		m_averageLatencyTotal.m_averageLatencyNoDupl.m_averageResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.Avg());
		m_averageLatencyTotal.m_averageLatencyNoDupl.m_minResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.Min());
		m_averageLatencyTotal.m_averageLatencyNoDupl.m_maxResult.Update(m_averageLatencyPerRNSelection.back().m_averageLatencyNoDupl.Max());
	}

}


void ResultsCollection::UpdateMessageLifetimeStats()
{
	m_averageResult_mNSLife.m_averageResult.Update(m_average_mNSlife_singleRun.Avg());
	m_averageResult_mNSLife.m_minResult.Update(m_average_mNSlife_singleRun.Min());
	m_averageResult_mNSLife.m_maxResult.Update(m_average_mNSlife_singleRun.Max());

	m_averageResult_mNDMessLife.m_averageResult.Update(m_average_mNDMessLife_singleRun.Avg());
	m_averageResult_mNDMessLife.m_minResult.Update(m_average_mNDMessLife_singleRun.Min());
	m_averageResult_mNDMessLife.m_maxResult.Update(m_average_mNDMessLife_singleRun.Max());

	NS_LOG_INFO("m_average_mNSlife_singleRun: mean=" << m_average_mNSlife_singleRun.Avg()
			<< "stdDev=" << m_average_mNSlife_singleRun.Stddev());
	NS_LOG_INFO("m_average_mNDMessLife_singleRun: mean=" << m_average_mNDMessLife_singleRun.Avg()
				<< "stdDev=" << m_average_mNDMessLife_singleRun.Stddev());
}

void ResultsCollection::UpdateDupProbStats(int noOfRuns)
{
	if(noOfRuns == -1)
		noOfRuns = m_noOfRunsDup;
	m_averageDupProbAll.Update(((double)m_dupDetectCountAll) / noOfRuns);
	m_averageDupProb90.Update(((double)m_dupDetectCount90) / noOfRuns);
	m_averageDupProb75.Update(((double)m_dupDetectCount75) / noOfRuns);
	m_averageDupProb50.Update(((double)m_dupDetectCount50) / noOfRuns);

	/*NS_LOG_INFO("m_averageDupProbAll: mean=" << m_average_mNSlife_singleRun.Avg()
			<< "stdDev=" << m_average_mNSlife_singleRun.Stddev());*/
}

void ResultsCollection::UpdateDupDetectCount(InternetStackHelper_Nd* internetHelper, uint32_t nodeNo)
{
	for(InternetStackHelper_Nd::DadStatsExitCodeI it = internetHelper->m_exitCodeDadStats.begin();
			it != internetHelper->m_exitCodeDadStats.end(); it++)
	{
		 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::ExitCodeTypeSize + 2,
				 "Assertion failed! - exitCodeDadStats vector length not corresponding to expected size");

		 if((*(*it))[0] == (int32_t)nodeNo)
		 {
			 NS_ASSERT_MSG((*(*it))[Icmpv6L4Protocol_NdSimple::DAD_DUPLICATION + 2] == 0
					 || (*(*it))[Icmpv6L4Protocol_NdSimple::DAD_DUPLICATION + 2] == 1,
					 "DAD duplication found state reported more than once!!! - sth is wrong");
			 NS_ASSERT_MSG((*(*it))[Icmpv6L4Protocol_NdSimple::OTHERS_WITH_MY_ADDR + 2] == 0
			 					 || (*(*it))[Icmpv6L4Protocol_NdSimple::OTHERS_WITH_MY_ADDR + 2] == 1,
			 					 "OTHERS_WITH_MY_ADDR state reported more than once!!! - sth is wrong");
			 m_dupDetectCount = m_dupDetectCount + (*(*it))[Icmpv6L4Protocol_NdSimple::DAD_DUPLICATION + 2]
			                          + (*(*it))[Icmpv6L4Protocol_NdSimple::OTHERS_WITH_MY_ADDR + 2];
			 break;
		 }
	}
	NS_LOG_INFO("m_dupDetectCount: " << m_dupDetectCount);
}

void ResultsCollection::UpdateDupDetectCountRandomNodes(InternetStackHelper_Nd* internetHelper, int noOfDuplNodes)
{
	NS_ASSERT_MSG(!m_selectedDuplNodes.empty(), "Results collection - Selected duplicated nodes list empty, but dup. detection experiment is performed!!!");
	int counter = 0; 	//count for how many nodes DAD detected duplication
	std::map<int,DupProbInfo>::iterator it2;

	for(InternetStackHelper_Nd::DadStatsExitCodeI it = internetHelper->m_exitCodeDadStats.begin();
			it != internetHelper->m_exitCodeDadStats.end(); it++)
	{
		 NS_ASSERT_MSG((*it)->size() == Icmpv6L4Protocol_NdSimple::ExitCodeTypeSize + 2,
				 "Assertion failed! - exitCodeDadStats vector length not corresponding to expected size");
		 NS_ASSERT_MSG((*(*it))[Icmpv6L4Protocol_NdSimple::DAD_DUPLICATION + 2] == 0
				 || (*(*it))[Icmpv6L4Protocol_NdSimple::DAD_DUPLICATION + 2] == 1,
				 "DAD duplication found state reported more than once!!! - sth is wrong");
		 NS_ASSERT_MSG((*(*it))[Icmpv6L4Protocol_NdSimple::OTHERS_WITH_MY_ADDR + 2] == 0
				 || (*(*it))[Icmpv6L4Protocol_NdSimple::OTHERS_WITH_MY_ADDR + 2] == 1,
				 "OTHERS_WITH_MY_ADDR state reported more than once!!! - sth is wrong");

		 it2 = m_selectedDuplNodes.find((*(*it))[0]);
		 if( it2 != m_selectedDuplNodes.end())
		 {
			 counter += (*(*it))[Icmpv6L4Protocol_NdSimple::DAD_DUPLICATION + 2]
			                      + (*(*it))[Icmpv6L4Protocol_NdSimple::OTHERS_WITH_MY_ADDR + 2];
			 it2->second.m_dupCount += (*(*it))[Icmpv6L4Protocol_NdSimple::DAD_DUPLICATION + 2]
			                             +  (*(*it))[Icmpv6L4Protocol_NdSimple::OTHERS_WITH_MY_ADDR + 2];
		 }
	}
	if(counter == noOfDuplNodes)
		m_dupDetectCountAll++;
	if (counter >= 0.9 * noOfDuplNodes)
		m_dupDetectCount90++;
	if (counter >= 0.75 * noOfDuplNodes)
		m_dupDetectCount75++;
	if (counter >= 0.5 * noOfDuplNodes)
		m_dupDetectCount50++;

	NS_LOG_INFO("m_dupDetectCountAll: " << m_dupDetectCountAll);
	NS_LOG_INFO("m_dupDetectCount90: " << m_dupDetectCount90);
	NS_LOG_INFO("m_dupDetectCount75: " << m_dupDetectCount75);
	NS_LOG_INFO("m_dupDetectCount50: " << m_dupDetectCount50);
}

int ResultsCollection::GetDupDetectCount()
{
	return m_dupDetectCount;
}

void ResultsCollection::ResetStats()
{
	m_averageResult_mNSCount.Reset();
	m_averageResult_mNDMessCount.Reset();
	m_averageResult_mNSLife.Reset();
	m_averageResult_mNDMessLife.Reset();
	m_averageDupProbPerNode.Reset();
	m_dupDetectCount = 0;
	m_average_mNSlife_singleRun.Reset();
	m_average_mNDMessLife_singleRun.Reset();
	m_currAccuracyHLmNS = -1.0;
	m_currAccuracyHLTot = -1.0;
	m_currAccuracyMLmNS = -1.0;
	m_currAccuracyMLTot = -1.0;
	m_confidenceIntHLmNS = -1.0;
	m_confidenceIntHLTot = -1.0;
	m_confidenceIntMLmNS = -1.0;
	m_confidenceIntMLTot = -1.0;

	for(std::list<LatencyInfo>::iterator it = m_averageLatencyPerRNSelection.begin(); it != m_averageLatencyPerRNSelection.end(); it++)
	{
		it->m_averageLatencyINV.Reset();
		it->m_averageLatencyPREF.Reset();
		it->m_averageLatency.Reset();
		it->m_averageLatencyDupl.Reset();
		it->m_averageLatencyNoDupl.Reset();
	}
	m_averageLatencyPerRNSelection.clear();

	for(LatencyI it = m_averageLatencyPerNode.begin(); it != m_averageLatencyPerNode.end(); it++)
	{
		it->second.m_averageLatencyINV.Reset();
		it->second.m_averageLatencyPREF.Reset();
		it->second.m_averageLatency.Reset();
		it->second.m_averageLatencyDupl.Reset();
		it->second.m_averageLatencyNoDupl.Reset();

	}
	m_averageLatencyPerNode.clear();

	m_averageLatencyTotal.m_averageLatency.Reset();
	m_averageLatencyTotal.m_averageLatencyINV.Reset();
	m_averageLatencyTotal.m_averageLatencyPREF.Reset();
	m_averageLatencyTotal.m_averageLatencyDupl.Reset();
	m_averageLatencyTotal.m_averageLatencyNoDupl.Reset();

}

void ResultsCollection::ResetAverageStatsSingleRun()
{
	m_average_mNSolCount_singleRun.Reset();
	m_average_mNDMessCount_singleRun.Reset();
	m_average_mNSlife_singleRun.Reset();
	m_average_mNDMessLife_singleRun.Reset();
	m_currAccuracyHLmNS = -1.0;
	m_confidenceIntHLmNS = -1.0;
	m_currAccuracyMLmNS = -1.0;
	m_confidenceIntMLmNS = -1.0;
	m_currAccuracyHLTot = -1.0;
	m_confidenceIntHLTot = -1.0;
	m_currAccuracyMLTot = -1.0;
	m_confidenceIntMLTot = -1.0;
	m_dupDetectCountAll = 0;
	m_dupDetectCount90 = 0;
	m_dupDetectCount75 = 0;
	m_dupDetectCount50 = 0;

	LatencyI it;
	for(it = m_averageLatencyPerNode.begin(); it != m_averageLatencyPerNode.end(); it++)
	{
		it->second.m_averageLatencyINV.Reset();
		it->second.m_averageLatencyPREF.Reset();
		it->second.m_averageLatency.Reset();
		it->second.m_averageLatencyDupl.Reset();
		it->second.m_averageLatencyNoDupl.Reset();
	}
	m_averageLatencyPerNode.clear();
}

} /* namespace ns3 */
#endif /* IPV6_NDPP_EXPERIMENTS_H_ */


