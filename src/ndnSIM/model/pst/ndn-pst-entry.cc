/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
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

#include "ndn-pst-entry.h"

#include "ns3/ndn-pst.h"

#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/foreach.hpp>
namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("ndn.pst.Entry");

namespace ns3 {
namespace ndn {
namespace pst {

Entry::Entry (Pst &container,
              Ptr<const Name> prefix)
  : m_container (container)
  , m_prefix (prefix)
  //, m_maxRetxCount (0)
  , m_incoming (1)
  , m_outgoing (0)
{
  NS_LOG_FUNCTION (this);

  // UpdateLifetime is (and should) be called from the forwarding strategy

  UpdateLifetime (Seconds (600.0));  //no timeout, no fressness, zhi add
  //UpdateLifetime ((!header->GetInterestLifetime ().IsZero ()?
  //                 header->GetInterestLifetime ():
  //                 Seconds (1.0)));
}

Entry::~Entry ()
{
  NS_LOG_FUNCTION (GetPrefix ());
}

void
Entry::UpdateLifetime (const Time &offsetTime)
{
  NS_LOG_FUNCTION (this);

  Time newExpireTime = Simulator::Now () + (m_container.GetMaxPstEntryLifetime ().IsZero () ?
                                            offsetTime :
                                            std::min (m_container.GetMaxPstEntryLifetime (), offsetTime));
  if (newExpireTime > m_expireTime)
    m_expireTime = newExpireTime;

  NS_LOG_INFO (this->GetPrefix () << ", Updated lifetime to " << m_expireTime.ToDouble (Time::S) << "s, " << (m_expireTime-Simulator::Now ()).ToDouble (Time::S) << "s left");
}

const Name &
Entry::GetPrefix () const
{
  return *m_prefix;
}

const Time &
Entry::GetExpireTime () const
{
  return m_expireTime;
}

double
Entry::GetIncoming () const
{
  return m_incoming;
}

void
Entry::SetIncoming (double incoming)
{
  m_incoming=incoming;
}

void
Entry::IncomingInc ()
{
  m_incoming++;
}

double
Entry::GetOutgoing () const
{
  return m_outgoing;
}

void
Entry::SetOutgoing (double outgoing)
{
  m_outgoing=outgoing;
}

void
Entry::OutgoingInc ()
{
  m_outgoing++;
}

std::ostream& operator<< (std::ostream& os, const Entry &entry)
{
  os << "Prefix: " << entry.GetPrefix () << "\n";
  os << "In: ";
  /*
  bool first = true;
  BOOST_FOREACH (const IncomingFace &face, entry.m_incoming)
    {
      if (!first)
        os << ",";
      else
        first = false;

      os << *face.m_face;
    }

  os << "\nOut: ";
  first = true;
  BOOST_FOREACH (const OutgoingFace &face, entry.m_outgoing)
    {
      if (!first)
        os << ",";
      else
        first = false;

      os << *face.m_face;
    }
  os << "\nNonces: ";
  first = true;
  BOOST_FOREACH (uint32_t nonce, entry.m_seenNonces)
    {
      if (!first)
        os << ",";
      else
        first = false;

      os << nonce;
    }
  */
  return os;
}

} // namespace pst
} // namespace ndn
} // namespace ns3
