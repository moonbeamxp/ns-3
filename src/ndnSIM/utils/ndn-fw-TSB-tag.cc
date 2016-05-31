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

#include "ndn-fw-TSB-tag.h"

namespace ns3 {
namespace ndn {

TypeId
FwTSBTag::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::ndn::FwTSBTag")
    .SetParent<Tag>()
    .AddConstructor<FwTSBTag>()
    ;
  return tid;
}

TypeId
FwTSBTag::GetInstanceTypeId () const
{
  return FwTSBTag::GetTypeId ();
}

uint32_t
FwTSBTag::GetSerializedSize () const
{
  return sizeof(uint32_t);
}

void
FwTSBTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_TSB);
}
  
void
FwTSBTag::Deserialize (TagBuffer i)
{
  m_TSB = i.ReadU32 ();
}

void
FwTSBTag::Print (std::ostream &os) const
{
  os << m_TSB;
}

} // namespace ndn
} // namespace ns3
