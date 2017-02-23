/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "push-consumer.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "ns3/ndnSIM/ndn-cxx/encoding/qci.hpp"

#include <cstdlib>

NS_LOG_COMPONENT_DEFINE("ndn.PushConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(PushConsumer);

TypeId
PushConsumer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::PushConsumer")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<PushConsumer>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&PushConsumer::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&PushConsumer::SetRandomize, &PushConsumer::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&PushConsumer::m_seqMax), MakeIntegerChecker<uint32_t>())
      .AddAttribute("DataFrequency", "Frequency of data packets", StringValue("100"),
                    MakeDoubleAccessor(&PushConsumer::m_dataFrequency), MakeDoubleChecker<double>());

    ;

  return tid;
}

PushConsumer::PushConsumer()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
}

PushConsumer::~PushConsumer()
{
}

void
PushConsumer::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  Consumer::OnData(data); // tracing inside

/*
  NS_LOG_FUNCTION(this << data);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

  int hopCount = 0;
  auto ns3PacketTag = data->getTag<Ns3PacketTag>();
  if (ns3PacketTag != nullptr) { // e.g., packet came from local node's cache
    FwHopCountTag hopCountTag;
    if (ns3PacketTag->getPacket()->PeekPacketTag(hopCountTag)) {
      hopCount = hopCountTag.Get();
      NS_LOG_DEBUG("Hop count: " << hopCount);
    }
  }*/

  /*
  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);

  m_rtt->AckSeq(SequenceNumber32(seq)); */
}

void
PushConsumer::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";
  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &PushConsumer::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &PushConsumer::SendPacket, this);
}

void
PushConsumer::SetRandomize(const std::string& value)
{
  if (value == "uniform") {
    m_random = CreateObject<UniformRandomVariable>();
    m_random->SetAttribute("Min", DoubleValue(0.0));
    m_random->SetAttribute("Max", DoubleValue(2 * 1.0 / m_frequency));
  }
  else if (value == "exponential") {
    m_random = CreateObject<ExponentialRandomVariable>();
    m_random->SetAttribute("Mean", DoubleValue(1.0 / m_frequency));
    m_random->SetAttribute("Bound", DoubleValue(50 * 1.0 / m_frequency));
  }
  else
    m_random = 0;

  m_randomType = value;
}

std::string
PushConsumer::GetRandomize() const
{
  return m_randomType;
}

void
PushConsumer::SendPacket()
{
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();

  // uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  // while (m_retxSeqs.size()) {
  //   seq = *m_retxSeqs.begin();
  //   m_retxSeqs.erase(m_retxSeqs.begin());
  //   break;
  // }

  // if (seq == std::numeric_limits<uint32_t>::max()) {
  //   if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
  //     if (m_seq >= m_seqMax) {
  //       return; // we are totally done
  //     }
  //   }

  //   seq = m_seq++;
  // }

  //
  shared_ptr<Name> name = make_shared<Name>(m_interestName);
  //nameWithSequence->appendSequenceNumber(seq);
  //

  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  time::milliseconds creationTime(Now().GetMilliSeconds());
  interest->setNonce(creationTime.count() + (rand() % 1000));
  interest->setName(*name);
  interest->setPush(true);
  interest->setQCI(ndn::QCI_CLASSES::QCI_1);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  /*if (m_dataFrequency != 0) {
    uint32_t expectedPackets = (uint32_t)(m_dataFrequency * m_interestLifeTime.GetMilliSeconds());
    std::cout << "Interest lifetime " << m_interestLifeTime.GetMilliSeconds() << std::endl;
    interest->setExpectedData(expectedPackets);
  }*/

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << interest->toUri());

  //WillSendOutInterest(seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();
}

} // namespace ndn
} // namespace ns3
