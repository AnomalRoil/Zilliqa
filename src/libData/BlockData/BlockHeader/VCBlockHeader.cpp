/*
 * Copyright (c) 2018 Zilliqa
 * This source code is being disclosed to you solely for the purpose of your
 * participation in testing Zilliqa. You may view, compile and run the code for
 * that purpose and pursuant to the protocols and algorithms that are programmed
 * into, and intended by, the code. You may not do anything else with the code
 * without express permission from Zilliqa Research Pte. Ltd., including
 * modifying or publishing the code (or any part of it), and developing or
 * forming another public or private blockchain network. This source code is
 * provided 'as is' and no warranties are given as to title or non-infringement,
 * merchantability or fitness for purpose and, to the extent permitted by law,
 * all liability for your use of the code is disclaimed. Some programs in this
 * code are governed by the GNU General Public License v3.0 (available at
 * https://www.gnu.org/licenses/gpl-3.0.en.html) ('GPLv3'). The programs that
 * are governed by GPLv3.0 are those programs that are located in the folders
 * src/depends and tests/depends and which include a reference to GPLv3 in their
 * program files.
 */

#include "VCBlockHeader.h"
#include "libMessage/Messenger.h"
#include "libUtils/Logger.h"

using namespace std;
using namespace boost::multiprecision;

VCBlockHeader::VCBlockHeader() : m_CandidateLeaderIndex(1) {}

VCBlockHeader::VCBlockHeader(const vector<unsigned char>& src,
                             unsigned int offset) {
  if (!Deserialize(src, offset)) {
    LOG_GENERAL(INFO, "Error. We failed to initialize VCBlockHeader.");
  }
}

VCBlockHeader::VCBlockHeader(const uint64_t& vieWChangeDSEpochNo,
                             const uint64_t& viewChangeEpochNo,
                             const unsigned char viewChangeState,
                             const uint32_t expectedCandidateLeaderIndex,
                             const Peer& candidateLeaderNetworkInfo,
                             const PubKey& candidateLeaderPubKey,
                             const uint32_t vcCounter,
                             const boost::multiprecision::uint256_t& timestamp,
                             const CommitteeHash& committeeHash)
    : BlockHeaderBase(committeeHash),
      m_VieWChangeDSEpochNo(vieWChangeDSEpochNo),
      m_VieWChangeEpochNo(viewChangeEpochNo),
      m_ViewChangeState(viewChangeState),
      m_CandidateLeaderIndex(expectedCandidateLeaderIndex),
      m_CandidateLeaderNetworkInfo(candidateLeaderNetworkInfo),
      m_CandidateLeaderPubKey(candidateLeaderPubKey),
      m_VCCounter(vcCounter),
      m_Timestamp(timestamp) {}

bool VCBlockHeader::Serialize(vector<unsigned char>& dst,
                              unsigned int offset) const {
  if (!Messenger::SetVCBlockHeader(dst, offset, *this)) {
    LOG_GENERAL(WARNING, "Messenger::SetVCBlockHeader failed.");
    return false;
  }

  return true;
}

bool VCBlockHeader::Deserialize(const vector<unsigned char>& src,
                                unsigned int offset) {
  if (!Messenger::GetVCBlockHeader(src, offset, *this)) {
    LOG_GENERAL(WARNING, "Messenger::GetVCBlockHeader failed.");
    return false;
  }

  return true;
}

const uint64_t& VCBlockHeader::GetVieWChangeDSEpochNo() const {
  return m_VieWChangeDSEpochNo;
}

const uint64_t& VCBlockHeader::GetViewChangeEpochNo() const {
  return m_VieWChangeEpochNo;
}

unsigned char VCBlockHeader::GetViewChangeState() const {
  return m_ViewChangeState;
}

uint32_t VCBlockHeader::GetCandidateLeaderIndex() const {
  return m_CandidateLeaderIndex;
}

const Peer& VCBlockHeader::GetCandidateLeaderNetworkInfo() const {
  return m_CandidateLeaderNetworkInfo;
}

const PubKey& VCBlockHeader::GetCandidateLeaderPubKey() const {
  return m_CandidateLeaderPubKey;
}

uint32_t VCBlockHeader::GetViewChangeCounter() const { return m_VCCounter; }

const boost::multiprecision::uint256_t& VCBlockHeader::GetTimeStamp() const {
  return m_Timestamp;
}

bool VCBlockHeader::operator==(const VCBlockHeader& header) const {
  return (
      (m_VieWChangeDSEpochNo == header.m_VieWChangeDSEpochNo) &&
      (m_VieWChangeEpochNo == header.m_VieWChangeEpochNo) &&
      (m_ViewChangeState == header.m_ViewChangeState) &&
      (m_CandidateLeaderIndex == header.m_CandidateLeaderIndex) &&
      (m_CandidateLeaderNetworkInfo == header.m_CandidateLeaderNetworkInfo) &&
      (m_CandidateLeaderPubKey == header.m_CandidateLeaderPubKey) &&
      (m_VCCounter == header.m_VCCounter) &&
      (m_Timestamp == header.m_Timestamp));
}

bool VCBlockHeader::operator<(const VCBlockHeader& header) const {
  // To compare, first they must be of identical epochno and state
  return (m_VieWChangeDSEpochNo == header.m_VieWChangeDSEpochNo) &&
         (m_VieWChangeEpochNo == header.m_VieWChangeEpochNo) &&
         (m_ViewChangeState == header.m_ViewChangeState) &&
         (m_Timestamp == header.m_Timestamp) &&
         (m_VCCounter < header.m_VCCounter);
}

bool VCBlockHeader::operator>(const VCBlockHeader& header) const {
  // To compare, first they must be of identical epochno and state
  return (m_VieWChangeDSEpochNo == header.m_VieWChangeDSEpochNo) &&
         (m_VieWChangeEpochNo == header.m_VieWChangeEpochNo) &&
         (m_ViewChangeState == header.m_ViewChangeState) &&
         (m_Timestamp == header.m_Timestamp) &&
         (m_VCCounter > header.m_VCCounter);
}
