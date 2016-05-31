
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define Strategy	"Popcachestat"
#define CS_Capacity	"100"
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
  topologyReader.SetFileName ("topology/topo-ws-100-3-0.5.txt");
  topologyReader.Read ();
  
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Popcachestat");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity);  
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // Producer
  Ptr<Node> producers[2] = {	Names::Find<Node> ("0"),	Names::Find<Node> ("82")};
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerPopcachestat");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1026"));
  producerHelper.SetPrefix ("/prefix");
  ndnGlobalRoutingHelper.AddOrigins ("/prefix", producers[0]);
  producerHelper.Install (producers[0]); // root node
  producerHelper.Install (producers[1]); // root node
  
  // Consumer
  Ptr<Node> consumers[50] = {	Names::Find<Node> ("1"),	Names::Find<Node> ("3"),	Names::Find<Node> ("5"),
  				Names::Find<Node> ("7"), 	Names::Find<Node> ("9"), 	Names::Find<Node> ("11"),
				Names::Find<Node> ("13"), 	Names::Find<Node> ("15"),   	Names::Find<Node> ("17"),
				Names::Find<Node> ("19"), 	Names::Find<Node> ("21"), 	Names::Find<Node> ("23"),
				Names::Find<Node> ("25"), 	Names::Find<Node> ("27"), 	Names::Find<Node> ("29"),
				Names::Find<Node> ("31"), 	Names::Find<Node> ("33"), 	Names::Find<Node> ("35"),
				Names::Find<Node> ("37"), 	Names::Find<Node> ("39"), 	Names::Find<Node> ("41"),
				Names::Find<Node> ("43"), 	Names::Find<Node> ("45"),  	Names::Find<Node> ("47"),
				Names::Find<Node> ("49"), 	Names::Find<Node> ("51"), 	Names::Find<Node> ("53"),
				Names::Find<Node> ("55"), 	Names::Find<Node> ("57"), 	Names::Find<Node> ("59"),
				Names::Find<Node> ("61"), 	Names::Find<Node> ("63"), 	Names::Find<Node> ("65"),
				Names::Find<Node> ("67"),	Names::Find<Node> ("69"), 	Names::Find<Node> ("71"),
				Names::Find<Node> ("73"),	Names::Find<Node> ("75"),	Names::Find<Node> ("77"),
				Names::Find<Node> ("79"),	Names::Find<Node> ("81"),	Names::Find<Node> ("83"),
				Names::Find<Node> ("85"),	Names::Find<Node> ("87"),	Names::Find<Node> ("89"),
				Names::Find<Node> ("91"),	Names::Find<Node> ("93"),	Names::Find<Node> ("95"),
				Names::Find<Node> ("97"),	Names::Find<Node> ("99") };

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity));
  consumerHelper.SetAttribute ("q", StringValue ("0"));
  consumerHelper.SetAttribute ("s", StringValue (Alpha));
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate)); // Rate interests a second
  for(int i=0; i<=49; i++)
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
    Simulator::Schedule (Seconds (110), PeriodicStatsPrinter, *i, Seconds (30));
  }
  
  Simulator::Stop (Seconds (120.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
