/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef LB_POLICY_H_
#define LB_POLICY_H_

#include <boost/intrusive/options.hpp>
#include <boost/intrusive/set.hpp>

#include "ns3/ndnSIM/utils/ndn-fw-benefit-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-distance-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-popularity-tag.h"

//#include <iostream>
//#include <iomanip>

namespace ns3 {
namespace ndn {
namespace ndnSIM {

/**
 * @brief Traits for LB replacement policy
 */
struct lb_policy_traits
{
  /// @brief Name that can be used to identify the policy (for NS-3 object model and logging)
  static std::string GetName () { return "Lb"; }

  struct policy_hook_type : public boost::intrusive::set_member_hook<> { double benifit; };

  template<class Container>
  struct container_hook
  {
    typedef boost::intrusive::member_hook< Container,
                                           policy_hook_type,
                                           &Container::policy_hook_ > type;
  };

  template<class Base,
           class Container,
           class Hook>
  struct policy
  {
    static double& get_order (typename Container::iterator item)
    {
      return static_cast<policy_hook_type*>
        (policy_container::value_traits::to_node_ptr(*item))->benifit;
    }

    static const double& get_order (typename Container::const_iterator item)
    {
      return static_cast<const policy_hook_type*>
        (policy_container::value_traits::to_node_ptr(*item))->benifit;
    }

    template<class Key>
    struct MemberHookLess
    {
      bool operator () (const Key &a, const Key &b) const
      {
        return get_order (&a) < get_order (&b);
      }
    };

    typedef boost::intrusive::multiset< Container,
                                   boost::intrusive::compare< MemberHookLess< Container > >,
                                   Hook > policy_container;

    // could be just typedef
    class type : public policy_container
    {
    public:
      typedef policy policy_base; // to get access to get_order methods from outside
      typedef Container parent_trie;

      type (Base &base)
        : base_ (base)
        , max_size_ (100)
      {
      }

      inline void
      update (typename parent_trie::iterator item)
      {
        // do nothing
      }

      inline bool
      insert (typename parent_trie::iterator item)
      {     
        //zhi add
        BenefitTag Benefit;
        int b;
        item->payload()->GetData ()->GetPayload ()->PeekPacketTag (Benefit);
        b = Benefit.Get ();

        DistanceTag Distance;
        int d;
        item->payload()->GetData ()->GetPayload ()->PeekPacketTag (Distance);
        d = Distance.Get ()+b;
        
        PopularityTag Popularity;
        double p;
        item->payload()->GetData ()->GetPayload ()->PeekPacketTag (Popularity);
        p = Popularity.Get ();

        get_order (item) = p*b/d;
        
        //std::cout<<std::setprecision(2)<<p<<"\t"<<b<<"\t"<<d<<"\t"<<get_order (item)<<std::endl;

        if (max_size_ != 0 && policy_container::size () >= max_size_)
          {
            // this erases the "least benefit item" from cache
            base_.erase (&(*policy_container::begin ()));
            // std::cout<<"replaced\n";
          }

        policy_container::insert (*item);
        return true;
      }

      inline void
      lookup (typename parent_trie::iterator item)
      {
        // do nothing
      }

      inline void
      erase (typename parent_trie::iterator item)
      {
        policy_container::erase (policy_container::s_iterator_to (*item));
      }

      inline void
      clear ()
      {
        policy_container::clear ();
      }

      inline void
      set_max_size (size_t max_size)
      {
        max_size_ = max_size;
      }

      inline size_t
      get_max_size () const
      {
        return max_size_;
      }

    private:
      type () : base_(*((Base*)0)) { };

    private:
      Base &base_;
      size_t max_size_;
    };
  };
};

} // ndnSIM
} // ndn
} // ns3

#endif // LB_POLICY_H
