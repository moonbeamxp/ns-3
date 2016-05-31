/*
 * p2p-route-index.h
 *
 *  Created on: 2015年9月23日
 *      Author: zhi
 */

#ifndef P2P_ROUTE_INDEX_H_
#define P2P_ROUTE_INDEX_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class P2PRouteIndex :
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
  P2PRouteIndex ();
  
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
  PropagateInterest (Ptr<Face> inFace,
                     Ptr<const Interest> interest,
                     Ptr<pit::Entry> pitEntry,
                     uint32_t hops);

  virtual void
  ReturnInvalidData (Ptr<Face> inFace, 
                     Ptr<const Interest> interest, 
                     Ptr<pit::Entry> pitEntry);
                     
  // from super  
  virtual void
  SatisfyPendingInterest (Ptr<Face> inFace, // 0 allowed (from cache)
                          Ptr<const Data> data,
                          Ptr<pit::Entry> pitEntry);
                          
protected:
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif /* P2P_ROUTE_INDEX_H_ */

