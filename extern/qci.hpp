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

#ifndef QCI_TAG_H
#define QCI_TAG_H

#include "ns3/tag.h"

namespace ns3 {
namespace ndn {

/**
 * \brief Quality of service class identifiers. 
 * Quality of service class identifiers allow priorizing certain serices to ensure QoS requirements.
 * (Values are multiplied by ten to avoid floating point numbers)
 */
enum QCI_CLASSES
{
  QCI_1 = 20, /**< Conversational Voice  */
  QCI_2 = 40, /**< Conversational Video */
  QCI_3 = 30, /**< Real Time Gaming  */
  QCI_4 = 50, /**< Non-Conversational Video (Buffered Streaming) */
  QCI_5 = 10, /**< IMS Signalling */
  QCI_6 = 60, /**< Video (Buffered Streaming) TCP-Based (for example, www, email, chat, ftp, p2p and the like) */
  QCI_7 = 70, /**< Voice, Video (Live Streaming), Interactive Gaming */
  QCI_8 = 80, /**< Video (Buffered Streaming) TCP-Based (for example, www, email, chat, ftp, p2p and the like) */
  QCI_9 = 90, /**< Video (Buffered Streaming) TCP-Based (for example, www, email, chat, ftp, p2p and the like). Typically used as default bearer */
  QCI_65 = 7, /**< Mission Critical user plane Push To Talk voice (e.g., MCPTT) */
  QCI_66 = 20, /**< Non-Mission-Critical user plane Push To Talk voice */
  QCI_69 = 5, /**< Mission Critical delay sensitive signalling (e.g., MC-PTT signalling) */
  QCI_70 = 55 /**< Mission Critical Data (e.g. example services are the same as QCI 6/8/9) */
};

} // namespace ndn
} // namespace ns3

#endif // NDN_FW_HOP_COUNT_TAG_H
