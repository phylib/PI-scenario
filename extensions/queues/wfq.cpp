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

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include "wfq.hpp"
#include "tags/ndn-queue-virtual-finish-time-tag.hpp"
#include "ns3/ndnSIM/ndn-cxx/encoding/qci.hpp"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WFQ");

NS_OBJECT_ENSURE_REGISTERED (WFQ);

TypeId WFQ::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::WFQ")
    .SetParent<Queue> ()
    .SetGroupName("Network")
    .AddConstructor<WFQ> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&WFQ::SetMode,
                                     &WFQ::GetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this WFQ.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&WFQ::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this WFQ.",
                   UintegerValue (100 * 1024),
                   MakeUintegerAccessor (&WFQ::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

WFQ::WFQ () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0)
{
  NS_LOG_FUNCTION (this); 
}

WFQ::~WFQ ()
{
  NS_LOG_FUNCTION (this);
}

void
WFQ::SetMode (WFQ::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

WFQ::QueueMode
WFQ::GetMode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

bool 
WFQ::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  Ptr<Packet> packet = p->Copy();

  PppHeader ppp;
  packet->RemoveHeader(ppp);

  std::string name = ""; 
  try {
    switch (ndn::Convert::getPacketType(packet)) {
      case ::ndn::tlv::Interest: {
        std::shared_ptr<const ::ndn::Interest> i = ndn::Convert::FromPacket<::ndn::Interest>(packet);
        ::ndn::Name prefix = i->getName();
        prefix = prefix.getPrefix(2); 
        name = prefix.toUri();
        break;
      }
      case ::ndn::tlv::Data: {
        std::shared_ptr<const ::ndn::Data> d = ndn::Convert::FromPacket<::ndn::Data>(packet);
        ::ndn::Name prefix = d->getName();
        prefix = prefix.getPrefix(2); 
        name = prefix.toUri();
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

  if (m_mode == QUEUE_MODE_PACKETS && (countPackets() >= m_maxPackets))
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

  // Check if name is already in queue
  if (!hasName(name)) {
    // Create new queue
    std::pair<std::string, std::deque<Ptr<Packet>>> pair (name, std::deque<Ptr<Packet>>());
    m_queues.insert(pair);
    // Create new VirtualFinishTime-Entry
    std::pair<std::string, uint32_t> finishPair (name, 0);
    m_virtualFinish.insert(finishPair);
    // Store name of queue
    m_queue_names.insert(m_queue_names.end(), name);
  }
  // Add packet to queue
  auto res = m_queues.find(name);
  res->second.push_back(p);
  
  // updateTime
  updateTime(p, name);


  NS_LOG_LOGIC ("Number packets " << countPackets());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
  NS_LOG_LOGIC ("Number of queues " << m_queue_names.size());

  return true;
}

Ptr<Packet>
WFQ::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (countPackets() == 0)
  {
    NS_LOG_LOGIC ("Queue empty");
    return 0;
  }

  if (m_queue_names.size() == 0) {
    m_currentQueue = 0;
  } else {
    if (m_mode == QUEUE_MODE_PACKETS) {
      m_currentQueue = (m_currentQueue + 1) % (m_queue_names.size());
    } else if (m_mode == QUEUE_MODE_BYTES) {
      
      m_currentQueue = selectQueue();

    }
  }

  std::string name = m_queue_names.at(m_currentQueue);
  NS_LOG_LOGIC("Dequeu Packet from Queue " << name);
  
  auto res = m_queues.find(name);
  Ptr<Packet> p = res->second.front();
  res->second.pop_front();

  if (res->second.size() == 0) {
    NS_LOG_LOGIC("Erase queue for name " << name);
    m_queues.erase(name);
    m_virtualFinish.erase(name);
    m_queue_names.erase(m_queue_names.begin() + m_currentQueue);
  }

  ndn::VirtualFinishTimeTag tag;
  p->RemovePacketTag(tag);

  m_bytesInQueue -= p->GetSize ();

  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << countPackets());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
  NS_LOG_LOGIC ("Number of queues " << m_queue_names.size());

  return p;
}

Ptr<const Packet>
WFQ::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  auto currentQueue = m_currentQueue;
  if (m_queue_names.size() == 0) {
    currentQueue = 0;
  } else {
    if (m_mode == QUEUE_MODE_PACKETS) {
      currentQueue = (currentQueue + 1) % (m_queue_names.size());
    } else if (m_mode == QUEUE_MODE_BYTES) {
      
      currentQueue = selectQueue();

    }
  }

  std::string name = m_queue_names.at(currentQueue);
  NS_LOG_LOGIC("Peek Packet from Queue " << name);
  
  auto res = m_queues.find(name);
  Ptr<Packet> p = res->second.front();

  NS_LOG_LOGIC ("Number packets " << countPackets());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
  NS_LOG_LOGIC ("Number of queues " << m_queue_names.size());

  return p;
}

