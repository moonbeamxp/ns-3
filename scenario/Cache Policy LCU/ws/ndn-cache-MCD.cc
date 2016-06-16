
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
  topologyReader.SetFileName ("topology/topo-ws-34.3.0.5-p2p.txt");
  topologyReader.Read ();
  
  // Define Producer
  Ptr<Node> producers[1] = {	Names::Find<Node> ("0")};
  
/*  // Define Router
  Ptr<Node> routers[33] = {	Names::Find<Node> ("1"),	Names::Find<Node> ("2"),	Names::Find<Node> ("3"),
  				Names::Find<Node> ("4"), 	Names::Find<Node> ("5"), 	Names::Find<Node> ("6"),
				Names::Find<Node> ("7"), 	Names::Find<Node> ("8"),   	Names::Find<Node> ("9"),
				Names::Find<Node> ("10"), 	Names::Find<Node> ("11"), 	Names::Find<Node> ("12"),
				Names::Find<Node> ("13"), 	Names::Find<Node> ("14"), 	Names::Find<Node> ("15"),
				Names::Find<Node> ("16"), 	Names::Find<Node> ("17"), 	Names::Find<Node> ("18"),
				Names::Find<Node> ("19"), 	Names::Find<Node> ("20"), 	Names::Find<Node> ("21"),
				Names::Find<Node> ("22"), 	Names::Find<Node> ("23"),  	Names::Find<Node> ("24"),
				Names::Find<Node> ("25"), 	Names::Find<Node> ("26"), 	Names::Find<Node> ("27"),
				Names::Find<Node> ("28"), 	Names::Find<Node> ("29"), 	Names::Find<Node> ("30"),
				Names::Find<Node> ("31"), 	Names::Find<Node> ("32"), 	Names::Find<Node> ("33") };
*/
  // Define Consumer
  Ptr<Node> consumers[66] = {	Names::Find<Node> ("PEER01"),	Names::Find<Node> ("PEER02"),	Names::Find<Node> ("PEER03"),
  				Names::Find<Node> ("PEER04"),	Names::Find<Node> ("PEER05"),	Names::Find<Node> ("PEER06"),
  				Names::Find<Node> ("PEER07"),	Names::Find<Node> ("PEER08"),	Names::Find<Node> ("PEER09"),
  				Names::Find<Node> ("PEER10"),	Names::Find<Node> ("PEER11"),	Names::Find<Node> ("PEER12"),
  				Names::Find<Node> ("PEER13"),	Names::Find<Node> ("PEER14"),	Names::Find<Node> ("PEER15"),
  				Names::Find<Node> ("PEER16"),	Names::Find<Node> ("PEER17"),	Names::Find<Node> ("PEER18"),
  				Names::Find<Node> ("PEER19"),	Names::Find<Node> ("PEER20"),	Names::Find<Node> ("PEER21"),
  				Names::Find<Node> ("PEER22"),	Names::Find<Node> ("PEER23"),	Names::Find<Node> ("PEER24"),
  				Names::Find<Node> ("PEER25"),	Names::Find<Node> ("PEER26"),	Names::Find<Node> ("PEER27"),
  				Names::Find<Node> ("PEER28"),	Names::Find<Node> ("PEER29"),	Names::Find<Node> ("PEER30"),
  				Names::Find<Node> ("PEER31"),	Names::Find<Node> ("PEER32"),	Names::Find<Node> ("PEER33"),
  				Names::Find<Node> ("PEER34"),	Names::Find<Node> ("PEER35"),	Names::Find<Node> ("PEER36"),
  				Names::Find<Node> ("PEER37"),	Names::Find<Node> ("PEER38"),	Names::Find<Node> ("PEER39"),
  				Names::Find<Node> ("PEER40"),	Names::Find<Node> ("PEER41"),	Names::Find<Node> ("PEER42"),
  				Names::Find<Node> ("PEER43"),	Names::Find<Node> ("PEER44"),	Names::Find<Node> ("PEER45"),
  				Names::Find<Node> ("PEER46"),	Names::Find<Node> ("PEER47"),	Names::Find<Node> ("PEER48"),
  				Names::Find<Node> ("PEER49"),	Names::Find<Node> ("PEER50"),	Names::Find<Node> ("PEER51"),
  				Names::Find<Node> ("PEER52"),	Names::Find<Node> ("PEER53"),	Names::Find<Node> ("PEER54"),
  				Names::Find<Node> ("PEER55"),	Names::Find<Node> ("PEER56"),	Names::Find<Node> ("PEER57"),
  				Names::Find<Node> ("PEER58"),	Names::Find<Node> ("PEER59"),	Names::Find<Node> ("PEER60"),
  				Names::Find<Node> ("PEER61"),	Names::Find<Node> ("PEER62"),	Names::Find<Node> ("PEER63"),
  				Names::Find<Node> ("PEER64"),	Names::Find<Node> ("PEER65"),	Names::Find<Node> ("PEER66") }; 
  
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
