/*
 * p2p-route-prob-index.h
 *
 *  Created on: 2016年1月15日
 *      Author: zhi
 */

#ifndef P2P_ROUTE_PROB_INDEX_H_
#define P2P_ROUTE_PROB_INDEX_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {

class Pst;	// add the PST Class in the forwarding Strategy, zhi add
namespace pst { class Entry; }

namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class P2PRouteProbIndex :
    public BestRoute
{
private:
  typedef BestRoute super;

public:
  static TypeId
  GetTypeId ();

  /**
   * @brief Helper function to retrieve logging name for the forwarding strategy
   */
  static std::string
  GetLogName ();

  /**
   * @brief Default constructor
   */
  P2PRouteProbIndex ();
  
  // from super     
  virtual void
  OnInterest (Ptr<Face> face,
              Ptr<Interest> interest);
               
  // from super     
  virtual void
  OnData (Ptr<Face> face,
          Ptr<Data> data);
          
  // from super
  virtual void
  WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
 
protected:                           
  virtual void
  ReturnInvalidData (Ptr<Face> inFace, 
                     Ptr<const Interest> interest, 
                     Ptr<pit::Entry> pitEntry);
                     
  // from super  
  virtual void
  SatisfyPendingInterest (Ptr<Face> inFace, // 0 allowed (from cache)
                          Ptr<const Data> data,
                          Ptr<pit::Entry> pitEntry);
                          
  virtual bool
  UpdatePstInformation (Ptr<const Interest> interest);

protected:
  // inherited from Object class. Aggregate the PST to protocol stack, zhi add
  virtual void NotifyNewAggregate (); ///< @brief Even when object is aggregated to another Object
  virtual void DoDispose (); ///< @brief Do cleanup
                           
protected:
  Ptr<Pst> m_pst; // brief Reference to PST to which this forwarding strategy is associated, zhi add
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif /* P2P_ROUTE_PROB_INDEX_H_ */