uint
WFQ::countPackets(void) const
{
  int size = 0;
  for (auto it = m_queues.begin(); it != m_queues.end(); ++it) {
    size += it->second.size();
  }
  return size;
}

bool
WFQ::hasName(std::string name) const
{
  auto result = m_queues.find(name);
  if (result == m_queues.end()) {
    return false;
  }
  return true;
}

void
WFQ::updateTime(Ptr<const Packet> packet, std::string queueName)
{
  double finishRes = m_virtualFinish.find(queueName)->second;
  uint64_t now = Now().GetMilliSeconds();
  double virStart = (finishRes < now ? now : finishRes);
  ndn::VirtualFinishTimeTag tag;

  double relation = (double)getPriority(packet) / getCumulatedPriority();

  double weightedSize = packet->GetSize() * (1 - relation);

  double virFinish = virStart + weightedSize;
  tag.setVirtualFinishTime(virFinish);
  packet->AddPacketTag(tag);
  m_virtualFinish.find(queueName)->second = virFinish;
}

uint
WFQ::selectQueue() const
{
  uint selectedQueue = 0;
  int i = 0;
  double minVirFinish = std::numeric_limits<double>::max();
  for (auto it = m_queue_names.begin(); it != m_queue_names.end(); ++it) {
    auto queue = m_queues.find(*it)->second;
    if (queue.size() > 0) {
      auto packet = queue.front();
      ndn::VirtualFinishTimeTag tag;
      packet->RemovePacketTag(tag);
      double virFinish = tag.getVirtualFinishTime();
      if (virFinish < minVirFinish) {
        minVirFinish = virFinish;
        selectedQueue = i;
      }
      packet->AddPacketTag(tag);
    }
    i++;
  }
  return selectedQueue;
}

uint WFQ::getPriority(Ptr<const Packet> p) const
{
  Ptr<Packet> packet = p->Copy();

  PppHeader ppp;
  packet->RemoveHeader(ppp);

  uint32_t prio = ndn::QCI_CLASSES::QCI_9;
  try {
    switch (ndn::Convert::getPacketType(packet)) {
      case ::ndn::tlv::Interest: {
        std::shared_ptr<const ::ndn::Interest> i = ndn::Convert::FromPacket<::ndn::Interest>(packet);
        if (i-> getQCI() > 0) {
          prio = i->getQCI();
        }
        break;
      }
      case ::ndn::tlv::Data: {
        std::shared_ptr<const ::ndn::Data> d = ndn::Convert::FromPacket<::ndn::Data>(packet);
        if (d-> getQCI() > 0) {
          prio = d->getQCI();
        }
        break;
      }
      default:
        NS_LOG_ERROR("Unsupported TLV packet");
    }
  }
  catch (const ::ndn::tlv::Error& e) {
    NS_LOG_ERROR("Unrecognized TLV packet " << e.what());
  }
  return 100 - prio;
}

} // namespace ns3

