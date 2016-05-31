
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define CS_Capacity	"40"


#define Capacity1	"200"
#define Capacity2	"200"
#define Capacity3	"9600"

#define Alpha		"0.9"
#define Rate1		"482"
#define Rate2		"78"
#define Rate3		"440"

#define suffix			Capacity1"-"CS_Capacity"-"Alpha".txt"
#define aggregate_trace		"result/aggregate-trace-"suffix
#define rate_trace		"result/rate-trace-"suffix
#define cs_trace		"result/cs-trace-"suffix
#define app_delays_trace	"result/app-delays-trace-"suffix
#define drop_trace		"result/drop-trace-"suffix

using namespace ns3;

int 
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  AnnotatedTopologyReader topologyReader ("", 1);
  topologyReader.SetFileName ("topology/cdn.txt");
  topologyReader.Read ();

  ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity);
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache");
  ndnHelper.InstallAll ();

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications
  Ptr<Node> consumers[4] = { Names::Find<Node> ("Customer1"), Names::Find<Node> ("Customer2"), Names::Find<Node> ("Customer3"), Names::Find<Node> ("Customer4") };
  Ptr<Node> producers[3] = { Names::Find<Node> ("Producer1"), Names::Find<Node> ("CDN1"), Names::Find<Node> ("CDN2") };

  // Consumer
  ApplicationContainer consumer;
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");

  consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity1));
  consumerHelper.SetPrefix ("/prefix1");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate1));
  consumer = consumerHelper.Install (consumers[0]);
  consumer = consumerHelper.Install (consumers[1]);
  consumer = consumerHelper.Install (consumers[3]);
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate2));
  consumer = consumerHelper.Install (consumers[2]);

  consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity2));
  consumerHelper.SetPrefix ("/prefix2");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate2));
  consumer = consumerHelper.Install (consumers[0]);
  consumer = consumerHelper.Install (consumers[1]);
  consumer = consumerHelper.Install (consumers[3]);
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate1));
  consumer = consumerHelper.Install (consumers[2]);

  consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity3));
  consumerHelper.SetPrefix ("/prefix3");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rate3));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);


  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");

  // Set Global Routing Information

  std::string prefix;

  prefix = "/prefix1";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[1]);

  prefix = "/prefix2";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[2]);

  prefix = "/prefix1";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix2";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix3";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);

// Set Producer

  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));

  producerHelper.SetPrefix ("/prefix1");
  producerHelper.Install (producers[1]); 

  producerHelper.SetPrefix ("/prefix2");
  producerHelper.Install (producers[2]); 

  producerHelper.SetPrefix ("/prefix1");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix2");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix3");
  producerHelper.Install (producers[0]);

  ndn::GlobalRoutingHelper::CalculateRoutes ();

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
