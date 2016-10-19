/*
 * Popcachefrmidstat.cc
 *
 *  Created on: 2015年12月3日
 *      Author: zhi
 */

#include "pop-cache-fr-mid-stat.h"

#include "best-route.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-pst.h"
#include "ns3/ndn-pst-entry.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-content-store.h"
#include "ns3/ndn-face.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-benefit-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-distance-tag.h"

#include "ns3/random-variable.h"

//#include <iostream>
//#include <iomanip>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (Popcachefrmidstat);

LogComponent Popcachefrmidstat::g_log = LogComponent (Popcachefrmidstat::GetLogName ().c_str ());

std::string
Popcachefrmidstat::GetLogName ()
{
  return super::GetLogName ()+".Popcachefrmidstat";
}


TypeId
Popcachefrmidstat::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::Popcachefrmidstat")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <Popcachefrmidstat> ()
    ;
  return tid;
}

Popcachefrmidstat::Popcachefrmidstat ()
{
}

void
Popcachefrmidstat::NotifyNewAggregate ()
{
  if (m_pit == 0)
    {
      m_pit = GetObject<Pit> ();
    }
  if (m_pst == 0)
    {
      m_pst = GetObject<Pst> ();	//add the PST, zhi add
    }
  if (m_fib == 0)
    {
      m_fib = GetObject<Fib> ();
    }
  if (m_contentStore == 0)
    {
      m_contentStore = GetObject<ContentStore> ();
    }

  Object::NotifyNewAggregate ();
}

void
Popcachefrmidstat::DoDispose ()
{
  m_pit = 0;
  m_pst = 0;	//clean the PST, zhi add
  m_contentStore = 0;
  m_fib = 0;

  Object::DoDispose ();
}

void
Popcachefrmidstat::OnInterest (Ptr<Face> inFace,
                               Ptr<Interest> interest)
{
  if (interest->GetNack () > 0)
    {
      OnNack (inFace, interest);
      return;
    }  
  
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);

  // Update the Incoming Interest Packet Counts information, zhi add
  if(!UpdatePstInformation (interest))
  {
    // Failed to Update the PST Information, should do anything?
  }
  
  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*interest);
  bool similarInterest = true;
  if (pitEntry == 0)
    {
      similarInterest = false;
      pitEntry = m_pit->Create (interest);
      if (pitEntry != 0)
        {
          DidCreatePitEntry (inFace, interest, pitEntry);
        }
      else
        {
          FailedToCreatePitEntry (inFace, interest);
          return;
        }
    }
 
  bool isDuplicated = true;
  if (!pitEntry->IsNonceSeen (interest->GetNonce ()))
    {
      pitEntry->AddSeenNonce (interest->GetNonce ());
      isDuplicated = false;
    }

  if (isDuplicated)
    {
      DidReceiveDuplicateInterest (inFace, interest, pitEntry);
      return;
    }

  Ptr<Data> contentObject;
  contentObject = m_contentStore->Lookup (interest);
  if (contentObject != 0)
    { 
      // add the Tags to the data package which is from CS
      DistanceTag	Distance;
      BenefitTag	Benefit;
      FwHopCountTag	hopCountTag; 
      
      ConstCast<Packet> (contentObject->GetPayload ())->RemoveAllPacketTags ();
  
      if (interest->GetPayload ()->PeekPacketTag (hopCountTag))
        {
          Distance.Set(hopCountTag.Get());
          contentObject->GetPayload ()->AddPacketTag (hopCountTag);
          contentObject->GetPayload ()->AddPacketTag (Distance);
          contentObject->GetPayload ()->AddPacketTag (Benefit);
        }

      pitEntry->AddIncoming (inFace/*, Seconds (1.0)*/);

      // Do data plane performance measurements
      WillSatisfyPendingInterest (0, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (0, contentObject, pitEntry);
      return;
    }

  if (similarInterest && ShouldSuppressIncomingInterest (inFace, interest, pitEntry))
    {
      pitEntry->AddIncoming (inFace/*, interest->GetInterestLifetime ()*/);
      // update PIT entry lifetime
      pitEntry->UpdateLifetime (interest->GetInterestLifetime ());

      // Suppress this interest if we're still expecting data from some other face
      NS_LOG_DEBUG ("Suppress interests");
      m_dropInterests (interest, inFace);

      DidSuppressSimilarInterest (inFace, interest, pitEntry);
      return;
    }

  if (similarInterest)
    {
      DidForwardSimilarInterest (inFace, interest, pitEntry);
    }

  PropagateInterest (inFace, interest, pitEntry);
}

