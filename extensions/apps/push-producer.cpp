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

#include "push-producer.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-ns3.hpp"
#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

NS_LOG_COMPONENT_DEFINE("ndn.PushProducer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(PushProducer);

TypeId
PushProducer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::PushProducer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<PushProducer>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&PushProducer::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&PushProducer::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&PushProducer::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&PushProducer::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&PushProducer::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("Frequency", "Frequency of data packets", StringValue("10.0"),
                    MakeDoubleAccessor(&PushProducer::m_frequency), MakeDoubleChecker<double>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&PushProducer::m_keyLocator), MakeNameChecker())
      .AddAttribute("QCI",
                    "QoS class Identifier (QCI)",
                    UintegerValue(0), MakeUintegerAccessor(&PushProducer::m_qci), MakeUintegerChecker<uint32_t>());
  return tid;
}

PushProducer::PushProducer()
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
PushProducer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
PushProducer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
PushProducer::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_DEBUG("Received Interest " << interest->getName());
  m_pushResRequired = true;

  // If producer already produces data, do nothing
  if (m_producing)
    return;

  
  PushProducer::SendPacket();
  m_producing = true;
}

// void
// PushProducer::SendPacket()
// {
//   PushProducer::SendPacket(false);
// }

void
PushProducer::SendPacket() {

  Name dataName(m_prefix);
  dataName.appendSequenceNumber(m_seq);

  // Only increase sequence number if it is a regular packet
  m_seq++; 
  
  // dataName.append(m_postfix);
  // dataName.appendVersion();

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setPush(true);  
  
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));
  if (m_qci != 0) {
    data->setQCI(m_qci);
  }

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::nonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);

  PushProducer::ScheduleNextPacket();
}

void
PushProducer::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";
  // if (m_firstTime) {
  //   m_sendEvent = Simulator::Schedule(Seconds(0.0), &PushConsumer::SendPacket, this);
  //   m_firstTime = false;
  // }
  // else if (!m_sendEvent.IsRunning())
  //  m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
  //                                                    : Seconds(m_random->GetValue()),
  //                                                    &PushConsumer::SendPacket, this);
  Simulator::Schedule(Seconds(1.0 / m_frequency), &PushProducer::SendPacket, this);                                    
}

} // namespace ndn
} // namespace ns3
