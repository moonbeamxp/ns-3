/*
 * constant-prob.h
 *
 *  Created on: 2014年6月30日
 *      Author: zhi
 */

#ifndef CONSTANT_PROB_H_
#define CONSTANT_PROB_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class ConstantProb :
    public BestRoute
{
private:
  typedef BestRoute super;

  void
  SetProb (double prob);

  double
  GetProb () const;

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
  ConstantProb ();

  // from super
  virtual void
  OnData (Ptr<Face> face,
          Ptr<Data> data);

protected:
  static LogComponent g_log;

private:
  double m_prob;
};

} // namespace fw
} // namespace ndn
} // namespace ns3


#endif /* CONSTANT_PROB_H_ */

