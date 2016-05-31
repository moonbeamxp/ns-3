
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;


int 
main (int argc, char *argv[])
{

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  AnnotatedTopologyReader topologyReader ("", 1);
  topologyReader.SetFileName ("topology/topo1.txt");
  topologyReader.Read ();


  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  // ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::CustomStrategy");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "10");
  ndnHelper.InstallAll ();

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications
  Ptr<Node> consumers[4] = { Names::Find<Node> ("Customer1"), Names::Find<Node> ("Customer2"), Names::Find<Node> ("Customer3"), Names::Find<Node> ("Customer4") };
  Ptr<Node> producers[3] = { Names::Find<Node> ("Producer1"), Names::Find<Node> ("CDN1"), Names::Find<Node> ("CDN2") };

  // Consumer
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  // Consumer will request /prefix/0, /prefix/1, ...

  ApplicationContainer consumer;

  consumerHelper.SetPrefix ("/prefix1");
  consumerHelper.SetAttribute ("Frequency", StringValue ("1")); // 10 interests a second

  consumer = consumerHelper.Install (consumers[0]);
  consumer.Start (Seconds (3));
  consumer.Stop  (Seconds (10));

  consumer = consumerHelper.Install (consumers[2]);
  consumer.Start (Seconds (11));
  consumer.Stop  (Seconds (20));

  consumerHelper.SetPrefix ("/prefix2");
  consumerHelper.SetAttribute ("Frequency", StringValue ("1")); // 10 interests a second

  consumer = consumerHelper.Install (consumers[1]);
  consumer.Start (Seconds (5));
  consumer.Stop  (Seconds (10));

  consumer = consumerHelper.Install (consumers[3]);
  consumer.Start (Seconds (7));
  consumer.Stop  (Seconds (10));

  consumer = consumerHelper.Install (consumers[2]);
  consumer.Start (Seconds (15));
  consumer.Stop  (Seconds (20));

  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix

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

// Set Producer
  producerHelper.SetPrefix ("/prefix1");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (producers[1]); 

  producerHelper.SetPrefix ("/prefix2");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (producers[2]); 

  producerHelper.SetPrefix ("/prefix1");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (producers[0]);

  producerHelper.SetPrefix ("/prefix2");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (producers[0]); 

  ndn::GlobalRoutingHelper::CalculateRoutes ();
  ndn::L3AggregateTracer::InstallAll ("result/aggregate-trace.txt", Seconds (1.0));
  ndn::CsTracer::InstallAll ("result/cs-trace.txt", Seconds (1));
  ndn::AppDelayTracer::InstallAll ("./result/app-delays-trace.txt");

  ndn::L3AggregateTracer::Install(Names::Find<Node> ("Router1"),"result/aggregate-trace-of-router1.txt", Seconds (1.0));
  ndn::L3RateTracer::Install(Names::Find<Node> ("Router1"), "result/rate-trace-of-router1.txt", Seconds (1.0));
  L2RateTracer::InstallAll ("result/drop-trace.txt", Seconds (0.5));

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
