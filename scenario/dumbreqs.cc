// ndn-simple.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

int 
main (int argc, char *argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("20"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create (4);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (1));
  p2p.Install (nodes.Get (1), nodes.Get (2));
  p2p.Install (nodes.Get (2), nodes.Get (3));



  // Install CCNx stack on all nodes
  ndn::StackHelper ccnxHelper;
  ccnxHelper.SetDefaultRoutes (true);
  ccnxHelper.SetContentStore ("ns3::ndn::cs::Freshness::Lru",
                              "MaxSize", "2"); // allow just 2 entries to be cached
  ccnxHelper.Install(nodes.Get(1));
  ccnxHelper.Install(nodes.Get(2));
  ccnxHelper.Install(nodes.Get(3));
  ccnxHelper.SetContentStore ("ns3::ndn::cs::Nocache"); // allow just 2 entries to be cached
  ccnxHelper.Install(nodes.Get(0));

  // Consumer
  ndn::AppHelper consumerHelper ("DumbRequester");

  // /*
  //   1) at time 1 second requests Data from a producer that does not specify freshness
  //   2) at time 10 seconds requests the same Data packet as client 1

  //   3) at time 2 seconds requests Data from a producer that specifies freshness set to 2 seconds
  //   4) at time 12 seconds requests the same Data packet as client 3

  //   Expectation:
  //   Interests from 1, 3 and 4 will reach producers
  //   Interset from 2 will be served from cache
  //  */

  ApplicationContainer apps;
  
  consumerHelper.SetPrefix ("/no-freshness");
  apps = consumerHelper.Install (nodes.Get (0));
  apps.Start (Seconds (0.1));
  apps.Stop  (Seconds (10.0));

  consumerHelper.SetPrefix ("/with-freshness");
  apps = consumerHelper.Install (nodes.Get (0));
  apps.Start (Seconds (10.1));
  apps.Stop  (Seconds (20.0));
  
  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));

  producerHelper.SetAttribute ("Freshness", TimeValue (Seconds (0))); // unlimited freshness
  producerHelper.SetPrefix ("/no-freshness");
  producerHelper.Install (nodes.Get (3)); // last node

  producerHelper.SetAttribute ("Freshness", TimeValue (Seconds (1.0))); // freshness 2 seconds (!!! freshness granularity is 1 seconds !!!)
  producerHelper.SetPrefix ("/with-freshness");
  producerHelper.Install (nodes.Get (3)); // last node
                               
  Simulator::Stop (Seconds (30.0));
  ndn::L3AggregateTracer::InstallAll ("result/aggregate-trace.txt", Seconds (1.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
