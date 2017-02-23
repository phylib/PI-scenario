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

#ifndef FAIRQUEUE_H
#define FAIRQUEUE_H

#include <queue>
#include <unordered_map>
#include "ns3/packet.h"
#include "ns3/queue.h"

namespace ns3 {

/**
 * \ingroup queue
 *
 * \brief A Fair packet queue with tail-drop drop policy.
 *
 * A Fair packet queue with tail-drop drop policy. Fair Queuing takes into account
 * that some traffic flows produces more packets than others. Fair queuing allows
 * the same amount of packets for all traffic flows.
 *
 * To separate traffic flows in NDN the first two parts of the names are used.
 */
class FairQueue : public Queue {
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
  FairQueue ();

  virtual ~FairQueue();

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (FairQueue::QueueMode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  FairQueue::QueueMode GetMode (void) const;

protected:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  /**
   * \brief Returns the number of packets in the queue
   */
  uint countPackets(void) const; 

  /**
   * \brief Checks if queue for the given name exists
   *
   * @param name Name to query
   */
  bool hasName(std::string name) const;

  /**
   * \brief Updates virtual finishing time
   * 
   * Calculates the new virtual finishing time for the given queue considering the given packet
   */
  void updateTime(Ptr<const Packet> packet, std::string queueName);

  /**
   * \brief Select the next queue to dequeu
   */
  uint selectQueue() const;

  std::queue<Ptr<Packet> > m_packets; //!< the packets in the queue
  std::unordered_map<std::string, std::deque<Ptr<Packet>>> m_queues; //!< map containing queues for all traffic flows
  std::vector<std::string> m_queue_names; //!< Names of the queues
  std::unordered_map<std::string, double> m_virtualFinish; //!< Virtual finishing times for all queues
  uint32_t m_currentQueue = 0;
  uint32_t m_maxPackets;              //!< max packets in the queue
  uint32_t m_maxBytes;                //!< max bytes in the queue
  uint32_t m_bytesInQueue;            //!< actual bytes in the queue
  QueueMode m_mode;                   //!< queue mode (packets or bytes limited)
};

} // namespace ns3

#endif /* DROPTAIL_H */
