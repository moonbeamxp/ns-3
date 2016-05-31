/*
 * lcu-cache.cc
 *
 *  Created on: 2016年5月23日
 *      Author: zhi
 */

#include "lcu-cache.h"

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
#include "ns3/ndnSIM/utils/ndn-fw-cacheid-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-cacheinfo-tag.h"

//#include <iostream>
//#include <iomanip>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (LCUCache);

LogComponent LCUCache::g_log = LogComponent (LCUCache::GetLogName ().c_str ());

std::string
LCUCache::GetLogName ()
{
  return super::GetLogName ()+".LCUCache";
}


TypeId
LCUCache::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::LCUCache")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <LCUCache> ()
    ;
  return tid;
}

LCUCache::LCUCache ()
{
}

void
LCUCache::NotifyNewAggregate ()
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
LCUCache::DoDispose ()
{
  m_pit = 0;
  m_pst = 0;	//clean the PST, zhi add
  m_contentStore = 0;
  m_fib = 0;

  Object::DoDispose ();
}

void
LCUCache::OnInterest (Ptr<Face> inFace,
                      Ptr<Interest> interest)
{
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);
  
  // Update the Incoming Interest Packet Counts information, zhi add
  Ptr<pst::Entry> pstEntry = UpdateIncoming (interest);

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

  // record the hops of interest passed
  FwHopCountTag hopCountTag;
  uint32_t hops;
  interest->GetPayload ()->PeekPacketTag (hopCountTag);
  hops = hopCountTag.Get();
  
  CacheID   CacheIdTag;
  interest->GetPayload ()->PeekPacketTag (CacheIdTag);
  
  CacheInfo CacheInfoTag;
  interest->GetPayload ()->PeekPacketTag (CacheInfoTag);
  
  //std::cout<<CacheIdTag.Get()<<"\t"<<CacheInfoTag.Get()<<"\n";
  
  //ConstCast<Packet> (interest->GetPayload ())->ReplacePacketTag (CacheIdTag);
  //interest->SetWire (0); // clean the Wiredata
  
  Ptr<Data> contentObject;
  contentObject = m_contentStore->Lookup (interest);
  if (contentObject != 0)
    {   
      FwHopCountTag hopCountTag;
      if (interest->GetPayload ()->PeekPacketTag (hopCountTag))
        {
          contentObject->GetPayload ()->AddPacketTag (hopCountTag);

        }
      // add the Cache ID Tag to the data from Content Store to indicate the cache place
      contentObject->GetPayload ()->AddPacketTag (CacheIdTag);
      
      pitEntry->AddIncoming (inFace, hops/*, Seconds (1.0)*/); // add the hops para

      // Do data plane performance measurements
      WillSatisfyPendingInterest (0, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (0, contentObject, pitEntry);
          
      // Make decision whether need to swap out the cached item the downstream node decided to cache
      if(CacheIdTag.Get()!=-1)
      {
        pstEntry->SetIncoming(pstEntry->GetIncoming()-CacheInfoTag.Get());
        
        if(pstEntry->GetIncoming() < m_pst->GetThresholdValue ())
        {
          m_contentStore->Remove (contentObject->GetName());
        }
      }
      return;
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


  pstEntry->OutgoingInc();
  
  /*
  Ptr<Name> debug = Create<Name> ("/prefix/%00%0F");
  if(interest->GetName () == *debug)
  {
    std::cout<<pstEntry->GetIncoming()<<"\t"<<pstEntry->GetOutgoing()<<"\t"<< GetObject<Node>()->GetId()<<"\t"<<CacheIdTag.Get()<<"\n";
  }*/
  
  if(CacheIdTag.Get()==-1)	// no node determine cache the chunk before
  {
    // cache decision
    if(pstEntry->GetIncoming() >= m_pst->GetThresholdValue ())
    {
      CacheIdTag.Set(GetObject<Node>()->GetId());
      ConstCast<Packet> (interest->GetPayload ())->ReplacePacketTag (CacheIdTag);
      
      CacheInfoTag.Set(pstEntry->GetOutgoing());
      ConstCast<Packet> (interest->GetPayload ())->ReplacePacketTag (CacheInfoTag);

      interest->SetWire (0);	// clean the Wiredata
           
      pstEntry->SetOutgoing(0);
    }
  }
  else	// the downstream node has dicided to cache this chunk
  {
    if(pstEntry->GetIncoming()-CacheInfoTag.Get() < 0)
    {
      std::cout<<pstEntry->GetPrefix()<<"\t"<<pstEntry->GetIncoming()<<"\t"<<CacheInfoTag.Get()<<"\t"<<pstEntry->GetIncoming()-CacheInfoTag.Get()<< GetObject<Node>()->GetId()<<"\n";
    }
    
    double proportion = CacheInfoTag.Get()/pstEntry->GetIncoming();
    double outgoing = pstEntry->GetOutgoing();
    
    //if(proportion!=1) std::cout<<"Proportion="<<proportion<<"\t"<<pstEntry->GetPrefix()<<"\t"<<pstEntry->GetIncoming()<<"\t"<<CacheInfoTag.Get()<<"\t"<< GetObject<Node>()->GetId()<<"\n";
    
    //pstEntry->SetIncoming(pstEntry->GetIncoming()-CacheInfoTag.Get());
    pstEntry->SetIncoming(std::max(pstEntry->GetIncoming()-CacheInfoTag.Get(),0.0));
    m_pst->Update(pstEntry);	// update the order of the PST Entry
    
    CacheInfoTag.Set(outgoing*proportion);
    ConstCast<Packet> (interest->GetPayload ())->ReplacePacketTag (CacheInfoTag);
    interest->SetWire (0); // clean the Wiredata
    
    pstEntry->SetOutgoing(outgoing*(1-proportion));
  }
  

  PropagateInterest (inFace, interest, pitEntry, hops);
}

void
LCUCache::OnData (Ptr<Face> inFace,
                  Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);

  CacheID   CacheIdTag;
  data->GetPayload ()->PeekPacketTag (CacheIdTag);

  // Lookup PIT entry
  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () & Face::APPLICATION)))
        {
          if(!m_contentStore->IsFull() || (uint32_t)CacheIdTag.Get()==GetObject<Node>()->GetId()) cached = m_contentStore->Add (data);
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

      if(!m_contentStore->IsFull() || (uint32_t)CacheIdTag.Get()==GetObject<Node>()->GetId()) cached = m_contentStore->Add (data);

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
LCUCache::PropagateInterest (Ptr<Face> inFace,
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

Ptr<pst::Entry>
LCUCache::UpdateIncoming (Ptr<const Interest> interest)
{
  // add the statistic information to the PST, zhi add
  
  // Lookup operation will increase the access times of existed prefix by one
  Ptr<pst::Entry> pstEntry = m_pst->Lookup (*interest);
   
  if (pstEntry == 0)
  {
    pstEntry = m_pst->Create (interest);  // create the pst Entry, use prifix in interest
    if (pstEntry == 0)
    {
      NS_LOG_ERROR("Failed to create the pstEntry");
    }
  }
  else
  { 
    if(pstEntry->GetIncoming() > m_pst->GetThresholdValue () && pstEntry->GetIncoming()-1 <= m_pst->GetThresholdValue ())
    {
      m_pst->UpdateThresholdValue ();
    }
  }
  return pstEntry;
}

} // namespace fw
} // namespace ndn
} // namespace ns3

