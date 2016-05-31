/*
 * constant-prob.cc
 *
 *  Created on: 2014年6月30日
 *      Author: zhi
 */

#include "constant-prob.h"

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

#include "ns3/random-variable.h"

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (ConstantProb);

LogComponent ConstantProb::g_log = LogComponent (ConstantProb::GetLogName ().c_str ());

std::string
ConstantProb::GetLogName ()
{
  return super::GetLogName ()+".ConstantProb";
}


TypeId
ConstantProb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::ConstantProb")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <ConstantProb> ()
    .AddAttribute ("Prob", "probability of caching",
                   StringValue ("0.5"),
                   MakeDoubleAccessor (&ConstantProb::SetProb, &ConstantProb::GetProb),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

ConstantProb::ConstantProb ()
{
}


void
ConstantProb::SetProb (double prob)
{
  m_prob = prob;
}

double
ConstantProb::GetProb () const
{
  return m_prob;
}


void
ConstantProb::OnData (Ptr<Face> inFace,
                            Ptr<Data> data)
{
  NS_LOG_FUNCTION (inFace << data->GetName ());
  m_inData (data, inFace);

  //generate the random number

  UniformVariable m_Prob(0, 1); 
  double p_prob = m_Prob.GetValue();	

  // Lookup PIT entry

  Ptr<pit::Entry> pitEntry = m_pit->Lookup (*data);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () & Face::APPLICATION)))
        {
          // Optimistically add or update entry in the content store
          if(p_prob<=m_prob) cached = m_contentStore->Add (data);
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

      //Probability
      if(p_prob<=m_prob) cached = m_contentStore->Add (data);

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

