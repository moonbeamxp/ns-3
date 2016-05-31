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

#include "ndn-pst-impl.h"

#include "ns3/string.h"
#include "ns3/uinteger.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

using namespace boost::tuples;
using namespace boost;
namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("ndn.pst.PstImpl");

namespace ns3 {
namespace ndn {
namespace pst {

LogComponent PstImpl::g_log = LogComponent ("ndn.pst.Default");

NS_OBJECT_ENSURE_REGISTERED (PstImpl);

TypeId
PstImpl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::pst::Default")
    .SetGroupName ("Ndn")
    .SetParent<Pst> ()
    .AddConstructor< PstImpl > ()
    .AddAttribute ("MaxSize",
                   "Set maximum size of PST in bytes. If 0, limit is not enforced",
                   UintegerValue (0),
                   MakeUintegerAccessor (&PstImpl::GetMaxSize,
                                         &PstImpl::SetMaxSize),
                   MakeUintegerChecker<uint32_t> ())

    .AddAttribute ("CurrentSize", "Get current size of PST in bytes",
                   TypeId::ATTR_GET,
                   UintegerValue (0),
                   MakeUintegerAccessor (&PstImpl::GetCurrentSize),
                   MakeUintegerChecker<uint32_t> ())
                   
    .AddAttribute ("Threshold",
                   "Set Threshold of PST",
                   UintegerValue (0),
                   MakeUintegerAccessor (&PstImpl::GetThreshold,
                                         &PstImpl::SetThreshold),
                   MakeUintegerChecker<uint32_t> ())
    ;

  return tid;
}

struct isNotExcluded
{
  inline
  isNotExcluded (const Exclude &exclude)
    : m_exclude (exclude)
  {
  }
  
