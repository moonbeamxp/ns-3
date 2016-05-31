/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "ndn-fw-distance-tag.h"

namespace ns3 {
namespace ndn {

TypeId
DistanceTag::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::ndn::DistanceTag")
    .SetParent<Tag>()
    .AddConstructor<DistanceTag>()
    ;
  return tid;
}

TypeId
DistanceTag::GetInstanceTypeId () const
{
  return DistanceTag::GetTypeId ();
}

uint32_t
DistanceTag::GetSerializedSize () const
{
  return sizeof(uint32_t);
}

void
DistanceTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_Distance);
}
  
void
DistanceTag::Deserialize (TagBuffer i)
{
  m_Distance = i.ReadU32 ();
}

void
DistanceTag::Print (std::ostream &os) const
{
  os << m_Distance;
}

} // namespace ndn
} // namespace ns3
