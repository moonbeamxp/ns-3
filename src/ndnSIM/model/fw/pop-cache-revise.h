/*
 * pop-cache-revise.h
 *
 *  Created on: 2014年7月1日
 *      Author: zhi
 */

#ifndef POP_CACHE_REVISE_H_
#define POP_CACHE_REVISE_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class Popcacherevise :
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
  Popcacherevise ();

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
                     
  virtual void
  SatisfyPendingInterest (Ptr<Face> inFace, // 0 allowed (from cache)
                          Ptr<Data> data,
                          Ptr<pit::Entry> pitEntry);
                     
protected:
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif /* POP_CACHE_REVISE_H_ */

