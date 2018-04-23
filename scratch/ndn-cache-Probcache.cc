
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define Strategy	"Probcache"
#define CS_Capacity	"40"
#define Capacity	"10000"
#define Alpha		"1.0"
#define Rate		"350"

#define suffix			Strategy"-"Capacity"-"CS_Capacity"-"Rate"-"Alpha".txt"
#define aggregate_trace		"result/aggregate-trace-"suffix
#define rate_trace		"result/rate-trace-"suffix
#define cs_trace		"result/cs-trace-"suffix
#define app_delays_trace	"result/app-delays-trace-"suffix
#define drop_trace		"result/drop-trace-"suffix
#define cs_index		"result/cs-index-"suffix

using namespace ns3;

int 
main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Read the Topology information
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topology/topo-ba-150-1-hybrid.txt");
  topologyReader.Read ();
  
  Ptr<Node> producers[1] = { 	Names::Find<Node> ("0")};

  Ptr<Node> consumers[100] = {	Names::Find<Node> ("4"),	Names::Find<Node> ("6"),	Names::Find<Node> ("8"),
				Names::Find<Node> ("17"),	Names::Find<Node> ("18"),	Names::Find<Node> ("28"),
				Names::Find<Node> ("30"),	Names::Find<Node> ("31"),	Names::Find<Node> ("38"),
				Names::Find<Node> ("40"),	Names::Find<Node> ("41"),	Names::Find<Node> ("42"),
				Names::Find<Node> ("43"),	Names::Find<Node> ("44"),	Names::Find<Node> ("45"),
				Names::Find<Node> ("46"),	Names::Find<Node> ("49"),	Names::Find<Node> ("50"),
				Names::Find<Node> ("54"),	Names::Find<Node> ("55"),	Names::Find<Node> ("56"),
				Names::Find<Node> ("62"),	Names::Find<Node> ("63"),	Names::Find<Node> ("64"),
				Names::Find<Node> ("65"),	Names::Find<Node> ("66"),	Names::Find<Node> ("67"),
				Names::Find<Node> ("68"),	Names::Find<Node> ("69"),	Names::Find<Node> ("70"),
				Names::Find<Node> ("71"),	Names::Find<Node> ("72"),	Names::Find<Node> ("75"),
				Names::Find<Node> ("76"),	Names::Find<Node> ("77"),	Names::Find<Node> ("79"),
				Names::Find<Node> ("80"),	Names::Find<Node> ("81"),	Names::Find<Node> ("84"),
				Names::Find<Node> ("85"),	Names::Find<Node> ("86"),	Names::Find<Node> ("87"),
				Names::Find<Node> ("88"),	Names::Find<Node> ("89"),	Names::Find<Node> ("91"),
				Names::Find<Node> ("93"),	Names::Find<Node> ("94"),	Names::Find<Node> ("95"),
				Names::Find<Node> ("96"),	Names::Find<Node> ("97"),	Names::Find<Node> ("98"),
				Names::Find<Node> ("99"),	Names::Find<Node> ("100"),	Names::Find<Node> ("101"),
				Names::Find<Node> ("102"),	Names::Find<Node> ("103"),	Names::Find<Node> ("104"),
				Names::Find<Node> ("105"),	Names::Find<Node> ("106"),	Names::Find<Node> ("107"),
				Names::Find<Node> ("108"),	Names::Find<Node> ("109"),	Names::Find<Node> ("110"),
				Names::Find<Node> ("111"),	Names::Find<Node> ("113"),	Names::Find<Node> ("114"),
				Names::Find<Node> ("115"),	Names::Find<Node> ("116"),	Names::Find<Node> ("117"),
				Names::Find<Node> ("118"),	Names::Find<Node> ("119"),	Names::Find<Node> ("120"),
				Names::Find<Node> ("121"),	Names::Find<Node> ("122"),	Names::Find<Node> ("123"),
				Names::Find<Node> ("124"),	Names::Find<Node> ("125"),	Names::Find<Node> ("126"),
				Names::Find<Node> ("127"),	Names::Find<Node> ("128"),	Names::Find<Node> ("130"),
				Names::Find<Node> ("131"),	Names::Find<Node> ("132"),	Names::Find<Node> ("133"),
				Names::Find<Node> ("134"),	Names::Find<Node> ("135"),	Names::Find<Node> ("136"),
				Names::Find<Node> ("137"),	Names::Find<Node> ("138"),	Names::Find<Node> ("139"),
				Names::Find<Node> ("140"),	Names::Find<Node> ("141"),	Names::Find<Node> ("142"),
				Names::Find<Node> ("143"),	Names::Find<Node> ("144"),	Names::Find<Node> ("145"),
				Names::Find<Node> ("146"),	Names::Find<Node> ("147"),	Names::Find<Node> ("148"),
				Names::Find<Node> ("149")};
				
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Probcache");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity);
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // for producers
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerProbcache");
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
  
  for(int i=0; i<=99; i++)
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
  
  Simulator::Stop (Seconds (120.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
