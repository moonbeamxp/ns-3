
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
  
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Read the Topology information
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topology/topo-test-window.txt");
  topologyReader.Read ();
  
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::PerOutFaceLimits", "Limit", "ns3::ndn::Limits::Window", "EnableNACKs", "true");
  ndnHelper.EnableLimits(true, Seconds(0.1));
  
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", CS_Capacity);
  ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache"); 
  
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // Producer
  Ptr<Node> producers[2] = {Names::Find<Node> ("Producer1"), Names::Find<Node> ("Producer2")};
  
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  
  producerHelper.SetPrefix ("/prefix1");
  ndnGlobalRoutingHelper.AddOrigins ("/prefix1", producers[0]);
  producerHelper.Install (producers[0]);
  
  producerHelper.SetPrefix ("/prefix2");
  ndnGlobalRoutingHelper.AddOrigins ("/prefix2", producers[1]);
  producerHelper.Install (producers[1]);
  
  // Consumer
  Ptr<Node> consumers[2] = {Names::Find<Node> ("Consumer1"), Names::Find<Node> ("Consumer2")};
                                                        
  //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  //consumerHelper.SetAttribute ("NumberOfContents", StringValue (Capacity));
  //consumerHelper.SetAttribute ("q", StringValue ("0"));
  //consumerHelper.SetAttribute ("s", StringValue (Alpha));
  
  ndn::AppHelper consumerHelper1 ("ns3::ndn::ConsumerWindow::ConsumerPeer");
  consumerHelper1.SetPrefix ("/prefix1");
  //consumerHelper1.SetAttribute("Size", StringValue("-1"));
  consumerHelper1.SetAttribute("MaxSeq", StringValue("20000"));
  consumerHelper1.Install (consumers[0]);
  
  ndn::AppHelper consumerHelper2 ("ns3::ndn::ConsumerCbr::ConsumerPeer");
  consumerHelper2.SetPrefix ("/prefix2");
  consumerHelper2.SetAttribute("Frequency", DoubleValue(2000));
  
  ApplicationContainer Flash_Crowd;
  
  Flash_Crowd = consumerHelper2.Install (consumers[1]);
  Flash_Crowd.Start(Seconds(4));
  Flash_Crowd.Stop(Seconds(6));
 
  Flash_Crowd = consumerHelper2.Install (consumers[1]);  
  Flash_Crowd.Start(Seconds(10));
  Flash_Crowd.Stop(Seconds(12));
  
  Flash_Crowd = consumerHelper2.Install (consumers[1]);  
  Flash_Crowd.Start(Seconds(14));
  Flash_Crowd.Stop(Seconds(20));

  //Calculate the global routing
  ndnGlobalRoutingHelper.CalculateRoutes ();
  
  /*
  ndn::L3AggregateTracer::InstallAll (aggregate_trace, Seconds (1.0));
  ndn::L3RateTracer::InstallAll (rate_trace, Seconds (1.0));
  ndn::CsTracer::InstallAll (cs_trace, Seconds (1));
  ndn::AppDelayTracer::InstallAll (app_delays_trace);
  L2RateTracer::InstallAll (drop_trace, Seconds (1.0));
  */
  
  Simulator::Stop (Seconds (30.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
