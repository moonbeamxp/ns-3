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

#ifndef _NDN_PST_ENTRY_IMPL_H_
#define	_NDN_PST_ENTRY_IMPL_H_

namespace ns3 {
namespace ndn {

class Pst;

namespace pst {

/**
 * @ingroup ndn-pst
 * @brief PST entry implementation with additional pointers to the underlying container
 */
template<class Pst>
class EntryImpl : public Entry
{
public:
  typedef Entry base_type;

  typedef Entry super;
  #define CONTAINER static_cast<Pst&> (m_container)
  
public:
  EntryImpl (Pst &pst,
             Ptr<const Name> prefix)
  : Entry (pst, prefix)
  , item_ (0)
  {
    //CONTAINER.i_time.insert (*this);
    //CONTAINER.RescheduleCleaning ();
  }
  
  virtual ~EntryImpl ()
  {
    //CONTAINER.i_time.erase (Pst::time_index::s_iterator_to (*this));
    
    //CONTAINER.RescheduleCleaning ();
  }

  virtual void
  UpdateLifetime (const Time &offsetTime)
  {
    CONTAINER.i_time.erase (Pst::time_index::s_iterator_to (*this));
    super::UpdateLifetime (offsetTime);
    CONTAINER.i_time.insert (*this);

    CONTAINER.RescheduleCleaning ();
  }

/*
  virtual void
  OffsetLifetime (const Time &offsetTime)
  {
    CONTAINER.i_time.erase (Pst::time_index::s_iterator_to (*this));
    super::OffsetLifetime (offsetTime);
    CONTAINER.i_time.insert (*this);

    CONTAINER.RescheduleCleaning ();
  }
*/
  
  // to make sure policies work
  void
  SetTrie (typename Pst::super::iterator item) { item_ = item; }

  typename Pst::super::iterator to_iterator () { return item_; }
  typename Pst::super::const_iterator to_iterator () const { return item_; }

public:
  boost::intrusive::set_member_hook<> time_hook_;
  
private:
  typename Pst::super::iterator item_;
};

/// @cond include_hidden
template<class T>
struct TimestampIndex
{
  bool
  operator () (const T &a, const T &b) const
  {
    return a.GetExpireTime () < b.GetExpireTime ();
  }
};
/// @endcond

} // namespace pst
} // namespace ndn
} // namespace ns3

#endif 
