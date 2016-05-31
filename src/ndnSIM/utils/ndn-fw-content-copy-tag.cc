/*
 * ndn-fw-content-copy-tag.cc
 *
 *  Created on: 2013年8月5日
 *      Author: wang
 */

#include "ndn-fw-content-copy-tag.h"

namespace ns3 {
namespace ndn {

TypeId
FwcontentCopyTag::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::ndn::FwcontentCopyTag")
    .SetParent<Tag>()
    .AddConstructor<FwcontentCopyTag>()
    ;
  return tid;
}

TypeId
FwcontentCopyTag::GetInstanceTypeId () const
{
  return FwcontentCopyTag::GetTypeId ();
}

uint32_t
FwcontentCopyTag::GetSerializedSize () const
{
  return sizeof(uint32_t);
}

void
FwcontentCopyTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (c_copyTag);
}
/*
void
FwcontentCopyTag::SetcontentCopyTag (int copytag)
{
	c_copyTag=copytag;
}
*/  
void
FwcontentCopyTag::Deserialize (TagBuffer i)
{
  c_copyTag = i.ReadU32 ();
}

void
FwcontentCopyTag::Print (std::ostream &os) const
{
  os << c_copyTag;
}

} // namespace ndn
} // namespace ns3





