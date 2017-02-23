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

#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue.h"

#include "generic-priority-queue.hpp"

namespace ns3 {

struct PriorityComparator
{
  bool operator()(const std::pair<Ptr<Packet>, uint32_t>& lhs, const std::pair<Ptr<Packet>, uint32_t>& rhs) const
  {
    return lhs.second < rhs.second;
  }
};

/**
 * \ingroup queue
 *
 * \brief A Priority packet queue with tail-drop drop policy.
 *
 * Priority queuing takes into account that some packets are more important
 * than others. Therefore priority queuing first handles the more important
 * packets and then handles lower priority packets.
 * 
 * Packets without priority flag are handled with QCI class 9, which is 
 * the default class in the QCI model.
 */
class PriorityQueue : public Queue {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief DropTailQueue Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  PriorityQueue ();

  virtual ~PriorityQueue();

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (PriorityQueue::QueueMode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  PriorityQueue::QueueMode GetMode (void) const;

  void
  SetDropPolicy (PriorityQueue::DropPolicy policy);

  PriorityQueue::DropPolicy
  GetDropPolicy (void) const;

protected:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  ::PriorityQueue<Ptr<Packet>> m_packets; //!< the packets in the queue
  uint32_t m_maxPackets;              //!< max packets in the queue
  uint32_t m_maxBytes;                //!< max bytes in the queue
  uint32_t m_bytesInQueue;            //!< actual bytes in the queue
  QueueMode m_mode;                   //!< queue mode (packets or bytes limited)
  PriorityQueue::DropPolicy m_dropPolicy;
};

} // namespace ns3

#endif /* DROPTAIL_H */
