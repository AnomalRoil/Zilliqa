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

#include <vector>

#include "Synchronizer.h"
#include "common/Constants.h"
#include "common/Messages.h"
#include "libCrypto/Schnorr.h"
#include "libData/AccountData/Transaction.h"
#include "libData/BlockData/Block.h"
#include "libPersistence/BlockStorage.h"
#include "libUtils/TimeUtils.h"

using namespace std;
using namespace boost::multiprecision;

DSBlock Synchronizer::ConstructGenesisDSBlock() {
  BlockHash prevHash;

  for (unsigned int i = 0; i < prevHash.asArray().size(); i++) {
    prevHash.asArray().at(i) = i + 1;
  }

  vector<unsigned char> tmpprivkey = DataConversion::HexStrToUint8Vec(
      "BCCDF94ACEC5B6F1A2D96BDDC6CBE22F3C6DFD89FD791F18B722080A908253CD");
  vector<unsigned char> tmppubkey = DataConversion::HexStrToUint8Vec(
      "02AAE728127EB5A30B07D798D5236251808AD2C8BA3F18B230449D0C938969B552");
  // FIXME: Handle exceptions.
  PrivKey privKey(tmpprivkey, 0);
  PubKey pubKey(tmppubkey, 0);
  std::pair<PrivKey, PubKey> keypair = make_pair(privKey, pubKey);
  uint64_t genesisBlockNumer = 0;
  uint256_t genesisTimestamp = 0;
  std::map<PubKey, Peer> powDSWinners;

  // FIXME: Handle exceptions.
  DSBlock dsBlock(
      DSBlockHeader(DS_POW_DIFFICULTY, POW_DIFFICULTY, prevHash, keypair.second,
                    genesisBlockNumer, genesisTimestamp, SWInfo(), powDSWinners,
                    DSBlockHashSet(), CommitteeHash()),
      CoSignatures());
  dsBlock.SetBlockHash(dsBlock.GetHeader().GetMyHash());
  return dsBlock;
}

bool Synchronizer::AddGenesisDSBlockToBlockChain(DSBlockChain& dsBlockChain,
                                                 const DSBlock& dsBlock) {
  dsBlockChain.AddBlock(dsBlock);

  // Store DS Block to disk
  vector<unsigned char> serializedDSBlock;
  dsBlock.Serialize(serializedDSBlock, 0);
  BlockStorage::GetBlockStorage().PutDSBlock(dsBlock.GetHeader().GetBlockNum(),
                                             serializedDSBlock);

  return true;
}

bool Synchronizer::InitializeGenesisDSBlock(DSBlockChain& dsBlockChain) {
  DSBlock dsBlock = ConstructGenesisDSBlock();
  AddGenesisDSBlockToBlockChain(dsBlockChain, dsBlock);

  return true;
}

TxBlock Synchronizer::ConstructGenesisTxBlock() {
  vector<unsigned char> tmpprivkey = DataConversion::HexStrToUint8Vec(
      "BCCDF94ACEC5B6F1A2D96BDDC6CBE22F3C6DFD89FD791F18B722080A908253CD");
  vector<unsigned char> tmppubkey = DataConversion::HexStrToUint8Vec(
      "02AAE728127EB5A30B07D798D5236251808AD2C8BA3F18B230449D0C938969B552");
  // FIXME: Handle exceptions.
  PrivKey privKey(tmpprivkey, 0);
  PubKey pubKey(tmppubkey, 0);

  std::pair<PrivKey, PubKey> keypair = make_pair(privKey, pubKey);

  TxBlock txBlock(
      TxBlockHeader(TXBLOCKTYPE::FINAL, BLOCKVERSION::VERSION1, 1, 1,
                    BlockHash(), 0, 151384616955606, TxnHash(), StateHash(),
                    StateHash(), StateHash(), TxnHash(), 0, 5, keypair.second,
                    0, BlockHash(), CommitteeHash()),
      vector<bool>(1), vector<MicroBlockHashSet>(5), vector<uint32_t>(5),
      CoSignatures());
  txBlock.SetBlockHash(txBlock.GetHeader().GetMyHash());
  return txBlock;
}

bool Synchronizer::AddGenesisTxBlockToBlockChain(TxBlockChain& txBlockChain,
                                                 const TxBlock& txBlock) {
  txBlockChain.AddBlock(txBlock);

  // Store Tx Block to disk
  vector<unsigned char> serializedTxBlock;
  txBlock.Serialize(serializedTxBlock, 0);
  BlockStorage::GetBlockStorage().PutTxBlock(txBlock.GetHeader().GetBlockNum(),
                                             serializedTxBlock);

  return true;
}

bool Synchronizer::InitializeGenesisTxBlock(TxBlockChain& txBlockChain) {
  TxBlock txBlock = ConstructGenesisTxBlock();
  AddGenesisTxBlockToBlockChain(txBlockChain, txBlock);

  return true;
}

bool Synchronizer::InitializeGenesisBlocks(DSBlockChain& dsBlockChain,
                                           TxBlockChain& txBlockChain) {
  LOG_MARKER();
  InitializeGenesisDSBlock(dsBlockChain);
  InitializeGenesisTxBlock(txBlockChain);

  return true;
}

bool Synchronizer::FetchDSInfo(Lookup* lookup) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "Synchronizer::FetchDSInfo not expected to be called from "
                "LookUp node.");
    return true;
  }

  lookup->GetDSInfoFromLookupNodes();
  // lookup->GetDSInfoFromSeedNodes();
  return true;
}

bool Synchronizer::FetchLatestDSBlocks(Lookup* lookup,
                                       uint64_t currentBlockChainSize) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "Synchronizer::FetchLatestDSBlocks not expected to be "
                "called from LookUp node.");
    return true;
  }

  lookup->GetDSBlockFromLookupNodes(currentBlockChainSize, 0);
  return true;
}

bool Synchronizer::FetchLatestTxBlocks(Lookup* lookup,
                                       uint64_t currentBlockChainSize) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "Synchronizer::FetchLatestTxBlocks not expected to be "
                "called from LookUp node.");
    return true;
  }

  lookup->GetTxBlockFromLookupNodes(currentBlockChainSize, 0);
  return true;
}

bool Synchronizer::FetchLatestState(Lookup* lookup) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "Synchronizer::FetchLatestState not expected to be called "
                "from LookUp node.");
    return true;
  }

  lookup->GetStateFromLookupNodes();
  return true;
}

bool Synchronizer::AttemptPoW(Lookup* lookup) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "Synchronizer::AttemptPoW not expected to be called from "
                "LookUp node.");
    return true;
  }

  if (lookup->InitMining()) {
    LOG_GENERAL(INFO, "new node attempted pow");
    return true;
  } else {
    LOG_GENERAL(INFO, "new node did not attempt pow")
    return false;
  }
}

bool Synchronizer::FetchOfflineLookups(Lookup* lookup) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "Synchronizer::FetchOfflineLookups not expected to be "
                "called from LookUp node.");
    return true;
  }

  lookup->GetOfflineLookupNodes();
  return true;
}
