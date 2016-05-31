/*
 * mcd-storage.cc
 *
 *  Created on: 2013年8月9日
 *      Author: wang
 */




#include "mcd-storage.h"

#include "best-route.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"

#include "ns3/ndn-data.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-store.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-content-copy-tag.h"
#include "ns3/ptr.h"
#include "ns3/ndn-face.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

#include "ns3/assert.h"
#include "ns3/log.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <iostream>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (McdStorage);

LogComponent McdStorage::g_log = LogComponent (McdStorage::GetLogName ().c_str ());

std::string
McdStorage::GetLogName ()
{
  return super::GetLogName ()+".McdStorage";
}


TypeId
McdStorage::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::McdStorage")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <McdStorage> ()
    ;
  return tid;
}

McdStorage::McdStorage ()
{
}

void
McdStorage::OnInterest (Ptr<Face> inFace,
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
      
      //wang add
      //数据包返回时将内容存储标记变量置为2
      
      FwcontentCopyTag contentCopyTag;
      contentObject->GetPayload ()->AddPacketTag (contentCopyTag);
      
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
McdStorage::OnData (Ptr<Face> inFace,
                            Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);

  // Lookup PIT entry
  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () | Face::APPLICATION)))
        {
        //wang add
          FwcontentCopyTag contentCopyTag;
          if(data->GetPayload ()->PeekPacketTag(contentCopyTag))
          {
            if(contentCopyTag.Get() == 1)
         	{
         			//std::cout<<"pitEntry != 0 contentCopyTag.Get() == true\n";
         		Ptr<Packet> payloadCopy = data->GetPayload ()->Copy ();
         	
         		if (payloadCopy->RemovePacketTag (contentCopyTag))
         		{
         			contentCopyTag.ChangeFalse();
         			payloadCopy->AddPacketTag (contentCopyTag);
         		}         	         	
         		data->SetPayload (payloadCopy);
           		// Optimistically add or update entry in the content store
          		cached = m_contentStore->Add (data);
           }
           else
           {
         	//	std::cout<<"pitEntry == 0 contentCopyTag.Get() == false\n";
           }
           
          }
          //wang add 
          m_contentStore->Erase(data);
         // std::cout<<"删除成功if\n";
          ////////////////////////////
        }
      else
        {
          // Drop data packet if PIT entry is not found
          // (unsolicited data packets should not "poison" content store)

          //drop dulicated or not requested data packet
          //wang add
          //std::cout << "删除 else\n";
          m_contentStore->Erase(data);
          ////////////////////////////
          m_dropData (data, inFace);
        }

      DidReceiveUnsolicitedData (inFace, data, cached);
      return;
    }
  else
    {
    	bool cached = false;
      //wang add
//      if(payload->RemovePacketTag (contentCopyTag))
      FwcontentCopyTag contentCopyTag;
      if(data->GetPayload ()->PeekPacketTag(contentCopyTag))
      {
         if(contentCopyTag.Get() == 1)
         {
         	//std::cout<<"pitEntry != 0 contentCopyTag.Get() == true\n";
         	Ptr<Packet> payloadCopy = data->GetPayload ()->Copy ();
         	
         	if (payloadCopy->RemovePacketTag (contentCopyTag))
         	{
         		contentCopyTag.ChangeFalse();
         		payloadCopy->AddPacketTag (contentCopyTag);
         	}         	         	
         	data->SetPayload (payloadCopy);
           // Optimistically add or update entry in the content store
          cached = m_contentStore->Add (data);
          inFace->SendData(data);
        }          
      }
      else
      {
     	// std::cout << "无contentCopyTag存储，转发inFace->SendData(data);\n";
        cached = m_contentStore->Add (data);
        inFace->SendData(data);
      }
      
      
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

