#include <iostream>
#include <time.h>

#include "ns3/ndn-data.h"
#include "ns3/packet.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-benefit-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-popularity-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-distance-tag.h"

#include "ns3/random-variable.h"

using namespace ns3::ndn;

bool process(ns3::Ptr<Data> data)
{
  bool cached=0;
  
  // generate the random number
  ns3::UniformVariable m_prob(0, 1); 
  double p_prob = m_prob.GetValue();
  
  BenefitTag Benefit;
  int b;
  data->GetPayload ()->PeekPacketTag (Benefit);
  Benefit.Increment ();
  b = Benefit.Get ();
  Benefit.Set(p_prob*5);
  // Update the benefit value
  ns3::ConstCast<ns3::Packet> (data->GetPayload ())->ReplacePacketTag (Benefit);
  data->SetWire (0); // clean the Wiredata
  
  PopularityTag Popularity;
  double p;
  data->GetPayload ()->PeekPacketTag (Popularity);
  p = Popularity.Get ();

  DistanceTag Distance;
  int d;
  data->GetPayload ()->PeekPacketTag (Distance);
  d = Distance.Get ();  

  double prob=p*b/d;
  
  //std::cout<<p<<"\t"<<b<<"\t"<<d<<"\t"<<prob<<std::endl;

  if(p_prob<=prob)
  cached=1;
  
  return cached;
}

int 
main (int argc, char *argv[])
{
  clock_t start, finish;
  double totalTime;
  int cached=0;
  int times=10000000;
  ns3::Ptr<Data> data = ns3::Create<Data> (ns3::Create<ns3::Packet> (1024));
  //Ptr<Name> dataName = Create<Name> ("prefix");
  //dataName->append (m_postfix);
  //data->SetName (dataName);
  //data->SetFreshness (m_freshness);
  //data->SetTimestamp (Simulator::Now());
  //data->SetSignature (m_signature);
  //if (m_keyLocator.size () > 0)
  //  {
  //    data->SetKeyLocator (Create<Name> (m_keyLocator));
  //  }
  
  // Echo back FwHopCountTag if exists
  FwHopCountTag hopCountTag;
  data->GetPayload ()->AddPacketTag (hopCountTag);
    
  //add the Distance Tag to the data package
  DistanceTag Distance;
  Distance.Set(5);
  data->GetPayload ()->AddPacketTag (Distance);
  
  //add the Benefit Tag to the data package
  BenefitTag Benefit;
  Benefit.Set(3);
  data->GetPayload ()->AddPacketTag (Benefit);
  
  //add the Popularity Tag to the data package
  PopularityTag Popularity;
  Popularity.Set(0.5);
  data->GetPayload ()->AddPacketTag (Popularity);
  
  start = clock();
//----------------------------
  for(int i=0;i<=times;i++)
  if(process(data))	cached++;
//----------------------------
  finish = clock();
    
  totalTime=(double)(finish-start)/CLOCKS_PER_SEC;
  
  std::cout<<"cached "<<cached<<" times\n";
  std::cout<<"costs of "<<times<<" times is "<<totalTime*1000<<" ms"<<std::endl;
  std::cout<<"average cost is "<<totalTime*1000*1000/times<<" us"<<std::endl;
    
  return 0;
}

