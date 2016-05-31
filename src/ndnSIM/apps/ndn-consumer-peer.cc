/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 CNIC, P.R.China
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
 * Author: Jiang Zhi
 */

#include "ndn-consumer-peer.h"

#include "ns3/ndn-data.h"
#include "ndn-consumer-cbr.h"
#include "ndn-consumer-window.h"
#include "ndn-consumer-zipf-mandelbrot-unique.h"

namespace ns3 {
namespace ndn {

template class ConsumerPeer<ConsumerCbr>;
typedef ConsumerPeer<ConsumerCbr> ConsumerPeerCbr;
NS_OBJECT_ENSURE_REGISTERED (ConsumerPeerCbr);

template class ConsumerPeer<ConsumerWindow>;
typedef ConsumerPeer<ConsumerWindow> ConsumerPeerWindow;
NS_OBJECT_ENSURE_REGISTERED (ConsumerPeerWindow);

template class ConsumerPeer<ConsumerZipfMandelbrotUnique>;
typedef ConsumerPeer<ConsumerZipfMandelbrotUnique> ConsumerPeerZipfMandelbrotUnique;
NS_OBJECT_ENSURE_REGISTERED (ConsumerPeerZipfMandelbrotUnique);

#ifdef DOXYGEN
// /**
//  * \brief Strategy implementing per-out-face limits on top of BestRoute strategy
//  */
class ConsumerCbr::ConsumerPeer : public ::ns3::ndn::ConsumerPeer<ConsumerCbr> { };

class ConsumerWindow::ConsumerPeer : public ::ns3::ndn::ConsumerPeer<ConsumerWindow> { };

class ConsumerZipfMandelbrotUnique::ConsumerPeer : public ::ns3::ndn::ConsumerPeer<ConsumerZipfMandelbrotUnique> { };

#endif

} // namespace ndn
} // namespace ns3

