/*
 * ndn-fw-content-copy-tag.h
 *
 *  Created on: 2013年8月5日
 *      Author: wang
 */

#ifndef NDN_FW_CONTENT_COPY_TAG_H_
#define NDN_FW_CONTENT_COPY_TAG_H_

#include "ns3/tag.h"

namespace ns3 {
namespace ndn {

/**
 * @brief Packet tag that is used to track hop count for Interest-Data pairs
 */
class FwcontentCopyTag : public Tag
{
public:
  static TypeId
  GetTypeId (void);

  /**
   * @brief Default constructor
   * 数据包发送时将c_copyTag置为true，使第一个接收到数据包的节点可以缓存该数据包
   */
  FwcontentCopyTag () : c_copyTag (1) { };
  
  

  /**
   * @brief Destructor
   */
  ~FwcontentCopyTag () { }

  /**
   * @brief Increment hop count
   * 第一个接收到数据包的节点缓存数据后，调用Increment()，目的是到达cunsumer之前的节点(除第一个节点)不缓存该数据
   */
  void
  ChangeTrue () { c_copyTag = 1; }
  
  void
  ChangeFalse () { c_copyTag = 0; }
  
  

  /**
   * @brief Get value of hop count
   */
  uint32_t
  Get () const { return c_copyTag; }
  
  //virtual void
  //SetcontentCopyTag (bool copytag);
  

  ////////////////////////////////////////////////////////
  // from ObjectBase
  ////////////////////////////////////////////////////////
  virtual TypeId
  GetInstanceTypeId () const;

  ////////////////////////////////////////////////////////
  // from Tag
  ////////////////////////////////////////////////////////
/**/
  virtual uint32_t
  GetSerializedSize () const;

  virtual void
  Serialize (TagBuffer i) const;

  virtual void
  Deserialize (TagBuffer i);

  virtual void
  Print (std::ostream &os) const;

private:
  //节点用来判断是否缓存内容的变量，true节点将数据缓存，false则不缓存数据
  uint32_t c_copyTag;
};

} // namespace ndn
} // namespace ns3



#endif /* NDN_FW_CONTENT_COPY_TAG_H_ */

