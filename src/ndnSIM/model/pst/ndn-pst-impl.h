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

#ifndef _NDN_PST_IMPL_H_
#define	_NDN_PST_IMPL_H_

#include "ndn-pst.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "../../utils/trie/sort-trie.h"
#include "../../utils/trie/desc-policy.h"

#include "ndn-pst-entry-impl.h"

#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-name.h"

namespace ns3 {
namespace ndn {

namespace pst {

/**
 * @ingroup ndn-pst
 * @brief Class implementing Pending Interests Table
 */

class PstImpl : public Pst
              , protected ndnSIM::sort_trie<Name,
                                            ndnSIM::smart_pointer_payload_traits< EntryImpl< PstImpl > >,
                                            ndnSIM::desc_policy_traits
                                            >
{
public:
  typedef ndnSIM::sort_trie<Name,
                            ndnSIM::smart_pointer_payload_traits< EntryImpl< PstImpl > >,
                            ndnSIM::desc_policy_traits
                            > super;
  typedef EntryImpl< PstImpl > entry;

  /**
   * \brief Interface ID
   *
   * \return interface ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief PST constructor
   */
  PstImpl ();

  /**
   * \brief Destructor
   */
  virtual ~PstImpl ();

  // inherited from Pst
  virtual Ptr<Entry>
  Lookup (const Data &header);

  virtual Ptr<Entry>
  Lookup (const Interest &header);

  virtual Ptr<Entry>
  Find (const Name &prefix);

  virtual Ptr<Entry>
  Create (Ptr<const Interest> header);

  virtual void
  Update (Ptr<Entry> item);

  //virtual void
  //MarkErased (Ptr<Entry> entry);

  virtual void
  Print (std::ostream &os) const;

  virtual uint32_t
  GetSize () const;
  
  /******** no use, zhi add  ********
  
  virtual double
  GetCurrentTotal ();
  
  virtual void
  SetCurrentTotal (double);
  
  ***********************************/

  virtual double
  GetCurrentFirst () const; // Get the access counts of the most popular chunk, zhi add
  
  virtual void
  SetCurrentFirst (double); // Set the counts of the most popular chunk, zhi add
   
  virtual double
  GetThresholdValue () const;
  
  virtual void
  UpdateThresholdValue ();

  virtual Ptr<Entry>
  Begin ();

  virtual Ptr<Entry>
  End ();

  virtual Ptr<Entry>
  Next (Ptr<Entry>);

  const typename super::policy_container &
  GetPolicy () const { return super::getPolicy (); }

  typename super::policy_container &
  GetPolicy () { return super::getPolicy (); }

protected:
  void RescheduleCleaning ();
  void CleanExpired ();

  // inherited from Object class
  virtual void NotifyNewAggregate (); ///< @brief Even when object is aggregated to another Object
  virtual void DoDispose (); ///< @brief Do cleanup

private:
  uint32_t
  GetMaxSize () const;

  void
  SetMaxSize (uint32_t maxSize);

  uint32_t
  GetCurrentSize () const;
  
  uint32_t
  GetThreshold () const;
  
  void
  SetThreshold (uint32_t Threshold);
  
private:
  EventId m_cleanEvent;

  static LogComponent g_log; ///< @brief Logging variable

  // indexes
  typedef
  boost::intrusive::multiset<entry,
                        boost::intrusive::compare < TimestampIndex< entry > >,
                        boost::intrusive::member_hook< entry,
                                                       boost::intrusive::set_member_hook<>,
                                                       &entry::time_hook_>
                        > time_index;
  time_index i_time;

  friend class EntryImpl< PstImpl >;
  
  uint32_t m_Threshold;         // the Date whose rank execced the Threshold will be cached
  double   m_Threshold_Value;   // the Date whose request rate execced the Threshold will be cached
  double   m_First;             // record access counts of the most popular chunk, zhi add
  //double m_Total;             // record the total access times
};

} // namespace pst
} // namespace ndn
} // namespace ns3

#endif	/* NDN_PST_IMPL_H */
