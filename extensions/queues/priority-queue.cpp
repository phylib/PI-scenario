/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/ppp-header.h"
#include "ns3/ndnSIM/model/ndn-ns3.hpp"
#include "ns3/simulator.h"

#include "utils/ndn-ns3-packet-tag.hpp"
#include "ns3/ndnSIM/ndn-cxx/encoding/qci.hpp"

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include "priority-queue.hpp"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PriorityQueue");

NS_OBJECT_ENSURE_REGISTERED (PriorityQueue);

TypeId PriorityQueue::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::PriorityQueue")
    .SetParent<Queue> ()
    .SetGroupName("Network")
    .AddConstructor<PriorityQueue> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&PriorityQueue::SetMode,
                                     &PriorityQueue::GetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this PriorityQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&PriorityQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this PriorityQueue.",
                   UintegerValue (100 * 1024),
                   MakeUintegerAccessor (&PriorityQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

PriorityQueue::PriorityQueue () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0)
{
  NS_LOG_FUNCTION (this); 
}

PriorityQueue::~PriorityQueue ()
{
  NS_LOG_FUNCTION (this);
}

void
PriorityQueue::SetMode (PriorityQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

PriorityQueue::QueueMode
PriorityQueue::GetMode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
PriorityQueue::SetDropPolicy (PriorityQueue::DropPolicy policy)
{
  m_dropPolicy = policy;
}

PriorityQueue::DropPolicy
PriorityQueue::GetDropPolicy () const
{
  return m_dropPolicy;
}

bool 
PriorityQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  Ptr<Packet> packet = p->Copy();

  PppHeader ppp;
  packet->RemoveHeader(ppp);

  // Convert the packet to Interest or Data and get QCI class
  std::string name = ""; 
  uint32_t prio = ndn::QCI_CLASSES::QCI_9; // Default QCI class is 9
  try {
    switch (ndn::Convert::getPacketType(packet)) {
      case ::ndn::tlv::Interest: {
        std::shared_ptr<const ::ndn::Interest> i = ndn::Convert::FromPacket<::ndn::Interest>(packet);
        ::ndn::Name prefix = i->getName();
        prefix = prefix.getPrefix(2);
        name = prefix.toUri();
        if (i->getQCI() != 0) {
          prio = i->getQCI();
        }
        break;
      }
      case ::ndn::tlv::Data: {
        std::shared_ptr<const ::ndn::Data> d = ndn::Convert::FromPacket<::ndn::Data>(packet);
        ::ndn::Name prefix = d->getName();
        prefix = prefix.getPrefix(2);
        name = prefix.toUri();
        if (d->getQCI() != 0) {
          prio = d->getQCI();
        }
        break;
      }
      // case ::ndn::tlv::Nack: {
      //   shared_ptr<const Nack> n = Convert::FromPacket<Nack>(packet);
      //   this->onReceiveNack(*n);
      // }
      default:
        NS_LOG_ERROR("Unsupported TLV packet");
    }
  }
  catch (const ::ndn::tlv::Error& e) {
    NS_LOG_ERROR("Unrecognized TLV packet " << e.what());
  }

  NS_LOG_DEBUG("Queuing packet with name: " << name);

  //NS_LOG_FUNCTION()

  if (m_mode == QUEUE_MODE_PACKETS && (m_packets.size() >= m_maxPackets))
    {
      NS_LOG_LOGIC ("Queue full (at max packets) -- droppping pkt");
      Drop (p);
      return false;
    }

  if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
    {
      NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- droppping pkt");
      Drop (p);
      return false;
    }

  m_bytesInQueue += p->GetSize ();

  // Add packet to queue
  NS_LOG_DEBUG("Priority of packet " << prio);
  m_packets.push(p, prio);


  NS_LOG_LOGIC ("Number packets " << m_packets.size());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return true;
}

Ptr<Packet>
PriorityQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (m_packets.size() == 0)
  {
    NS_LOG_LOGIC ("Queue empty");
    return 0;
  }

  auto p = m_packets.pop();

  m_bytesInQueue -= p->GetSize ();

  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

Ptr<const Packet>
PriorityQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.size() == 0)
  {
    NS_LOG_LOGIC ("Queue empty");
    return 0;
  }

  auto p = m_packets.top();

  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

} // namespace ns3

