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
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("20"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create (10);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (1));
  p2p.Install (nodes.Get (1), nodes.Get (2));
  p2p.Install (nodes.Get (2), nodes.Get (3));
  p2p.Install (nodes.Get (2), nodes.Get (4));
  p2p.Install (nodes.Get (3), nodes.Get (5));
  p2p.Install (nodes.Get (3), nodes.Get (6));
  p2p.Install (nodes.Get (3), nodes.Get (7));
  p2p.Install (nodes.Get (0), nodes.Get (8));
  p2p.Install (nodes.Get (2), nodes.Get (9));


  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  // ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::CustomStrategy");

  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "100");

  ndnHelper.InstallAll ();

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
  // Consumer will request /prefix/0, /prefix/1, ...
  ApplicationContainer consumer;
  consumerHelper.SetPrefix ("/prefix1");
  consumerHelper.SetAttribute ("Frequency", StringValue ("1")); // 10 interests a second

  consumer = consumerHelper.Install (nodes.Get (6));
  consumer.Start (Seconds (3));
  consumer.Stop  (Seconds (10));

  consumer = consumerHelper.Install (nodes.Get (9));
  consumer.Start (Seconds (11));
  consumer.Stop  (Seconds (20));

  consumerHelper.SetPrefix ("/prefix2");
  consumerHelper.SetAttribute ("Frequency", StringValue ("1")); // 10 interests a second
  consumer = consumerHelper.Install (nodes.Get (7));
  consumer.Start (Seconds (5));
  consumer.Stop  (Seconds (10));

  consumer = consumerHelper.Install (nodes.Get (8));
  consumer.Start (Seconds (7));
  consumer.Stop  (Seconds (10));

  consumer = consumerHelper.Install (nodes.Get (9));
  consumer.Start (Seconds (15));
  consumer.Stop  (Seconds (20));

  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix

// Set Global Routing Information
  Ptr<Node> producer;
  std::string prefix;
  producer = nodes.Get(4);
  prefix = "/prefix1";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producer);
  producer = nodes.Get(5);
  prefix = "/prefix2";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producer);
  producer = nodes.Get(0);
  prefix = "/prefix1";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producer);
  producer = nodes.Get(0);
  prefix = "/prefix2";
  ndnGlobalRoutingHelper.AddOrigins (prefix, producer);

  ndn::GlobalRoutingHelper::CalculateRoutes ();

// Set Producer
  producerHelper.SetPrefix ("/prefix1");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (nodes.Get (4)); 

  producerHelper.SetPrefix ("/prefix2");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (nodes.Get (5)); 

  producerHelper.SetPrefix ("/prefix1");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (nodes.Get (0));

  producerHelper.SetPrefix ("/prefix2");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (nodes.Get (0)); 

  Simulator::Stop (Seconds (20.0));

  ndn::L3AggregateTracer::InstallAll ("result/aggregate-trace.txt", Seconds (1.0));
  ndn::CsTracer::InstallAll ("result/cs-trace.txt", Seconds (1));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
