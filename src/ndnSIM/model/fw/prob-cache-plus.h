/*
 * prob-cache-plus.h
 *
 *  Created on: 2017年9月14日
 *      Author: zhi
 */

#ifndef PROB_CACHE_PLUS_H_
#define PROB_CACHE_PLUS_H_

#include "pop-cache-revise.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class Probcacheplus :
    public Popcacherevise
{
private:
  typedef Popcacherevise super;

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
  Probcacheplus ();

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


#endif /* PROB_CACHE_PLUS_H_ */

