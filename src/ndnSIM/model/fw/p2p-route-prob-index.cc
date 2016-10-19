/*
 * p2p-route-prob-index.cc
 *
 *  Created on: 2016年1月15日
 *      Author: zhi
 */

#include "p2p-route-prob-index.h"

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

#include "ns3/ndnSIM/utils/ndn-fw-invalid-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-benefit-tag.h"

#include "ns3/random-variable.h"

//#include <iostream>
//#include <iomanip>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (P2PRouteProbIndex);

LogComponent P2PRouteProbIndex::g_log = LogComponent (P2PRouteProbIndex::GetLogName ().c_str ());

std::string
P2PRouteProbIndex::GetLogName ()
{
  return super::GetLogName ()+".P2PRouteProbIndex";
}


TypeId
P2PRouteProbIndex::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::P2PRouteProbIndex")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <P2PRouteProbIndex> ()
    ;
  return tid;
}

P2PRouteProbIndex::P2PRouteProbIndex ()
{
}

void
P2PRouteProbIndex::NotifyNewAggregate ()
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
P2PRouteProbIndex::DoDispose ()
{
  m_pit = 0;
  m_pst = 0;	//clean the PST, zhi add
  m_contentStore = 0;
  m_fib = 0;

  Object::DoDispose ();
}

void
P2PRouteProbIndex::OnInterest (Ptr<Face> inFace,
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

  PropagateInterest (inFace, interest, pitEntry); //add the hops para
}

void
P2PRouteProbIndex::OnData (Ptr<Face> inFace, Ptr<Data> data)
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
  else
    {
      // Update the ls value
      BenefitTag Length_Server;
      data->GetPayload ()->PeekPacketTag (Length_Server);
      Length_Server.Increment ();
      ConstCast<Packet> (data->GetPayload ())->ReplacePacketTag (Length_Server);
      data->SetWire (0); // clean the Wiredata
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
P2PRouteProbIndex::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  NS_LOG_DEBUG ("WillEraseTimedOutPendingInterest for " << pitEntry->GetPrefix ());

  // if the PendingInterest Entry is timeout, then erase the correspond FIB Face or Entry, zhi add
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
  // m_fib->Remove (&pitEntry->GetPrefix ());  old method : delete the Entry directly, only consider the prefix, zhi add
  super::WillEraseTimedOutPendingInterest (pitEntry);
}

void
P2PRouteProbIndex::ReturnInvalidData(Ptr<Face> inFace, Ptr<const Interest> interest, Ptr<pit::Entry> pitEntry)
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
P2PRouteProbIndex::SatisfyPendingInterest (Ptr<Face> inFace,
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

  // get ls
  BenefitTag Length_Server;
  data->GetPayload ()->PeekPacketTag (Length_Server);
  int ls = Length_Server.Get ();
  
  // get p
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
  
  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      bool ok = incoming.m_face->SendData (data);

      DidSendOutData (inFace, incoming.m_face, data, pitEntry);
      
      // get lc
      int lc = incoming.m_hops;
      
      // caculate the probility of tracing
      double prob = 0;
      
      if(ls) prob = p*(ls-lc)/ls;
      
      //generate the random number
      UniformVariable m_prob(0, 1); 
      double ramdom = m_prob.GetValue();
      
      //std::cout<<std::setprecision(2)<<p<<"\t"<<ls<<"\t"<<lc<<"\t"<<prob<<std::endl;
      
      // Add the reverse route path acorrding to the client request
    
      if(ramdom<=prob)	//if the hops from client is no more than Index_Range, then add the index route
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

bool
P2PRouteProbIndex::UpdatePstInformation (Ptr<const Interest> interest)
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

