
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#define Rank1	"693"
#define Rank2	"92"
#define Rank3	"53"
#define Rank4	"38"
#define Rank5	"30"
#define Rank6	"24"
#define Rank7	"21"
#define Rank8	"18"
#define Rank9	"16"
#define Rank10	"14"

using namespace ns3;

int 
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  AnnotatedTopologyReader topologyReader ("", 1);
  topologyReader.SetFileName ("topology/nocdn.txt");
  topologyReader.Read ();

  ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "10");
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache");
  ndnHelper.InstallAll ();

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications
  Ptr<Node> consumers[4] = { Names::Find<Node> ("Customer1"), Names::Find<Node> ("Customer2"), Names::Find<Node> ("Customer3"), Names::Find<Node> ("Customer4") };
  Ptr<Node> producers[1] = { Names::Find<Node> ("Producer1")};

  // Consumer
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue ("100"));
  ApplicationContainer consumer;

  consumerHelper.SetPrefix ("/prefix1");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank1));
  consumer = consumerHelper.Install (consumers[0]);
  consumer = consumerHelper.Install (consumers[1]);
  consumer = consumerHelper.Install (consumers[3]);
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank2));
  consumer = consumerHelper.Install (consumers[2]);

  consumerHelper.SetPrefix ("/prefix2");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank2));
  consumer = consumerHelper.Install (consumers[0]);
  consumer = consumerHelper.Install (consumers[1]);
  consumer = consumerHelper.Install (consumers[3]);
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank1));
  consumer = consumerHelper.Install (consumers[2]);

  consumerHelper.SetPrefix ("/prefix3");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank3));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  consumerHelper.SetPrefix ("/prefix4");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank4));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  consumerHelper.SetPrefix ("/prefix5");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank5));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  consumerHelper.SetPrefix ("/prefix6");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank6));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  consumerHelper.SetPrefix ("/prefix7");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank7));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  consumerHelper.SetPrefix ("/prefix8");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank8));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  consumerHelper.SetPrefix ("/prefix9");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank9));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  consumerHelper.SetPrefix ("/prefix10");
  consumerHelper.SetAttribute ("Frequency", StringValue (Rank10));
  for(int i=0;i<4;i++)
  consumer = consumerHelper.Install (consumers[i]);

  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");

  // Set Global Routing Information

  std::string prefix;

  prefix = "/prefix1";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix2";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix3";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix4";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix5";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix6";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix7";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix8";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix9";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);
  prefix = "/prefix10";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producers[0]);

// Set Producer

  producerHelper.SetPrefix ("/prefix1");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (producers[0]);

  producerHelper.SetPrefix ("/prefix2");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix3");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix4");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix5");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix6");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix7");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix8");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix9");
  producerHelper.Install (producers[0]);
  producerHelper.SetPrefix ("/prefix10");
  producerHelper.Install (producers[0]);

  ndn::GlobalRoutingHelper::CalculateRoutes ();

  ndn::L3AggregateTracer::InstallAll ("result/aggregate-trace-10.txt", Seconds (1.0));
  ndn::L3RateTracer::InstallAll ("result/rate-trace-10.txt", Seconds (1.0));
  ndn::CsTracer::InstallAll ("result/cs-trace-10.txt", Seconds (1));
  ndn::AppDelayTracer::InstallAll ("result/app-delays-trace-10.txt");
  L2RateTracer::InstallAll ("result/drop-trace-10.txt", Seconds (1.0));

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
