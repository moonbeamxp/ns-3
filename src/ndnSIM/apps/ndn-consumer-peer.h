/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 CNIC, P.R.China
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jiang Zhi
 */


#ifndef NDN_CONSUMER_PEER_H_
#define NDN_CONSUMER_PEER_H_

#include "ndn-consumer.h"

#include "ns3/event-id.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-invalid-tag.h"

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerPeer");

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief NDN app requesting contents following Zipf-Mandelbrot Distbituion
 *
 * The class implements an app which requests contents following Zipf-Mandelbrot Distribution
 * Here is the explaination of Zipf-Mandelbrot Distribution: http://en.wikipedia.org/wiki/Zipf%E2%80%93Mandelbrot_law
 */
 
template<class Parent>
class ConsumerPeer : public Parent
{
private:
  typedef Parent super;
  
  EventId m_ResendEvent; //used to triggle the event of recieving Invalid data

public:
  /**
   * @brief Get TypeId of the class
   */
  static TypeId GetTypeId ();

  /**
   * @brief Default constructor
   */
  ConsumerPeer () 
  { }

  /// \copydoc Consumer::OnData
  virtual void
  OnData (Ptr<const Data> contentObject);
  
  virtual void
  Retransmit (uint32_t seq);
  
};

template<class Parent>
TypeId
ConsumerPeer<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::ConsumerPeer").c_str ())
    .SetGroupName ("Ndn")
    .template SetParent <super> ()
    .template AddConstructor <ConsumerPeer> ()
    ;
  return tid;
}

template<class Parent>
void
ConsumerPeer<Parent>::OnData (Ptr<const Data> data)
{
  uint32_t seq = data->GetName ().get (-1).toSeqNum ();
  
  //Get the Invalid tag from the Data packet
  bool invalid = false;
  InvalidTag Invalid;
  if(data->GetPayload ()->PeekPacketTag (Invalid))
    {
      invalid = Invalid.Get ();
    }
  if(!invalid)  //Revieve normal Data packet, call the super::Ondata() to response to the valid data packet recieving
    {
      NS_LOG_INFO ("< Recieve Normal DATA for " << seq);
      super::OnData(data);
      return;
    }

  //if the data packet invalid, retransmit the Interest packet immediately
  NS_LOG_INFO ("< Recieve Invalid DATA for " << seq<<", Retransmit Interest");
  
  if (!m_ResendEvent.IsRunning ())
    {
      m_ResendEvent = Simulator::Schedule (Seconds (0.0), &ConsumerPeer<Parent>::Retransmit, this, seq);  
    }
    
  // another way for retansmitting which need to wait for the next time slot
  /*
  ConsumerPeer<Parent>::m_retxSeqs.insert (seq);
  ConsumerPeer<Parent>::m_seqTimeouts.erase (seq);
  ConsumerPeer<Parent>::ScheduleNextPacket ();
  */
}

template<class Parent>
void
ConsumerPeer<Parent>::Retransmit (uint32_t seq)
{
  ConsumerPeer<Parent>::m_seqTimeouts.erase (seq);
  
  Ptr<Name> nameWithSequence = Create<Name> (ConsumerPeer<Parent>::m_interestName);
  nameWithSequence->appendSeqNum (seq);
  
  Ptr<Interest> interest = Create<Interest> ();
  interest->SetName                (nameWithSequence);
  interest->SetNonce               (ConsumerPeer<Parent>::m_rand.GetValue ());
  interest->SetInterestLifetime    (ConsumerPeer<Parent>::m_interestLifeTime);

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO ("> Interest for " << seq);

  ConsumerPeer<Parent>::WillSendOutInterest (seq);

  FwHopCountTag hopCountTag;
  interest->GetPayload ()->AddPacketTag (hopCountTag);

  ConsumerPeer<Parent>::m_transmittedInterests (interest, this, ConsumerPeer<Parent>::m_face);
  ConsumerPeer<Parent>::m_face->ReceiveInterest (interest);
}

} /* namespace ndn */
} /* namespace ns3 */
#endif /* NDN_CONSUMER_PEER_H_ */
