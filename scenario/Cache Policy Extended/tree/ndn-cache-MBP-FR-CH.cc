
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define Strategy	"MBP-FR-CH"
#define CS_Capacity	"40"
#define Capacity	"50000"
#define Alpha		"1.0"
#define Rate		"200"

#define suffix			Strategy"-"Capacity"-"CS_Capacity"-"Rate"-"Alpha".txt"
#define aggregate_trace		"result/aggregate-trace-"suffix
#define rate_trace		"result/rate-trace-"suffix
#define cs_trace		"result/cs-trace-"suffix
#define app_delays_trace	"result/app-delays-trace-"suffix
#define drop_trace		"result/drop-trace-"suffix
#define cs_index		"result/cs-index-"suffix

using namespace ns3;

/*void
PeriodicStatsPrinter (Ptr<Node> node, Time next)
{
  Ptr<ndn::ContentStore> cs = node->GetObject<ndn::ContentStore> ();
  Ptr<ns3::ndn::cs::Entry> item = cs -> Begin();
  std::ofstream of(cs_index, std::ios::app);
  while(item)
  {
    of << Simulator::Now ().ToDouble (Time::S) << "\t"
       << node->GetId () << "\t"
       << Names::FindName (node) << "\t"
       << item->GetName () << "\n";
    item = cs -> Next(item);
  }
  of.flush();
  of.close(); 
  Simulator::Schedule (next, PeriodicStatsPrinter, node, next);
}*/

int 
main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Read the Topology information
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topology/topo-6-level-4k-tree-p2p.txt");
  topologyReader.Read ();
  
  // Define Producer
  Ptr<Node> producers[1] = {	Names::Find<Node> ("Root")};
    
/*  // Define Router
  Ptr<Node> routers[33] = {	Names::Find<Node> ("R1"),	Names::Find<Node> ("R2"),	Names::Find<Node> ("R3"),
  				Names::Find<Node> ("R11"), 	Names::Find<Node> ("R13"), 	Names::Find<Node> ("R21"),
				Names::Find<Node> ("R22"), 	Names::Find<Node> ("R31"),   	Names::Find<Node> ("R32"),
				Names::Find<Node> ("R33"), 	Names::Find<Node> ("R111"), 	Names::Find<Node> ("R112"),
				Names::Find<Node> ("R131"), 	Names::Find<Node> ("R132"), 	Names::Find<Node> ("R133"),
				Names::Find<Node> ("R211"), 	Names::Find<Node> ("R213"), 	Names::Find<Node> ("R222"),
				Names::Find<Node> ("R223"), 	Names::Find<Node> ("R224"), 	Names::Find<Node> ("R311"),
				Names::Find<Node> ("R312"), 	Names::Find<Node> ("R313"),  	Names::Find<Node> ("R321"),
				Names::Find<Node> ("R322"), 	Names::Find<Node> ("R331"), 	Names::Find<Node> ("R332"),
				Names::Find<Node> ("R1331"), 	Names::Find<Node> ("R2111"),  	Names::Find<Node> ("R3311"),
				Names::Find<Node> ("R34"), 	Names::Find<Node> ("R341"), 	Names::Find<Node> ("R342") };
*/
  // Define Consumer
  Ptr<Node> consumers[66] = { 	Names::Find<Node> ("R1111"),  Names::Find<Node> ("R1112"),  Names::Find<Node> ("R1113"),
				Names::Find<Node> ("R1121"),  Names::Find<Node> ("R1122"),  Names::Find<Node> ("R1123"),
				Names::Find<Node> ("R1124"),  Names::Find<Node> ("R12"),    Names::Find<Node> ("R1311"),
				Names::Find<Node> ("R1312"),  Names::Find<Node> ("R1321"),  Names::Find<Node> ("R3322"),
				Names::Find<Node> ("R1332"),  Names::Find<Node> ("R1333"),  Names::Find<Node> ("R134"),
				Names::Find<Node> ("R333"),   Names::Find<Node> ("R2112"),  Names::Find<Node> ("R212"),
				Names::Find<Node> ("R2131"),  Names::Find<Node> ("R2132"),  Names::Find<Node> ("R2133"),
				Names::Find<Node> ("R2134"),  Names::Find<Node> ("R221"),   Names::Find<Node> ("R2221"),
				Names::Find<Node> ("R2222"),  Names::Find<Node> ("R2231"),  Names::Find<Node> ("R2241"),
				Names::Find<Node> ("R2242"),  Names::Find<Node> ("R2243"),  Names::Find<Node> ("R3111"),
				Names::Find<Node> ("R3112"),  Names::Find<Node> ("R3121"),  Names::Find<Node> ("R3131"),
				Names::Find<Node> ("R3132"),  Names::Find<Node> ("R3133"),  Names::Find<Node> ("R3134"),
				Names::Find<Node> ("R3211"),  Names::Find<Node> ("R3212"),  Names::Find<Node> ("R3213"),
				Names::Find<Node> ("R3221"),  Names::Find<Node> ("R3222"),  Names::Find<Node> ("R3223"),
				Names::Find<Node> ("R3224"),  Names::Find<Node> ("R343"),   Names::Find<Node> ("R3312"),
				Names::Find<Node> ("R3313"),  Names::Find<Node> ("R3314"),  Names::Find<Node> ("R3321"),
				Names::Find<Node> ("R13311"), Names::Find<Node> ("R13312"), Names::Find<Node> ("R13313"),
				Names::Find<Node> ("R21111"), Names::Find<Node> ("R21112"), Names::Find<Node> ("R21113"),
				Names::Find<Node> ("R33111"), Names::Find<Node> ("R33112"), Names::Find<Node> ("R33113"),
				Names::Find<Node> ("R344"),   Names::Find<Node> ("R3411"),  Names::Find<Node> ("R3412"),
				Names::Find<Node> ("R3413"),  Names::Find<Node> ("R3414"),  Names::Find<Node> ("R3421"),
				Names::Find<Node> ("R3422"),  Names::Find<Node> ("R3423"),  Names::Find<Node> ("R3424") };
  
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Popcachefrmidstat");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity);
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // for producers
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerPopcachestat");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.SetPrefix ("/prefix");
  ndnGlobalRoutingHelper.AddOrigins ("/prefix", producers[0]);
  
  producerHelper.Install (producers[0]); // root node
  
  // for consumers
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrotUnique");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity));
  consumerHelper.SetAttribute ("q", StringValue ("0"));
  consumerHelper.SetAttribute ("s", StringValue (Alpha));
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate)); // Rate interests a second
  
  for(int i=0; i<=65; i++)
  {
    consumerHelper.Install (consumers[i]);
  }
  
  //Calculate the global routing
  ndnGlobalRoutingHelper.CalculateRoutes ();
  
  // tracers
  ndn::L3AggregateTracer::InstallAll (aggregate_trace, Seconds (1.0));
  ndn::L3RateTracer::InstallAll (rate_trace, Seconds (1.0));
  ndn::CsTracer::InstallAll (cs_trace, Seconds (1));
  ndn::AppDelayTracer::InstallAll (app_delays_trace);
  L2RateTracer::InstallAll (drop_trace, Seconds (1.0));
  
  /*for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
  {
    Simulator::Schedule (Seconds (50), PeriodicStatsPrinter, *i, Seconds (30));
  }*/
  
  Simulator::Stop (Seconds (120.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
