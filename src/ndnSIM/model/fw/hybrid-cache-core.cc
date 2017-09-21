/*
 * hybrid-cache-core.cc
 *
 *  Created on: 2017年9月9日
 *      Author: zhi
 */

#include "hybrid-cache-core.h"

#include "ns3/ndn-interest.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-content-store.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-benefit-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-distance-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-popularity-tag.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
//#include "ns3/simulator.h"
//#include "ns3/boolean.h"
//#include "ns3/string.h"

//#include <boost/ref.hpp>
#include <boost/foreach.hpp>
//#include <boost/lambda/lambda.hpp>
//#include <boost/lambda/bind.hpp>
//#include <boost/tuple/tuple.hpp>

#include "ns3/random-variable.h"

#include <iostream>
#include <iomanip>

//namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (HybirdCacheCore);

LogComponent HybirdCacheCore::g_log = LogComponent (HybirdCacheCore::GetLogName ().c_str ());

std::string
HybirdCacheCore::GetLogName ()
{
  return super::GetLogName ()+".HybirdCacheCore";
}


TypeId
HybirdCacheCore::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::HybirdCacheCore")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <HybirdCacheCore> ()
    ;
  return tid;
}

HybirdCacheCore::HybirdCacheCore ()
{
}

void
HybirdCacheCore::OnInterest (Ptr<Face> inFace,
                             Ptr<Interest> interest)
{
  if (interest->GetNack () > 0)
    {
      OnNack (inFace, interest);
      return;
    }  
  
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
          DidCreatePitEntry (inFace, interest, pitEntry);  // do nothing
        }
      else
        {
          FailedToCreatePitEntry (inFace, interest); // drop interest
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

      pitEntry->AddIncoming (inFace/*, Seconds (1.0)*/); // add the hops para

      // Do data plane performance measurements
      WillSatisfyPendingInterest (0, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (0, contentObject, pitEntry);
      return;
    }

  // record the hops of interest passed
  DistanceTag CorehopCountTag;
  uint32_t hops = 0;
  if(interest->GetPayload ()->PeekPacketTag (CorehopCountTag))
  {
    hops = CorehopCountTag.Get();
    //std::cout<<"Node="<<GetObject<Node>()->GetId()<<"\t"<<"hops="<<hops<<std::endl;
  
    // Update the benefit value
    CorehopCountTag.Set (hops+1);
    ConstCast<Packet> (interest->GetPayload ())->ReplacePacketTag (CorehopCountTag);
    interest->SetWire (0); // clean the Wiredata
  }

  if (similarInterest && ShouldSuppressIncomingInterest (inFace, interest, pitEntry))
    {
      pitEntry->AddIncoming (inFace, hops/*, interest->GetInterestLifetime ()*/);
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
      DidForwardSimilarInterest (inFace, interest, pitEntry); // do nothing
    }

  PropagateInterest (inFace, interest, pitEntry, hops);
}

void
HybirdCacheCore::OnData (Ptr<Face> inFace,
                         Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);

  // Lookup PIT entry

  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () & Face::APPLICATION)))
        {
          // Optimistically add or update entry in the content store
          cached = m_contentStore->Add (data);
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
      // caculate the cache probility
      BenefitTag Benefit;
      data->GetPayload ()->PeekPacketTag (Benefit);
      Benefit.Increment ();
      uint32_t k = Benefit.Get ();
      // Update the benefit value
      ConstCast<Packet> (data->GetPayload ())->ReplacePacketTag (Benefit);
      data->SetWire (0); // clean the Wiredata
  
      PopularityTag Rank;
      data->GetPayload ()->PeekPacketTag (Rank);
      double rank = Rank.Get ();
      
      double hops = 0;
      uint32_t i = 0;
      BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
        {
           hops += incoming.m_hops;
           i++;
        }

      double Lc = hops/i + k;

      uint32_t N = 10000;  //Capacity of Contents
  
      double prob = Lc ? (N-rank+1)/N*k/Lc : 0;
  
      //generate the random number
      UniformVariable m_prob(0, 1); 
      double p_prob = m_prob.GetValue();

      //std::cout<<std::setprecision(2)<<"Node"<<GetObject<Node>()->GetId()<<" Core"<<"\t"<<"Rank="<<rank<<"\t"<<"k="<<k<<"\t"<<"Lc="<<Lc<<"\t"<<prob<<std::endl;
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

void
HybirdCacheCore::PropagateInterest (Ptr<Face> inFace,
                                    Ptr<const Interest> interest,
                                    Ptr<pit::Entry> pitEntry,
                                    uint32_t hops)
{
  bool isRetransmitted = m_detectRetransmissions && // a small guard
                         DetectRetransmittedInterest (inFace, interest, pitEntry);

  pitEntry->AddIncoming (inFace, hops/*, interest->GetInterestLifetime ()*/);
  /// @todo Make lifetime per incoming interface
  pitEntry->UpdateLifetime (interest->GetInterestLifetime ());

  bool propagated = DoPropagateInterest (inFace, interest, pitEntry);

  if (!propagated && isRetransmitted) //give another chance if retransmitted
    {
      // increase max number of allowed retransmissions
      pitEntry->IncreaseAllowedRetxCount ();

      // try again
      propagated = DoPropagateInterest (inFace, interest, pitEntry);
    }

  // if (!propagated)
  //   {
  //     NS_LOG_DEBUG ("++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  //     NS_LOG_DEBUG ("+++ Not propagated ["<< interest->GetName () <<"], but number of outgoing faces: " << pitEntry->GetOutgoing ().size ());
  //     NS_LOG_DEBUG ("++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  //   }

  // ForwardingStrategy will try its best to forward packet to at least one interface.
  // If no interests was propagated, then there is not other option for forwarding or
  // ForwardingStrategy failed to find it.
  if (!propagated && pitEntry->AreAllOutgoingInVain ())
    {
      DidExhaustForwardingOptions (inFace, interest, pitEntry);
    }
}

} // namespace fw
} // namespace ndn
} // namespace ns3

