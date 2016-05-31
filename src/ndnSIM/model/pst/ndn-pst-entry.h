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

#ifndef _NDN_PST_ENTRY_H_
#define _NDN_PST_ENTRY_H_

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

#include "ns3/ndn-name.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
// #include <boost/multi_index/composite_key.hpp>
// #include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
// #include <boost/multi_index/mem_fun.hpp>
#include <set>
#include <boost/shared_ptr.hpp>

namespace ns3 {
namespace ndn {

class Pst;

namespace pst {

class Entry : public SimpleRefCount<Entry>
{
public:

  Entry (Pst &container, Ptr<const Name> prefix);

  virtual ~Entry ();

  virtual void
  UpdateLifetime (const Time &lifetime);

  const Name &
  GetPrefix () const;

  const Time &
  GetExpireTime () const;
  
  virtual double
  GetIncoming () const; // Get the counts of incoming Interest, zhi add
  
  virtual void
  SetIncoming (double); // Set the counts of incoming Interest, zhi add
  
  virtual void
  IncomingInc ();       // Increace the counts of incoming Interest by 1, zhi add
  
  virtual double
  GetOutgoing () const; // Get the counts of outgoing Interest, zhi add
  
  virtual void
  SetOutgoing (double); // Set the counts of outgoing Interest, zhi add
  
  virtual void
  OutgoingInc ();       // Increace the counts of outgoing Interest by 1, zhi add
  
private:
  friend std::ostream& operator<< (std::ostream& os, const Entry &entry);

protected:
  Pst &m_container; ///< @brief Reference to the container (to rearrange indexes, if necessary)

  Ptr<const Name> m_prefix; ///< \brief Interest of the PST entry (if several interests are received, then nonce is from the first Interest)

  Time m_expireTime;         ///< \brief Time when PST entry will be removed

  double m_incoming;    // counts of incoming Interest, zhi add
  double m_outgoing;    // counts of outgoing Interest, zhi add

  //std::list< boost::shared_ptr<fw::Tag> > m_fwTags; ///< @brief Forwarding strategy tags
};

/// @cond include_hidden

struct EntryIsNotEmpty
{
  bool
  operator () (Ptr<Entry> entry)
  {
    //return !entry->GetIncoming ().empty ();
    return 1;
  }
};

/// @endcond

std::ostream& operator<< (std::ostream& os, const Entry &entry);

} // namespace pst
} // namespace ndn
} // namespace ns3

#endif // _NDN_PST_ENTRY_H_
