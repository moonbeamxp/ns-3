/*
 * hybrid-cache-edge.cc
 *
 *  Created on: 2017年9月9日
 *      Author: zhi
 */

#include "hybrid-cache-edge.h"

#include "ns3/ndn-interest.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-content-store.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
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

//#include <iostream>

//namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (HybirdCacheEdge);

LogComponent HybirdCacheEdge::g_log = LogComponent (HybirdCacheEdge::GetLogName ().c_str ());

std::string
HybirdCacheEdge::GetLogName ()
{
  return super::GetLogName ()+".HybirdCacheEdge";
}


TypeId
HybirdCacheEdge::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::HybirdCacheEdge")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <HybirdCacheEdge> ()
    ;
  return tid;
}

HybirdCacheEdge::HybirdCacheEdge ()
{
}

void
HybirdCacheEdge::OnInterest (Ptr<Face> inFace,
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
  FwHopCountTag hopCountTag;
  interest->GetPayload ()->PeekPacketTag (hopCountTag);
  uint32_t hops = hopCountTag.Get();

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
HybirdCacheEdge::OnData (Ptr<Face> inFace,
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
      bool cache = false;
      
      PopularityTag Rank;
      data->GetPayload ()->PeekPacketTag (Rank);
      double   rank = Rank.Get ();
      uint32_t distance = (*(pitEntry->GetIncoming ()).begin()).m_hops; 
        
      if(m_contentStore->IsFull())
        {
          uint32_t cs_Size = m_contentStore->GetSize ();
          if(distance*cs_Size+1 <= rank && rank <= (distance+1)*cs_Size) cache = true;
          //std::cout<<"Node"<<GetObject<Node>()->GetId()<<" Edge"<<"\t"<<"Hops="<<distance<<"\t"<<"cs_Size="<<cs_Size<<"\t"<<std::endl;
        }
      else
        {
          cache = true;
        }
      
      bool cached = false;
      // Cache Condition Judgement
      if(cache) cached = m_contentStore->Add (data);

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
HybirdCacheEdge::PropagateInterest (Ptr<Face> inFace,
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

