/*
 * prob-cache-revise.cc
 *
 *  Created on: 2017年9月13日
 *      Author: zhi
 */

#include "prob-cache-revise.h"

#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-content-store.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-benefit-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-distance-tag.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
//#include "ns3/simulator.h"
//#include "ns3/boolean.h"
//#include "ns3/string.h"

//#include <boost/ref.hpp>
//#include <boost/foreach.hpp>
//#include <boost/lambda/lambda.hpp>
//#include <boost/lambda/bind.hpp>
//#include <boost/tuple/tuple.hpp>

#include "ns3/random-variable.h"

//#include <iostream>
//#include <iomanip>

//namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (Probcacherevise);

LogComponent Probcacherevise::g_log = LogComponent (Probcacherevise::GetLogName ().c_str ());

std::string
Probcacherevise::GetLogName ()
{
  return super::GetLogName ()+".Probcacherevise";
}


TypeId
Probcacherevise::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::Probcacherevise")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <Probcacherevise> ()
    ;
  return tid;
}

Probcacherevise::Probcacherevise ()
{
}

void
Probcacherevise::OnData (Ptr<Face> inFace,
                         Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);

  // caculate the cache probility of Probcacherevise

  BenefitTag TSBTag;
  double x;
  data->GetPayload ()->PeekPacketTag (TSBTag);
  TSBTag.Increment ();
  x = TSBTag.Get ();
  // Update the TSBTag value
  ConstCast<Packet> (data->GetPayload ())->ReplacePacketTag (TSBTag);
  data->SetWire (0); // clean the Wiredata
  
  DistanceTag TSITag;
  double c;
  data->GetPayload ()->PeekPacketTag (TSITag);
  c = TSITag.Get () + x;
  
  double prob=(c-x+1)*x/10/c;
  
  // generate the random number
  UniformVariable m_prob(0, 1); 
  double p_prob = m_prob.GetValue();	

  //std::cout<<std::setprecision(2)<<"TSI="<<c<<"\t"<<"TSB="<<x<<"\t"<<"Prob="<<prob<<std::endl;

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

} // namespace fw
} // namespace ndn
} // namespace ns3

