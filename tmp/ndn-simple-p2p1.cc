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

using namespace ns3;

void
PeriodicStatsPrinter (Ptr<Node> node, Time next)
{
  Ptr<ndn::Pit> pit = node->GetObject<ndn::Pit> ();

  std::cout << Simulator::Now ().ToDouble (Time::S) << "\t"
            << node->GetId () << "\t"
            << Names::FindName (node) << "\t"
            << pit->GetSize () << "\n";
  
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
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("2000"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create (8);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (1));
  p2p.Install (nodes.Get (1), nodes.Get (2));
  p2p.Install (nodes.Get (2), nodes.Get (3));
  p2p.Install (nodes.Get (2), nodes.Get (4));
  p2p.Install (nodes.Get (3), nodes.Get (5));
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
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::P2PRoutePeer");

  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "10000");
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Probability::Lru", "MaxSize", "10", "CacheProbability", "0.5");
  //ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache");
  //ndnHelper.InstallAll ();
  ndnHelper.Install (nodes.Get (0));
  
  ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache");
  ndnHelper.Install (nodes.Get (4));
  
  ndnHelper.SetContentStore ("ns3::ndn::cs::Nocache");
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::P2PRouteIndex");
  ndnHelper.Install (nodes.Get (1));
  ndnHelper.Install (nodes.Get (2));
  ndnHelper.Install (nodes.Get (3));
  ndnHelper.Install (nodes.Get (5));
  ndnHelper.Install (nodes.Get (6));
  ndnHelper.Install (nodes.Get (7));

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();
  
  // Installing applications

  // Consumer
  //ndn::AppHelper consumerHelper ("DumbRequester");
  //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr::ConsumerPeer");
  //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerFixedVaule");
  //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot::ConsumerPeer");
  consumerHelper.SetAttribute ("NumberOfContents", StringValue ("10"));
  consumerHelper.SetAttribute ("q", StringValue ("0"));
  consumerHelper.SetAttribute ("s", StringValue ("1"));
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue ("10")); // 10 interests a second
  consumerHelper.Install (nodes.Get (0)); // first node
  
  ndn::AppHelper consumerHelper1 ("ns3::ndn::ConsumerZipfMandelbrot");
  consumerHelper1.SetAttribute ("NumberOfContents", StringValue ("1"));
  consumerHelper1.SetAttribute ("q", StringValue ("0"));
  consumerHelper1.SetAttribute ("s", StringValue ("1"));
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper1.SetPrefix ("/prefix");
  consumerHelper1.SetAttribute ("Frequency", StringValue ("1")); // 10 interests a second
  //consumerHelper1.Install (nodes.Get (4)); // first node

  ApplicationContainer Flash_Crowd;
  
  Flash_Crowd = consumerHelper1.Install (nodes.Get (4));
  Flash_Crowd.Start(Seconds(20));
  //Flash_Crowd.Stop(Seconds(20));
  
  // Producer
  //ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  //ndn::AppHelper producerHelper ("ns3::ndn::ProducerProbcache");
  ndn::AppHelper producerHelper ("ns3::ndn::ProducerPopcache");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix ("/prefix");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  ndnGlobalRoutingHelper.AddOrigins ("/prefix", nodes.Get (7));
  producerHelper.Install (nodes.Get (7)); // last node
 
  Simulator::Schedule (Seconds (15.0), ndn::LinkControlHelper::FailLink, nodes.Get (0), nodes.Get (1));
  //Simulator::Schedule (Seconds (55.0), ndn::LinkControlHelper::UpLink,   nodes.Get (0), nodes.Get (1));
  //Simulator::Schedule (Seconds (55.0), ndn::LinkControlHelper::UpLink,   nodes.Get (1), nodes.Get (2));
  
  Simulator::Schedule (Seconds (10), PeriodicStatsPrinter, nodes.Get (2), Seconds (0.8));
  //Calculate the global routing
  ndnGlobalRoutingHelper.CalculateRoutes ();
  
  Simulator::Stop (Seconds (60.0));

  //ndn::AppDelayTracer::InstallAll ("result/app-delays-trace.txt");

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
