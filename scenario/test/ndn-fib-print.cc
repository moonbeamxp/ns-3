/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 * Author: Jiang Zhi
 */
// ndn-fib-print.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

int 
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1000Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("1ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("200"));

  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (3);

  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (1));
  p2p.Install (nodes.Get (1), nodes.Get (2));

  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");

  ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache");
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue ("100"));
  consumerHelper.SetAttribute ("q", StringValue ("0"));
  consumerHelper.SetAttribute ("s", StringValue ("1"));
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue ("1"));
  consumerHelper.Install (nodes.Get (0)); // first node

  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (nodes.Get (2));
  
  ndnGlobalRoutingHelper.AddOrigins ("/prefix", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/a", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/b", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/c", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/a/a", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/a/b", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/a/c", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/b/a", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/b/b", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/c/d", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/c/a", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/c/b", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefixc/c", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/a/a/a", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/a/a/b", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/c/a/a/a", nodes.Get (2));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix/c/a/a/b", nodes.Get (2));

  ndnGlobalRoutingHelper.CalculateRoutes ();
  
  Ptr<ndn::Fib> fib = nodes.Get (0)->GetObject<ndn::Fib> ();
  fib->Print (std::cout);
  
  Simulator::Stop (Seconds (1.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