void
Popcachefrmidstat::OnData (Ptr<Face> inFace,
                           Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);

  /******** old method ********
  
  BenefitTag Benefit;

  Ptr<Packet> payloadCopy = data->GetPayload ()->Copy ();
  if (payloadCopy->RemovePacketTag (Benefit))
  {
    Benefit.Increment ();
    payloadCopy->AddPacketTag (Benefit);
  }         	         	
  data->SetPayload (payloadCopy);
  
  ******** old method ********/
  
  // caculate the cache probility
  BenefitTag Benefit;
  int b;
  data->GetPayload ()->PeekPacketTag (Benefit);
  Benefit.Increment ();
  b = Benefit.Get ();
  // Update the benefit value
  ConstCast<Packet> (data->GetPayload ())->ReplacePacketTag (Benefit);
  data->SetWire (0); // clean the Wiredata
  
  // Get the Populcarity of chunk base on statistic
  Ptr<pst::Entry> pstEntry = m_pst->Lookup (*data);
  int current,first;
  double p;
  if (pstEntry==0)
  {
    p=0;
  }
  else
  {
    current = pstEntry->GetIncoming();
    first   = m_pst->GetCurrentFirst();
    p=double(current)/double(first);
  }

  DistanceTag Distance;
  int d;
  data->GetPayload ()->PeekPacketTag (Distance);
  d = Distance.Get ();

  double prob=p*b/d;
  
  //generate the random number
  UniformVariable m_prob(0, 1); 
  double p_prob = m_prob.GetValue();	

  //std::cout<<std::setprecision(2)<<p<<"\t"<<b<<"\t"<<d<<"\t"<<prob<<std::endl;

  // Lookup PIT entry

  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () & Face::APPLICATION)))
        {
          // Optimistically add or update entry in the content store
          if(p_prob<=prob) cached = m_contentStore->Add (data);
        }
      else
        {
          // Drop data packet if PIT entry is not found
          // (unsolicited data packets should not "poison" content store)

          //drop dulicated or not requested data packet
          m_dropData (data, inFace);
        }

      DidReceiveUnsolicitedData (inFace, data, cached);
      return;
    }
  else
    {
      bool cached = false;

      // Probability
      if(p_prob<=prob) cached = m_contentStore->Add (data);

      DidReceiveSolicitedData (inFace, data, cached);
    }
    
  while (pitEntry != 0)
    {
      // Do data plane performance measurements
      WillSatisfyPendingInterest (inFace, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (inFace, data, pitEntry);

      // Lookup another PIT entry
      pitEntry = m_pit->Lookup (*data);
    }
}

bool
Popcachefrmidstat::UpdatePstInformation (Ptr<const Interest> interest)
{
  // add the statistic information to the PST, zhi add
  
  // Lookup operation will increase the access times of existed prefix by one
  Ptr<pst::Entry> pstEntry = m_pst->Lookup (*interest);
  
  if (pstEntry == 0)
  {
    pstEntry = m_pst->Create (interest);  // create the pst Entry, use prefix in interest
    if (pstEntry == 0)
    {
      NS_LOG_ERROR("Failed to create the pstEntry");
      return false;
    }
  }
  else
  {  
    if(pstEntry->GetIncoming() > m_pst->GetCurrentFirst())
    {
      m_pst->SetCurrentFirst(pstEntry->GetIncoming());
    }
  }
  return true;
}

} // namespace fw
} // namespace ndn
} // namespace ns3

