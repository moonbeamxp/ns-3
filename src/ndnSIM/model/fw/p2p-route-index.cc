/*
 * p2p-route-index.cc
 *
 *  Created on: 2015年9月23日
 *      Author: zhi
 */
#define Index_Range 1

#include "p2p-route-index.h"

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

#include "ns3/ndnSIM/utils/ndn-fw-invalid-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"

#include "ns3/random-variable.h"

//#include <iostream>
//#include <iomanip>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (P2PRouteIndex);

LogComponent P2PRouteIndex::g_log = LogComponent (P2PRouteIndex::GetLogName ().c_str ());

std::string
P2PRouteIndex::GetLogName ()
{
  return super::GetLogName ()+".P2PRouteIndex";
}


TypeId
P2PRouteIndex::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::P2PRouteIndex")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <P2PRouteIndex> ()
    ;
  return tid;
}

P2PRouteIndex::P2PRouteIndex ()
{
}

void
P2PRouteIndex::OnInterest (Ptr<Face> inFace,
                           Ptr<Interest> interest)
{
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);

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
      pitEntry = m_pit->Create (interest, inFace);	//use Create function with inFace judgement
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
      DidForwardSimilarInterest (inFace, interest, pitEntry);
    }

  PropagateInterest (inFace, interest, pitEntry, hops); //add the hops para
}

void
P2PRouteIndex::OnData (Ptr<Face> inFace, Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);

  //Get the Invalid tag from the Data packet
  bool invalid = false;
  InvalidTag Invalid;
  if(data->GetPayload ()->PeekPacketTag (Invalid))
    {
      invalid = Invalid.Get ();
    }
  //for each inValid Data packet, check the FIB and find the Entry, delete the face same as inFace or the whole Entry
  if(invalid)
    {
      //do operation when the data is Invalid, delete the face invalid
      Ptr<fib::Entry> fibEntry = m_fib->Find (data->GetName ());
      if (fibEntry != 0)
        {
          fibEntry->RemoveFace (inFace);
          if((*fibEntry).m_faces.size () == 0)
            {
              m_fib->Remove (&fibEntry->GetPrefix ());
            }
          NS_LOG_DEBUG ("Recieve Invalid DATA for "<<data->GetName()<<", Modify FIB Entry");
        }
    }
  // Lookup PIT entry
  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () & Face::APPLICATION)))
        {
          // Optimistically add or update entry in the content store
          if(!invalid)	//cache normal data
            {
              cached = m_contentStore->Add (data);     
            }
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
      
      if(!invalid)  //cache normal data
        {
          cached = m_contentStore->Add (data);     
        }
        
      DidReceiveSolicitedData (inFace, data, cached);
    }

  while (pitEntry != 0)
    {
      // Do data plane performance measurements
      if(!invalid)
        {
          WillSatisfyPendingInterest (inFace, pitEntry);
        }
        
      // Actually satisfy pending interest
      SatisfyPendingInterest (inFace, data, pitEntry);

      // Lookup another PIT entry
      pitEntry = m_pit->Lookup (*data);
    }
}

void
P2PRouteIndex::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  NS_LOG_DEBUG ("WillEraseTimedOutPendingInterest for " << pitEntry->GetPrefix ());

  // if the PendingInterest Entry is timeout, then erase the correspond FIB Face or Entry.
  Ptr<fib::Entry> fibEntry = m_fib->Find (pitEntry->GetPrefix ());
      if (fibEntry != 0)
        {
          BOOST_FOREACH (const pit::OutgoingFace &outgoing, pitEntry->GetOutgoing ())
            {
              fibEntry->RemoveFace (outgoing.m_face);
            }
          if((*fibEntry).m_faces.size () == 0)
            {
              m_fib->Remove (&fibEntry->GetPrefix ());
            }
          NS_LOG_DEBUG ("Due to PIT Entry Timeout, modify FIB Entry");
        }
  // m_fib->Remove (&pitEntry->GetPrefix ());  old method : delete the Entry directly, only consider the prefix
  super::WillEraseTimedOutPendingInterest (pitEntry);
}

void
P2PRouteIndex::PropagateInterest (Ptr<Face> inFace,
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
  if (!propagated)
    {
      ReturnInvalidData(inFace, interest, pitEntry);
    }
    
  /*if (!propagated && pitEntry->AreAllOutgoingInVain ())
    {
      DidExhaustForwardingOptions (inFace, interest, pitEntry);
    }*/
}

void
P2PRouteIndex::ReturnInvalidData(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
{
  // create the invalid data which is used to clear PIT entry and response to consumer
  Ptr<Name> dataName = Create<Name> (interest->GetName ());
  Ptr<Data> contentObject = Create<Data> (Create<Packet> (128));
  contentObject->SetName (dataName);
  contentObject->SetTimestamp (Simulator::Now());
  
  InvalidTag Invalid;
  contentObject->GetPayload ()->AddPacketTag (Invalid);  //add the Invalid tag to the Data packet

  //NS_LOG_INFO ("Peer("<< GetNode()->GetId() <<") respodning with Invalid Data: " << contentObject->GetName ());
  
  FwHopCountTag hopCountTag;
  if (interest->GetPayload ()->PeekPacketTag (hopCountTag))
    {
      contentObject->GetPayload ()->AddPacketTag (hopCountTag);
    }
  SatisfyPendingInterest (0, contentObject, pitEntry);
}

// add the index routing path when the PIT Entry is satisfied
void
P2PRouteIndex::SatisfyPendingInterest (Ptr<Face> inFace,
                                       Ptr<const Data> data,
                                       Ptr<pit::Entry> pitEntry)
{
  // Get the Invalid tag from the Data packet
  bool invalid = false;
  InvalidTag Invalid;
  if(data->GetPayload ()->PeekPacketTag (Invalid))
    {
      invalid = Invalid.Get ();
    }
    
  if(invalid)  //if the data packet invalid, forward the Invalid Data normally in order to retransmit the Interest packet immediately
    {
      NS_LOG_DEBUG ("Recieve Invalid DATA Packet, Just Forwarding");
      super::SatisfyPendingInterest (inFace, data, pitEntry);
      return;
    }
    
  if (inFace != 0)
    pitEntry->RemoveIncoming (inFace);

  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      bool ok = incoming.m_face->SendData (data);

      DidSendOutData (inFace, incoming.m_face, data, pitEntry);
      
      // Add the reverse route path acorrding to the client request
    
      if(incoming.m_hops<=Index_Range && incoming.m_hops>0) //if the hops from client is no more than Index_Range, then add the index route
        {
          Ptr<fib::Entry> entry = m_fib->Add (data->GetName(),incoming.m_face,incoming.m_hops); //incoming.m_hops is the cost
          NS_LOG_DEBUG ("Recieve Normal DATA for "<<data->GetName()<<", Add the FIB Entry to " << *incoming.m_face);
        }  

      //entry->SetRealDelayToProducer (incoming.m_face, 10);
      //end
                        
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

} // namespace fw
} // namespace ndn
} // namespace ns3
