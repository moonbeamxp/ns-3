/*
 * pop-cache.cc
 *
 *  Created on: 2014年6月30日
 *      Author: zhi
 */

#include "pop-cache.h"

#include "best-route.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
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
#include "ns3/ndnSIM/utils/ndn-fw-popularity-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-distance-tag.h"

#include "ns3/random-variable.h"

//#include <iostream>
//#include <iomanip>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (Popcache);

LogComponent Popcache::g_log = LogComponent (Popcache::GetLogName ().c_str ());

std::string
Popcache::GetLogName ()
{
  return super::GetLogName ()+".Popcache";
}


TypeId
Popcache::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::Popcache")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <Popcache> ()
    ;
  return tid;
}

Popcache::Popcache ()
{
}

void
Popcache::OnInterest (Ptr<Face> inFace,
                                Ptr<Interest> interest)
{
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);

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
      FwHopCountTag hopCountTag;
      if (interest->GetPayload ()->PeekPacketTag (hopCountTag))
        {
          contentObject->GetPayload ()->AddPacketTag (hopCountTag);
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
Popcache::OnData (Ptr<Face> inFace,
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

  PopularityTag Popularity;
  double p;
  data->GetPayload ()->PeekPacketTag (Popularity);
  p = Popularity.Get ();

  DistanceTag Distance;
  int d;
  data->GetPayload ()->PeekPacketTag (Distance);
  d = Distance.Get ();

  double prob=p*b/d;
  
  // generate the random number
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

      //Probability
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

} // namespace fw
} // namespace ndn
} // namespace ns3

