// ndn-congestion-topo-plugin.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{

  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("100Mbps"));
  Config::SetDefault ("ns3::ndn::RttEstimator::MaxRTO", StringValue ("1s"));
  Config::SetDefault ("ns3::ndn::ConsumerWindow::Window", StringValue ("1"));

  CommandLine cmd;
  cmd.Parse (argc, argv);

  AnnotatedTopologyReader topologyReader ("", 25);
  topologyReader.SetFileName ("topology/topo-6-node1.txt");
  topologyReader.Read ();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits",
                                   "Limit", "ns3::ndn::Limits::Window",
                                   "EnableNACKs", "true");
  ndnHelper.EnableLimits(true, Seconds(0.1));
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Lru",
                              //"MaxSize", "10000");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache"); 
  ndnHelper.InstallAll ();

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();
  
  //------------------------Set Nodes------------------------
  
  Ptr<Node> consumer1 = Names::Find<Node> ("client1");
  Ptr<Node> consumer2 = Names::Find<Node> ("client2");

  Ptr<Node> producer1 = Names::Find<Node> ("server1");
  Ptr<Node> producer2 = Names::Find<Node> ("server2");

  //------------------------Producer-------------------------
  
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));

  ndnGlobalRoutingHelper.AddOrigins ("/dst1", producer1);
  producerHelper.SetPrefix ("/dst1");
  producerHelper.Install (producer1);

  ndnGlobalRoutingHelper.AddOrigins ("/dst2", producer2);
  producerHelper.SetPrefix ("/dst2");
  producerHelper.Install (producer2);
  
  //------------------------Consumer-------------------------

  ndn::AppHelper consumerHelper1 ("ns3::ndn::ConsumerWindow");
  consumerHelper1.SetAttribute ("Size", StringValue ("20"));
  //consumerHelper.SetAttribute ("MaxSeq", StringValue ("1000"));

  // on the first consumer node install a Consumer application
  // that will express interests in /dst1 namespace
  consumerHelper1.SetPrefix ("/dst1");
  consumerHelper1.Install (consumer1); 

  ndn::AppHelper consumerHelper2 ("ns3::ndn::ConsumerCbr");
  consumerHelper2.SetAttribute ("MaxSeq", StringValue ("1000"));
  consumerHelper2.SetAttribute ("Frequency", StringValue ("1000"));
  //consumerHelper2.SetAttribute ("StatTime", StringValue ("10.0"));


   //consumerHelper.SetAttribute ("MaxSeq", StringValue ("1000"));

  // on the second consumer node install a Consumer application
  // that will express interests in /dst2 namespace
  
  consumerHelper2.SetPrefix ("/dst2");
  ApplicationContainer Flash_Crowd =   consumerHelper2.Install (consumer2);
  Flash_Crowd.Start(Seconds(8));



  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes ();

  Simulator::Stop (Seconds (200.0));
  
  L2RateTracer::InstallAll ("drop-trace-cs.txt", Seconds (0.5));
  ndn::L3RateTracer::InstallAll ("rate-trace-cs.txt", Seconds (0.5));
  ndn::AppDelayTracer::InstallAll ("app-delays-trace-cs.txt");
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
