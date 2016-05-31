/*
 * pop-cache-stat.h
 *
 *  Created on: 2014年9月20日
 *      Author: zhi
 */

#ifndef POP_CACHE_STAT_H_
#define POP_CACHE_STAT_H_

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
class Popcachestat :
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
  Popcachestat ();

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


#endif /* POP_CACHE_STAT_H_ */

