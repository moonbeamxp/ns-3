/*
 * prob-cache-revise.h
 *
 *  Created on: 2017年9月13日
 *      Author: zhi
 */

#ifndef PROB_CACHE_REVISE_H_
#define PROB_CACHE_REVISE_H_

#include "pop-cache-revise.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class Probcacherevise :
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
  Probcacherevise ();

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


#endif /* PROB_CACHE_REVISE_H_ */

