
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define Strategy	"PopcacheOnce"
#define CS_Capacity	"40"
#define Capacity	"10000"
#define Alpha		"1.1"
#define Rate		"1000"

#define suffix			Strategy"-"Capacity"-"CS_Capacity"-"Rate"-"Alpha".txt"
#define aggregate_trace		"result/aggregate-trace-"suffix
#define rate_trace		"result/rate-trace-"suffix
#define cs_trace		"result/cs-trace-"suffix
#define app_delays_trace	"result/app-delays-trace-"suffix
#define drop_trace		"result/drop-trace-"suffix

using namespace ns3;

int 
main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Read the Topology information
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topology/topo-5-level-tree.txt");
  topologyReader.Read ();
  
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::PopcacheOnce");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity);
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // Producer
  Ptr<Node> producers[1] = { Names::Find<Node> ("Root")};
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerPopcache");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.SetPrefix ("/prefix");
  ndnGlobalRoutingHelper.AddOrigins ("/prefix", producers[0]);
  producerHelper.Install (producers[0]); // root node
  
  // Consumer
  Ptr<Node> consumers[16] = { Names::Find<Node> ("Leaf1"),  Names::Find<Node> ("Leaf2"), 
                              Names::Find<Node> ("Leaf3"),  Names::Find<Node> ("Leaf4"), 
                              Names::Find<Node> ("Leaf5"),  Names::Find<Node> ("Leaf6"), 
                              Names::Find<Node> ("Leaf7"),  Names::Find<Node> ("Leaf8"), 
                              Names::Find<Node> ("Leaf9"),  Names::Find<Node> ("Leaf10"), 
                              Names::Find<Node> ("Leaf11"), Names::Find<Node> ("Leaf12"), 
                              Names::Find<Node> ("Leaf13"), Names::Find<Node> ("Leaf14"), 
                              Names::Find<Node> ("Leaf15"), Names::Find<Node> ("Leaf16") };
                                                        
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrotwithPop");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity));
  consumerHelper.SetAttribute ("q", StringValue ("0"));
  consumerHelper.SetAttribute ("s", StringValue (Alpha));
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate)); // Rate interests a second
  for(int i=0; i<=15; i++)
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
  
  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
