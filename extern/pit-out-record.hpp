/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_TABLE_PIT_OUT_RECORD_HPP
#define NFD_DAEMON_TABLE_PIT_OUT_RECORD_HPP

#include "pit-face-record.hpp"

namespace nfd {
namespace pit {

/** \brief contains information about an Interest toward an outgoing face
 */
class OutRecord : public FaceRecord
{
public:
  explicit
  OutRecord(shared_ptr<Face> face);

  /** \return last NACK returned by \p getFace()
   *
   *  A nullptr return value means the Interest is still pending or has timed out.
   *  A non-null return value means the last outgoing Interest has been NACKed.
   */
  const lp::NackHeader*
  getIncomingNack() const
  {
    return m_incomingNack.get();
  }

  /** \brief sets a NACK received from \p getFace()
   *  \return whether incoming NACK is accepted
   *
   *  This is invoked in incoming NACK pipeline.
   *  An incoming NACK is accepted if its Nonce matches \p getLastNonce().
   *  If accepted, \p nack.getHeader() will be copied,
   *  and any pointer previously returned by \p .getIncomingNack() .
   */
  bool
  setIncomingNack(const lp::Nack& nack);

  /** \brief clears last NACK
   *
   *  This is invoked in outgoing Interest pipeline.
   *  This invalidates any pointer previously returned by \p .getIncomingNack() .
   */
  void
  clearIncomingNack()
  {
    m_incomingNack.reset();
  }

  void
  receivedData()
  {
    m_receivedDatas++;
  }

  void
  setReceivedDatas(uint receivedDatas)
  {
    m_receivedDatas = receivedDatas;
  }

  uint getReceivedDatas () const
  {
    return m_receivedDatas;
  }

  void
  setExpectedDatas(uint expectedDatas)
  {
    m_expectedDatas = expectedDatas;
  }

  uint getExpectedDatas () const
  {
    return m_expectedDatas;
  }

private:
  unique_ptr<lp::NackHeader> m_incomingNack;

protected:
  uint m_receivedDatas = 0;
  uint m_expectedDatas = 0;
};

} // namespace pit
} // namespace nfd

#endif // NFD_DAEMON_TABLE_PIT_IN_RECORD_HPP
