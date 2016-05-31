/*
 * mcd-storage.h
 *
 *  Created on: 2013年8月9日
 *      Author: wang
 */

#ifndef MCD_STORAGE_H_
#define MCD_STORAGE_H_

#include "best-route.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class McdStorage :
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
  McdStorage ();


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


#endif /* MCD_STORAGE_H_ */

