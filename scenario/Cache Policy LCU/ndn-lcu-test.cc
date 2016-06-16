/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */
// ndn-simple.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#define Strategy	"LCE"
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

/*void
PeriodicStatsPrinter (Ptr<Node> node, Time next)
{
  Ptr<ndn::Pit> pit = node->GetObject<ndn::Pit> ();

  std::cout << Simulator::Now ().ToDouble (Time::S) << "\t"
            << node->GetId () << "\t"
            << Names::FindName (node) << "\t"
            << pit->GetSize () << "\n";
  
  Simulator::Schedule (next, PeriodicStatsPrinter, node, next);
}*/

void
PeriodicStatsPrinter (Ptr<Node> node, Time next)
{
  Ptr<ndn::Pst> pst = node->GetObject<ndn::Pst> ();

  std::cout << Simulator::Now ().ToDouble (Time::S) << "\t"
            << node->GetId () << "\t"
            << Names::FindName (node) << "\t"
            << pst->GetSize () << "\n";
            
  //Ptr<ns3::ndn::pst::Entry> pstEntry=pst->Begin ();
  
  /*while(pstEntry)
    {
      std::cout << pstEntry->GetPrefix () << "\t\t" << pstEntry->GetAccessTimes () <<"\n";
      pstEntry = pst->Next (pstEntry);
    }
  */
  
  pst->Print(std::cout);
  //pst->UpdateThresholdValue ();
  
  Simulator::Schedule (next, PeriodicStatsPrinter, node, next);
}

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */

int 
main (int argc, char *argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1000Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("1ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("200"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  /*
  // Creating nodes
  NodeContainer nodes;
  nodes.Create (3);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (1));
  p2p.Install (nodes.Get (1), nodes.Get (2));
  //p2p.Install (nodes.Get (2), nodes.Get (3));
  //p2p.Install (nodes.Get (3), nodes.Get (4));
  //p2p.Install (nodes.Get (4), nodes.Get (5));
  //p2p.Install (nodes.Get (5), nodes.Get (6));
  //p2p.Install (nodes.Get (6), nodes.Get (7));
  */
  
  NodeContainer nodes;
  nodes.Create (8);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (2));
  p2p.Install (nodes.Get (1), nodes.Get (2));
  p2p.Install (nodes.Get (2), nodes.Get (4));
  p2p.Install (nodes.Get (3), nodes.Get (4));
  p2p.Install (nodes.Get (4), nodes.Get (5));
  p2p.Install (nodes.Get (5), nodes.Get (6));
  p2p.Install (nodes.Get (6), nodes.Get (7));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::LcdStorage");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::McdStorage");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::ConstantProb", "Prob", "0.5");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Probcache");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Popcache");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Popcachemid");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::PopcacheOnce");
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Popcachestat");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::LCUCache");

  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "10");
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Probability::Lru", "MaxSize", "10", "CacheProbability", "0.5");
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache");
  
  ndnHelper.SetPst("ns3::ndn::pst::Default","Threshold","10");
  
  ndnHelper.InstallAll ();
  
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // Consumer
  //ndn::AppHelper consumerHelper ("DumbRequester");
  //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerFixedVaule");
  //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfUniqueCachetag");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue ("10000"));
  consumerHelper.SetAttribute ("q", StringValue ("0"));
  consumerHelper.SetAttribute ("s", StringValue ("1"));
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue ("200")); // 10 interests a second
  consumerHelper.SetAttribute ("Randomize", StringValue ("exponential"));
  consumerHelper.Install (nodes.Get (0)); // first node
  consumerHelper.Install (nodes.Get (1)); // first node
  consumerHelper.Install (nodes.Get (3)); // first node

  // Producer
  //ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  //ndn::AppHelper producerHelper ("ns3::ndn::ProducerProbcache");
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerCachetag");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix ("/prefix");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix", nodes.Get (7));
  producerHelper.Install (nodes.Get (7)); // last node
  
  //Calculate the global routing
  ndnGlobalRoutingHelper.CalculateRoutes ();
  
  //Simulator::Schedule (Seconds (290), PeriodicStatsPrinter, nodes.Get (0), Seconds (3000));
  //Simulator::Schedule (Seconds (290), PeriodicStatsPrinter, nodes.Get (1), Seconds (3000));
  //Simulator::Schedule (Seconds (290), PeriodicStatsPrinter, nodes.Get (2), Seconds (3000));

  Simulator::Schedule (Seconds (290), ndn::L3AggregateTracer::InstallAll, aggregate_trace, Seconds (1.0));

  Simulator::Stop (Seconds (300.0));

  //ndn::AppDelayTracer::InstallAll ("result/app-delays-trace.txt");

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
