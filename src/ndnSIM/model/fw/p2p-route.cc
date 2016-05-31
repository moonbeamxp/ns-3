/*
 * p2p-route.cc
 *
 *  Created on: 2015年1月7日
 *      Author: zhi
 */

#include "p2p-route.h"

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

#include "ns3/random-variable.h"

//#include <iostream>
//#include <iomanip>

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (P2PRoute);

LogComponent P2PRoute::g_log = LogComponent (P2PRoute::GetLogName ().c_str ());

std::string
P2PRoute::GetLogName ()
{
  return super::GetLogName ()+".P2PRoute";
}


TypeId
P2PRoute::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::P2PRoute")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <P2PRoute> ()
    ;
  return tid;
}

P2PRoute::P2PRoute ()
{
}

void
P2PRoute::SatisfyPendingInterest (Ptr<Face> inFace,
                                  Ptr<const Data> data,
                                  Ptr<pit::Entry> pitEntry)
{
  if (inFace != 0)
    pitEntry->RemoveIncoming (inFace);

  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      bool ok = incoming.m_face->SendData (data);

      DidSendOutData (inFace, incoming.m_face, data, pitEntry);
      
      // code to add the reverse route path acorrding to the client request
      DistanceTag Distance;
      int d;
      data->GetPayload ()->PeekPacketTag (Distance);
      d = Distance.Get ();   
      
      FwHopCountTag hopCountTag;
      int h;
      data->GetPayload ()->PeekPacketTag (hopCountTag);
      h = hopCountTag.Get();
      
      if(2*d-h<2&&2*d-h>0)  //if the distance between router and client is less than 1, then add the reverse route
        Ptr<fib::Entry> entry = m_fib->Add (data->GetName(),incoming.m_face,1); 
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

void
P2PRoute::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  NS_LOG_DEBUG ("WillEraseTimedOutPendingInterest for " << pitEntry->GetPrefix ());

  // if the PendingInterest Entry is timeout, then erase the correspond FIB Entry.
  m_fib->Remove (&pitEntry->GetPrefix ());

  super::WillEraseTimedOutPendingInterest (pitEntry);
}

bool
P2PRoute::DoPropagateInterest (Ptr<Face> inFace,
                               Ptr<const Interest> interest,
                               Ptr<pit::Entry> pitEntry)
{
  NS_LOG_FUNCTION (this << interest->GetName ());

  // No real need to call parent's (green-yellow-red's strategy) method, since it is incorporated
  // in the logic of the BestRoute strategy
  //
  // // Try to work out with just green faces
  // bool greenOk = super::DoPropagateInterest (inFace, interest, origPacket, pitEntry);
  // if (greenOk)
  //   return true;

  int propagatedCount = 0;

  BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ())
    {
      NS_LOG_DEBUG ("Trying " << boost::cref(metricFace));
      if (metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) // all non-read faces are in front
        break;
        
      if(inFace==metricFace.GetFace ()) //if the inFace is same as the FIB Entry, then erase the Entry.
        {
          m_fib->Remove (&pitEntry->GetPrefix ());
          continue;
        }

      if (!TrySendOutInterest (inFace, metricFace.GetFace (), interest, pitEntry))
        {
          continue;
        }

      propagatedCount++;
      break; // do only once
    }

  NS_LOG_INFO ("Propagated to " << propagatedCount << " faces");
  return propagatedCount > 0;
}

} // namespace fw
} // namespace ndn
} // namespace ns3

