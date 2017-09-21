/*
 * hybrid-cache-core.h
 *
 *  Created on: 2017年9月9日
 *      Author: zhi
 */

#ifndef HYBRID_CACHE_CORE_H_
#define HYBRID_CACHE_CORE_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class HybirdCacheCore :
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
  HybirdCacheCore ();

  // from super
  virtual void
  OnInterest (Ptr<Face> face,
              Ptr<Interest> interest);

  // from super
  virtual void
  OnData (Ptr<Face> face,
          Ptr<Data> data);

protected:
  virtual void
  PropagateInterest (Ptr<Face> inFace,
                     Ptr<const Interest> interest,
                     Ptr<pit::Entry> pitEntry,
                     uint32_t hops);
                     
protected:
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif /* HYBRID_CACHE_CORE_H_ */

