/*
 * pop-cache-mid-stat.cc
 *
 *  Created on: 2014年9月20日
 *      Author: zhi
 */

#include "pop-cache-mid-stat.h"

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

NS_OBJECT_ENSURE_REGISTERED (Popcachemidstat);

LogComponent Popcachemidstat::g_log = LogComponent (Popcachemidstat::GetLogName ().c_str ());

std::string
Popcachemidstat::GetLogName ()
{
  return super::GetLogName ()+".Popcachemidstat";
}


TypeId
Popcachemidstat::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::Popcachemidstat")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <Popcachemidstat> ()
    ;
  return tid;
}

Popcachemidstat::Popcachemidstat ()
{
}

void
Popcachemidstat::NotifyNewAggregate ()
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
Popcachemidstat::DoDispose ()
{
  m_pit = 0;
  m_pst = 0;	//clean the PST, zhi add
  m_contentStore = 0;
  m_fib = 0;

  Object::DoDispose ();
}

void
Popcachemidstat::OnInterest (Ptr<Face> inFace,
                             Ptr<Interest> interest)
{
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);
 
  // Update the Incoming Interest Packet Counts information, zhi add
  if(!UpdatePstInformation (interest))
  {
    // Failed to Update the PST Information, should do anything?
  }
   
  // record the hops of interest passed
  FwHopCountTag hopCountTag;
  uint32_t hops;
  interest->GetPayload ()->PeekPacketTag (hopCountTag);
  hops = hopCountTag.Get();
  
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

      pitEntry->AddIncoming (inFace, hops/*, Seconds (1.0)*/); // add the hops para

      // Do data plane performance measurements
      WillSatisfyPendingInterest (0, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (0, contentObject, pitEntry);
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

  PropagateInterest (inFace, interest, pitEntry, hops);
}

void
Popcachemidstat::OnData (Ptr<Face> inFace,
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
  
  // caculate the cache probility, zhi add
  
  BenefitTag Benefit;
  int b;
  data->GetPayload ()->PeekPacketTag (Benefit);
  Benefit.Increment ();
  b = Benefit.Get ();
  // Update the benefit value
  ConstCast<Packet> (data->GetPayload ())->ReplacePacketTag (Benefit);
  data->SetWire (0); // clean the Wiredata
  
  // Get the popularity base on the Tag
  //PopularityTag Popularity;
  //double p;
  //data->GetPayload ()->PeekPacketTag (Popularity);
  //p = Popularity.Get ();
  
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
  //std::cout<<pstEntry->GetPrefix()<<"\t"<<p1<<"\t"<<p<<"\t"<<"\n";
  //std::cout<<std::setprecision(4)<<p<<"\t"<<std::setprecision(4)<<q<<"\t"<<current<<"\t"<<first<<std::endl;
   
  DistanceTag Distance;
  int d;
  data->GetPayload ()->PeekPacketTag (Distance);
  d = Distance.Get ()+b;

  double prob=p*double(b)/double(d);
  
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
/*
void
Popcachemidstat::DidReceiveDuplicateInterest (Ptr<Face> inFace,
                                             Ptr<const Interest> interest,
                                             Ptr<pit::Entry> pitEntry)
{
  /////////////////////////////////////////////////////////////////////////////////////////
  //                                                                                     //
  // !!!! IMPORTANT CHANGE !!!! Duplicate interests will create incoming face entry !!!! //
  //                                                                                     //
  /////////////////////////////////////////////////////////////////////////////////////////
  pitEntry->AddIncoming (inFace);
  m_dropInterests (interest, inFace);
}
*/

void
Popcachemidstat::PropagateInterest (Ptr<Face> inFace,
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

void
Popcachemidstat::SatisfyPendingInterest (Ptr<Face> inFace,
                                         Ptr<Data> data,
                                         Ptr<pit::Entry> pitEntry)
{
  if (inFace != 0)
    pitEntry->RemoveIncoming (inFace);

  // get a copy of data
  DistanceTag Distance;
  Ptr<Packet> payloadCopy = data->GetPayload ()->Copy ();
  
  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      // change the distance tag of each data
      if (payloadCopy->RemovePacketTag (Distance))
      {
        Distance.Set(incoming.m_hops-1);
        payloadCopy->AddPacketTag (Distance);
      }         	         	
      data->SetPayload (payloadCopy);
      // end
       
      bool ok = incoming.m_face->SendData (data);

      DidSendOutData (inFace, incoming.m_face, data, pitEntry);
      NS_LOG_DEBUG ("Satisfy " << *incoming.m_face);

      if (!ok)
        {
          m_dropData (data, incoming.m_face);
          NS_LOG_DEBUG ("Cannot satisfy data to " << *incoming.m_face);
        }
    }

  // All incoming interests are satisfied. Remove them
  pitEntry->ClearIncoming ();

  // Remove all outgoing faces
  pitEntry->ClearOutgoing ();

  // Set pruning timout on PIT entry (instead of deleting the record)
  m_pit->MarkErased (pitEntry);
}

bool
Popcachemidstat::UpdatePstInformation (Ptr<const Interest> interest)
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

