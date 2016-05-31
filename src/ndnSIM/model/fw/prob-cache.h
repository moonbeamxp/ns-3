/*
 * prob-cache.h
 *
 *  Created on: 2014年7月1日
 *      Author: zhi
 */

#ifndef PROB_CACHE_H_
#define PROB_CACHE_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class Probcache :
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
  Probcache ();

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


#endif /* PROB_CACHE_H_ */

