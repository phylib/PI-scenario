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

#ifndef WFQ_H
#define WFQ_H

#include <queue>
#include <unordered_map>
#include "ns3/packet.h"
#include "ns3/queue.h"

namespace ns3 {

/**
 * \ingroup queue
 *
 * \brief A Weighted Fair packet queue that drops tail-end packets on overflow
 *
 * Weighted Fair Queue (WFQ) with Tail-drop Drop-Policy. WFQ combines fairness
 * for traffic flows with service priorities defined by QoS Flags. Therefore
 * WFQ does not starve low priority traffic but considers QoS flags.
 */
class WFQ : public Queue {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief WFQ Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  WFQ ();

  virtual ~WFQ();

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (WFQ::QueueMode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  WFQ::QueueMode GetMode (void) const;

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

  /**
   * \brief Fetches the priority of the given packet
   */
  uint getPriority(Ptr<const Packet> p) const;

  /**
   * \brief Sums up the priority of all traffic flows
   */
  inline uint getCumulatedPriority() const
  {
    uint sum = 0;
    for (const auto& any : m_queues) {
      Ptr<const Packet> p = any.second.front();
      sum += getPriority(p);
    }
    return sum;
  }

  std::queue<Ptr<Packet> > m_packets; //!< the packets in the queue
  std::unordered_map<std::string, std::deque<Ptr<Packet>>> m_queues;
  std::vector<std::string> m_queue_names;
  std::unordered_map<std::string, double> m_virtualFinish;
  uint32_t m_currentQueue = 0;
  uint32_t m_maxPackets;              //!< max packets in the queue
  uint32_t m_maxBytes;                //!< max bytes in the queue
  uint32_t m_bytesInQueue;            //!< actual bytes in the queue
  QueueMode m_mode;                   //!< queue mode (packets or bytes limited)
};

} // namespace ns3

#endif /* DROPTAIL_H */
