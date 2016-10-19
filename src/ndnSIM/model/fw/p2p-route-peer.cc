/*
 * p2p-route-peer.cc
 *
 *  Created on: 2015年9月23日
 *      Author: zhi
 */

#include "p2p-route-peer.h"

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
#include "ns3/ndnSIM/utils/ndn-fw-distance-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-invalid-tag.h"

#include "ns3/random-variable.h"

//#include <iostream>
//#include <iomanip>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (P2PRoutePeer);

LogComponent P2PRoutePeer::g_log = LogComponent (P2PRoutePeer::GetLogName ().c_str ());

std::string
P2PRoutePeer::GetLogName ()
{
  return super::GetLogName ()+".P2PRoutePeer";
}

TypeId
P2PRoutePeer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::P2PRoutePeer")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <P2PRoutePeer> ()
    ;
  return tid;
}

P2PRoutePeer::P2PRoutePeer ()
{
}

void
P2PRoutePeer::OnInterest (Ptr<Face> inFace,
                          Ptr<Interest> interest)
{
  if (interest->GetNack () > 0)
    {
      OnNack (inFace, interest);
      return;
    }  
  
  uint32_t flag=inFace->GetFlags ();
  
  if(flag!=0)	//flag is positive integer means this face is a APPFace, the Interest packet is from this node itself, do normal operation
    {
      NS_LOG_DEBUG ("Send the Interest toward to Producer");
      super::OnInterest (inFace, interest);
      return;
    }

  //else, flag is 0 means this face is a normal face, the Interest packet is from the other consumer, do Peer Cache Operation
  NS_LOG_FUNCTION (inFace << interest->GetName ());
  m_inInterests (interest, inFace);
  
  Ptr<Data> contentObject;
  contentObject = m_contentStore->Lookup (interest);
  
  if (contentObject == 0)		// CS miss, generate the Invalid Data packet to inform consumer retransmit interest packet
    {
      // create the invalid data which is used to clear PIT entry and response to consumer
      Ptr<Name> dataName = Create<Name> (interest->GetName ());
      contentObject = Create<Data> (Create<Packet> (128));
      contentObject->SetName (dataName);
      contentObject->SetTimestamp (Simulator::Now());
      
      InvalidTag Invalid;
      contentObject->GetPayload ()->AddPacketTag (Invalid);  //add the Invalid tag to the Data packet

      //NS_LOG_INFO ("Peer("<< GetNode()->GetId() <<") respodning with Invalid Data: " << contentObject->GetName ());
    }
    
  // Echo back FwHopCountTag if exists  
  FwHopCountTag hopCountTag;
  if (interest->GetPayload ()->PeekPacketTag (hopCountTag))
    {
      contentObject->GetPayload ()->AddPacketTag (hopCountTag);
    }
    
  // return the contentObject via inFace, from which the Interest packet is recieved
  bool ok = inFace->SendData (contentObject);
  DidSendOutData (inFace, inFace, contentObject, NULL);
  NS_LOG_DEBUG ("Successfully send the Invalid Data from PEER to " << *inFace);
  if (!ok)
    {
      m_dropData (contentObject, inFace);
      NS_LOG_DEBUG ("Cannot send the Invalid Data from PEER to " << *inFace);
    }   
}

//add the Invalid tag judgement in order to avoid cache the Invalid Data
void
P2PRoutePeer::OnData (Ptr<Face> inFace, Ptr<Data> data)
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

  // Lookup PIT entry
  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () & Face::APPLICATION)))
        {
          // Optimistically add or update entry in the content store
          if(!invalid) //avoid cache the Invalid Data
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
      
      if(!invalid) //avoid cache the Invalid Data
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
P2PRoutePeer::ExpiredNotify (const Name& name)
{
  std::cout<<name<<"\n";
}

} // namespace fw
} // namespace ndn
} // namespace ns3