  bool
  operator () (const name::Component &comp) const
  {
    return !m_exclude.isExcluded (comp);
  }

private:
  const Exclude &m_exclude;
};

PstImpl::PstImpl () : m_Threshold_Value(1), m_First (1)
{
}

PstImpl::~PstImpl ()
{
}

uint32_t
PstImpl::GetMaxSize () const
{
  return super::getPolicy ().get_max_size ();
}

void
PstImpl::SetMaxSize (uint32_t maxSize)
{
  super::getPolicy ().set_max_size (maxSize);
}

uint32_t
PstImpl::GetCurrentSize () const
{
  return super::getPolicy ().size ();
}

uint32_t
PstImpl::GetThreshold () const    //zhi add
{
  return m_Threshold;
}

void
PstImpl::SetThreshold (uint32_t Threshold)       //zhi add
{
  m_Threshold = Threshold;
}

void
PstImpl::NotifyNewAggregate ()
{
  Pst::NotifyNewAggregate ();
}

void
PstImpl::DoDispose ()
{
  super::clear ();
  Pst::DoDispose ();
}

void
PstImpl::RescheduleCleaning ()
{
  // m_cleanEvent.Cancel ();
  Simulator::Remove (m_cleanEvent); // slower, but better for memory
  if (i_time.empty ())
    {
      // NS_LOG_DEBUG ("No items in PST");
      return;
    }

  Time nextEvent = i_time.begin ()->GetExpireTime () - Simulator::Now ();
  if (nextEvent <= 0) nextEvent = Seconds (0);

  NS_LOG_DEBUG ("Schedule next cleaning in " <<
                nextEvent.ToDouble (Time::S) << "s (at " <<
                i_time.begin ()->GetExpireTime () << "s abs time");

  m_cleanEvent = Simulator::Schedule (nextEvent,
                                      &PstImpl::CleanExpired, this);
}

void
PstImpl::CleanExpired ()
{
  NS_LOG_LOGIC ("Cleaning PST. Total: " << i_time.size ());
  Time now = Simulator::Now ();

  // uint32_t count = 0;
  while (!i_time.empty ())
    {
      typename time_index::iterator entry = i_time.begin ();
      if (entry->GetExpireTime () <= now) // is the record stale?
        {
          //m_forwardingStrategy->WillEraseTimedOutPendingInterest (entry->to_iterator ()->payload ());
          super::erase (entry->to_iterator ());
          // count ++;
        }
      else
        break; // nothing else to do. All later records will not be stale
    }

  if (super::getPolicy ().size ())
    {
      NS_LOG_DEBUG ("Size: " << super::getPolicy ().size ());
      NS_LOG_DEBUG ("i_time size: " << i_time.size ());
    }
  RescheduleCleaning ();
}

Ptr<Entry>
PstImpl::Lookup (const Data &header)
{
  /// @todo use predicate to search with exclude filters
  
  typename super::iterator item = super::longest_prefix_match_if (header.GetName (), EntryIsNotEmpty ());

  if (item == super::end ())
    return 0;
  else
    return item->payload (); // which could also be 0
}

Ptr<Entry>
PstImpl::Lookup (const Interest &header)
{
  // NS_LOG_FUNCTION (header.GetName ());

  typename super::iterator item;
  if (header.GetExclude () == 0)
    {
      item = super::deepest_prefix_match (header.GetName ());
    }
  else
    {
      item = super::deepest_prefix_match_if_next_level (header.GetName (),
                                                        isNotExcluded (*header.GetExclude ()));
    }

  if (item == super::end ())
    return 0;
  else
    return item->payload (); // which could also be 0
}

Ptr<Entry>
PstImpl::Find (const Name &prefix)
{
  typename super::iterator item = super::find_exact (prefix);

  if (item == super::end ())
    return 0;
  else
    return item->payload ();
}

Ptr<Entry>
PstImpl::Create (Ptr<const Interest> header)
{
  //NS_LOG_DEBUG (header->GetName ());

  Ptr< entry > newEntry = ns3::Create< entry > (boost::ref (*this), header->GetNamePtr ());
  std::pair< typename super::iterator, bool > result = super::insert (header->GetName (), newEntry);
  if (result.first != super::end ())
    {
      if (result.second)
        {
          newEntry->SetTrie (result.first);
          return newEntry;
        }
      else
        {
          // should we do anything?
          // update payload? add new payload?
          return result.first->payload ();
        }
    }
  else
    return 0;
}

void
PstImpl::Update (Ptr<Entry> item)
{
  super::getPolicy ().update (StaticCast< entry > (item)->to_iterator ());
}
/*

void
PstImpl::MarkErased (Ptr<Entry> item)
{
  if (this->m_PstEntryPruningTimout.IsZero ())
    {
      super::erase (StaticCast< entry > (item)->to_iterator ());
    }
  else
    {
      item->OffsetLifetime (this->m_PstEntryPruningTimout - item->GetExpireTime () + Simulator::Now ());
    }
}
*/

/*
void
PstImpl::Print (std::ostream& os) const
{
  // !!! unordered_set imposes "random" order of item in the same level !!!
  typename super::parent_trie::const_recursive_iterator item (super::getTrie ()), end (0);
  for (; item != end; item++)
    {
      if (item->payload () == 0) continue;

      os << item->payload ()->GetPrefix () << "\t" << *item->payload () << "\n";
    }
}
*/

void
PstImpl::Print (std::ostream& os) const
{
  // !!! unordered_set imposes "random" order of item in the same level !!!
  typename super::policy_container::const_iterator item (super::getPolicy ().begin ()), end (super::getPolicy ().end ());
  for (; item != end; item++)
    {
      if (item->payload () == 0) continue;
      os <<item->payload ()->GetIncoming ()<<"\t"<<item->payload ()->GetOutgoing ()<<"\t"<<item->payload ()->GetPrefix ()<<"\n";
    }
}

uint32_t
PstImpl::GetSize () const
{
  return super::getPolicy ().size ();
}

/* no use
double
PstImpl::GetCurrentTotal ()
{
  return m_Total;
}

void
PstImpl::SetCurrentTotal (double total)
{
  m_Total = total;
}
*/

double
PstImpl::GetCurrentFirst () const    //zhi add
{
  return m_First;
}

void
PstImpl::SetCurrentFirst (double first)       //zhi add
{
  m_First = first;
}

double
PstImpl::GetThresholdValue () const    //zhi add
{
  return m_Threshold_Value;
}

void
PstImpl::UpdateThresholdValue ()       //zhi add
{
  if(GetSize()<GetThreshold())
    {
      m_Threshold_Value = 1;
    }
  else
    {
      typename super::policy_container::const_iterator item (super::getPolicy ().begin ());
      
      for (uint32_t i=1; i<GetThreshold(); item++)
        {
          if (item->payload () == 0) continue;
          i++;
        }
      m_Threshold_Value = item->payload ()->GetIncoming ();
      //std::cout <<"NodeID == " << GetObject<Node>()->GetId() << "\t"<<item->payload ()->GetPrefix () <<"\t\t"<<m_Threshold_Value<<"\t"<<m_Threshold<<"\n";
    }
}

Ptr<Entry>
PstImpl::Begin ()
{
  typename super::parent_trie::recursive_iterator item (super::getTrie ()), end (0);
  for (; item != end; item++)
    {
      if (item->payload () == 0) continue;
      break;
    }

  if (item == end)
    return End ();
  else
    return item->payload ();
}

Ptr<Entry>
PstImpl::End ()
{
  return 0;
}

Ptr<Entry>
PstImpl::Next (Ptr<Entry> from)
{
  if (from == 0) return 0;

  typename super::parent_trie::recursive_iterator
    item (*StaticCast< entry > (from)->to_iterator ()),
    end (0);

  for (item++; item != end; item++)
    {
      if (item->payload () == 0) continue;
      break;
    }

  if (item == end)
    return End ();
  else
    return item->payload ();
}

}
} // namespace ndn
} // namespace ns3
