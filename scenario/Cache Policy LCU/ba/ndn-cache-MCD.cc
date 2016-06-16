
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define Strategy	"MCD"
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

void
PeriodicStatsPrinter (Ptr<Node> node)
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
}

int 
main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Read the Topology information
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topology/topo-ba-100-1.txt");
  topologyReader.Read ();
  
  // Define Producer
  Ptr<Node> producers[1] = { 	Names::Find<Node> ("0")};
  
/*  // Define Router
  Ptr<Node> routers[33] = {	Names::Find<Node> ("1"),	Names::Find<Node> ("3"),	Names::Find<Node> ("4"),
				Names::Find<Node> ("5"),	Names::Find<Node> ("6"),	Names::Find<Node> ("8"),
				Names::Find<Node> ("9"),	Names::Find<Node> ("12"),	Names::Find<Node> ("13"),
				Names::Find<Node> ("14"),	Names::Find<Node> ("15"),	Names::Find<Node> ("16"),
				Names::Find<Node> ("18"),	Names::Find<Node> ("19"),	Names::Find<Node> ("21"),
				Names::Find<Node> ("23"),	Names::Find<Node> ("24"),	Names::Find<Node> ("25"),
				Names::Find<Node> ("27"),	Names::Find<Node> ("31"),	Names::Find<Node> ("33"),
				Names::Find<Node> ("35"),	Names::Find<Node> ("36"),	Names::Find<Node> ("37"),
				Names::Find<Node> ("39"),	Names::Find<Node> ("43"),	Names::Find<Node> ("49"),
				Names::Find<Node> ("50"),	Names::Find<Node> ("53"),	Names::Find<Node> ("73"),
				Names::Find<Node> ("75"),	Names::Find<Node> ("85"),	Names::Find<Node> ("86") }; 
*/			
  // Define Consumer
  Ptr<Node> consumers[66] = {	Names::Find<Node> ("2"),	Names::Find<Node> ("7"),	Names::Find<Node> ("10"),
				Names::Find<Node> ("11"),	Names::Find<Node> ("17"),	Names::Find<Node> ("20"),
				Names::Find<Node> ("22"),	Names::Find<Node> ("26"),	Names::Find<Node> ("28"),
				Names::Find<Node> ("29"),	Names::Find<Node> ("30"),	Names::Find<Node> ("32"),
				Names::Find<Node> ("34"),	Names::Find<Node> ("38"),	Names::Find<Node> ("40"),
				Names::Find<Node> ("41"),	Names::Find<Node> ("42"),	Names::Find<Node> ("44"),
				Names::Find<Node> ("45"),	Names::Find<Node> ("46"),	Names::Find<Node> ("47"),
				Names::Find<Node> ("48"),	Names::Find<Node> ("51"),	Names::Find<Node> ("52"),
				Names::Find<Node> ("54"),	Names::Find<Node> ("55"),	Names::Find<Node> ("56"),
				Names::Find<Node> ("57"),	Names::Find<Node> ("58"),	Names::Find<Node> ("59"),
				Names::Find<Node> ("60"),	Names::Find<Node> ("61"),	Names::Find<Node> ("62"),
				Names::Find<Node> ("63"),	Names::Find<Node> ("64"),	Names::Find<Node> ("65"),
				Names::Find<Node> ("66"),	Names::Find<Node> ("67"),	Names::Find<Node> ("68"),
				Names::Find<Node> ("69"),	Names::Find<Node> ("70"),	Names::Find<Node> ("71"),
				Names::Find<Node> ("72"),	Names::Find<Node> ("74"),	Names::Find<Node> ("76"),
				Names::Find<Node> ("77"),	Names::Find<Node> ("78"),	Names::Find<Node> ("79"),
				Names::Find<Node> ("80"),	Names::Find<Node> ("81"),	Names::Find<Node> ("82"),
				Names::Find<Node> ("83"),	Names::Find<Node> ("84"),	Names::Find<Node> ("87"),
				Names::Find<Node> ("88"),	Names::Find<Node> ("89"),	Names::Find<Node> ("90"),
				Names::Find<Node> ("91"),	Names::Find<Node> ("92"),	Names::Find<Node> ("93"),
				Names::Find<Node> ("94"),	Names::Find<Node> ("95"),	Names::Find<Node> ("96"),
				Names::Find<Node> ("97"),	Names::Find<Node> ("98"),	Names::Find<Node> ("99") };
  
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::McdStorage");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity);
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // for producers
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerMCD");
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
  consumerHelper.SetAttribute ("Randomize", StringValue ("exponential"));
  
  for(int i=0; i<=65; i++)
  {
    consumerHelper.Install (consumers[i]);
  }
  
  //Calculate the global routing
  ndnGlobalRoutingHelper.CalculateRoutes ();
  
  // tracers
  Simulator::Schedule (Seconds (60), ndn::L3AggregateTracer::InstallAll, aggregate_trace, Seconds (1.0));
  Simulator::Schedule (Seconds (60), ndn::L3RateTracer::InstallAll, rate_trace, Seconds (1.0));
  Simulator::Schedule (Seconds (60), ndn::CsTracer::InstallAll, cs_trace, Seconds (1));
  Simulator::Schedule (Seconds (60), ndn::AppDelayTracer::InstallAll, app_delays_trace);
  Simulator::Schedule (Seconds (60), L2RateTracer::InstallAll, drop_trace, Seconds (1.0));
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
  {
    Simulator::Schedule (Seconds (170), PeriodicStatsPrinter, *i);
  }
  
  Simulator::Stop (Seconds (180.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
