/*
 * p2p-route.h
 *
 *  Created on: 2015年1月7日
 *      Author: zhi
 */

#ifndef P2P_ROUTE_H_
#define P2P_ROUTE_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class P2PRoute :
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
  P2PRoute ();

  // from super
  virtual void
  WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
  
protected:        
  // from super  
  virtual void
  SatisfyPendingInterest (Ptr<Face> inFace, // 0 allowed (from cache)
                          Ptr<const Data> data,
                          Ptr<pit::Entry> pitEntry);
                                            
  // from super
  virtual bool
  DoPropagateInterest (Ptr<Face> incomingFace,
                       Ptr<const Interest> interest,
                       Ptr<pit::Entry> pitEntry);

protected:
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif /* P2P_ROUTE_H_ */

