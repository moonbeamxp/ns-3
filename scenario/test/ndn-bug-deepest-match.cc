/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * a bug of deepest-prefix-match
 * Author: Jiang Zhi
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/names.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

using namespace ns3;

int 
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("10Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("1ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("100"));

  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (1);

  ndn::StackHelper ndnHelper;
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "10");
  ndnHelper.InstallAll ();
   
  // Show the bug here 
  
  std::string prefix="/1/2";
  Ptr<ndn::Name> name1 = Create<ndn::Name> (prefix);
  Ptr<ndn::Data> data1 = Create<ndn::Data> (Create<Packet> (1024));
  data1->SetName (name1);
  
  prefix="/1/2/3/4/5/6";
  Ptr<ndn::Name> name2 = Create<ndn::Name> (prefix);
  Ptr<ndn::Data> data2 = Create<ndn::Data> (Create<Packet> (1024));
  data2->SetName (name2);
  
  prefix="/1/2/3/4";
  Ptr<ndn::Name> name3 = Create<ndn::Name> (prefix);
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  interest->SetName (name3);
  
  Ptr<ndn::ContentStore> cs = nodes.Get (0)->GetObject<ndn::ContentStore> ();
  cs->Add (data1);
  cs->Add (data2);
  std::cout<<"Content Store List"<<std::endl;
  cs->Print(std::cout);
  std::cout<<"Search for "<<interest->GetName()<<", Match "<<(cs->Lookup (interest))->GetName()<<std::endl;
  
  Simulator::Stop (Seconds (1.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
