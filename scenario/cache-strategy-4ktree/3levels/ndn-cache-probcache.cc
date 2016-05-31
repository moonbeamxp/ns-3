
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define Strategy	"Probcache"
#define CS_Capacity	"40"
#define Capacity	"10000"
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

void
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
}

int 
main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Read the Topology information
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topology/topo-3-level-4k-tree.txt");
  topologyReader.Read ();
  
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Probcache");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity); 
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // Producer
  Ptr<Node> producers[1] = { Names::Find<Node> ("Root")};
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerProbcache");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1026"));
  producerHelper.SetPrefix ("/prefix");
  ndnGlobalRoutingHelper.AddOrigins ("/prefix", producers[0]);
  producerHelper.Install (producers[0]); // root node
  
  // Consumer
  Ptr<Node> consumers[9] = { 	Names::Find<Node> ("R11"), Names::Find<Node> ("R12"), Names::Find<Node> ("R13"),
				Names::Find<Node> ("R21"), Names::Find<Node> ("R22"), Names::Find<Node> ("R31"),
				Names::Find<Node> ("R32"), Names::Find<Node> ("R33"), Names::Find<Node> ("R34") };

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity));
  consumerHelper.SetAttribute ("q", StringValue ("0"));
  consumerHelper.SetAttribute ("s", StringValue (Alpha));
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate)); // Rate interests a second
  for(int i=0; i<=8; i++)
  {
    consumerHelper.Install (consumers[i]);
  }
  
  //Calculate the global routing
  ndnGlobalRoutingHelper.CalculateRoutes ();
  
  ndn::L3AggregateTracer::InstallAll (aggregate_trace, Seconds (1.0));
  ndn::L3RateTracer::InstallAll (rate_trace, Seconds (1.0));
  ndn::CsTracer::InstallAll (cs_trace, Seconds (1));
  ndn::AppDelayTracer::InstallAll (app_delays_trace);
  L2RateTracer::InstallAll (drop_trace, Seconds (1.0));
  
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
  {
    Simulator::Schedule (Seconds (50), PeriodicStatsPrinter, *i, Seconds (30));
  }
  
  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
