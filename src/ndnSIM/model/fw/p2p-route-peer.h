/*
 * p2p-route-peer.h
 *
 *  Created on: 2015年9月23日
 *      Author: zhi
 */

#ifndef P2P_ROUTE_PEER_H_
#define P2P_ROUTE_PEER_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief P2P route strategy
 */
class P2PRoutePeer : public BestRoute
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
  P2PRoutePeer ();
        
  // from super
  virtual void
  OnInterest (Ptr<Face> face,
              Ptr<Interest> interest);
              
  // from super     
  virtual void
  OnData (Ptr<Face> face,
          Ptr<Data> data);
          
protected:
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif /* P2P_ROUTE_PEER_H_ */

