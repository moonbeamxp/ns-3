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

#ifndef _NDN_PST_H_
#define	_NDN_PST_H_

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

#include "ndn-pst-entry.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn
 * @defgroup ndn-pst PST
 */

/**
 * @ingroup ndn-pst
 * @brief Namespace for PST operations
 */
namespace pst {
}

class L3Protocol;
class Face;
class Data;
class Interest;

typedef Interest InterestHeader;
typedef Data DataHeader;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/**
 * @ingroup ndn-pst
 * @brief Class implementing Pending Interests Table
 */
class Pst : public Object
{
public:

  static TypeId GetTypeId ();

  Pst ();

  virtual ~Pst ();

  virtual Ptr<pst::Entry>
  Lookup (const Data &header) = 0;

  virtual Ptr<pst::Entry>
  Lookup (const Interest &header) = 0;

  virtual Ptr<pst::Entry>
  Find (const Name &prefix) = 0;

  virtual Ptr<pst::Entry>
  Create (Ptr<const Interest> header) = 0;

  virtual void
  Update (Ptr<pst::Entry> item) = 0;
  
  virtual void
  Print (std::ostream &os) const = 0;

  virtual uint32_t
  GetSize () const = 0;
  
  //virtual double
  //GetCurrentTotal () = 0;  // no use, zhi add
  
  //virtual void
  //SetCurrentTotal (double) = 0; // no use, zhi add
  
  virtual double
  GetCurrentFirst () const = 0;  // Get the access counts of the most popular chunk, zhi add
  
  virtual void
  SetCurrentFirst (double) = 0;  // Set the counts of the most popular chunk, zhi add
  
  virtual double
  GetThresholdValue () const = 0;

  virtual void
  UpdateThresholdValue () = 0;
  
  virtual Ptr<pst::Entry>
  Begin () = 0;

  virtual Ptr<pst::Entry>
  End () = 0;

  virtual Ptr<pst::Entry>
  Next (Ptr<pst::Entry>) = 0;

  static inline Ptr<Pst>
  GetPst (Ptr<Object> node);

  inline const Time&
  GetMaxPstEntryLifetime () const;

  inline void
  SetMaxPstEntryLifetime (const Time &maxLifetime);

protected:
  // configuration variables. Check implementation of GetTypeId for more details
  Time m_PstEntryPruningTimout;

  Time m_maxPstEntryLifetime;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline std::ostream&
operator<< (std::ostream& os, const Pst &pst)
{
  pst.Print (os);
  return os;
}

inline Ptr<Pst>
Pst::GetPst (Ptr<Object> node)
{
  return node->GetObject<Pst> ();
}

inline const Time&
Pst::GetMaxPstEntryLifetime () const
{
  return m_maxPstEntryLifetime;
}

inline void
Pst::SetMaxPstEntryLifetime (const Time &maxLifetime)
{
  m_maxPstEntryLifetime = maxLifetime;
}

} // namespace ndn
} // namespace ns3

#endif	/* NDN_PST_H */
