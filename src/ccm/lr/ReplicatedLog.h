#ifndef CCM_LR_REPLICATEDLOG_H
#define CCM_LR_REPLICATEDLOG_H
#include "LogEntry.h"
#include "LogReplicationMsg.h"
#include "SyncMsg.h"
#include <vector>
#include <memory>
#include <mutex>

using UpcallReplicatedLogCallbackType = std::function<void(std::shared_ptr<ClientMessage>)>;
class ReplicatedLog
{
   public:
    ReplicatedLog(UpcallReplicatedLogCallbackType upcallCb);
    ~ReplicatedLog() = default;    
    bool ArePreviousEntriesInLog(const unsigned int opNumber, const unsigned int viewNumber);
    bool ArePreviousEntriesInLog(const LogReplicationMsg &msg){return ArePreviousEntriesInLog(msg.GetOpNumber(),msg.GetViewNumber());}
    bool IsEntryAlreadyExisting(const unsigned int opNumber, const unsigned int viewNumber);
    bool IsEntryAlreadyExisting(const LogReplicationMsg &msg) {return IsEntryAlreadyExisting(msg.GetOpNumber(),msg.GetViewNumber());}
    bool ArePreviousEntriesCommitted(const unsigned int opNumber);
    bool AddEntryToLogIfNotAlreadyIn(const LogReplicationMsg &msgToMakeEntryFrom);
    bool CommitEntryIfPresent(const LogReplicationMsg &msgToCommitCorespondingEntryFor);
    void PerformUpcalls(const bool isForce);
    unsigned int GetLatestEntryOpNumber();
    bool GetLogEntriesAsSyncMsgVector(const int myId, std::vector<std::shared_ptr<SyncMsg>> &vectorToPopulate);
    void PopulateFromVector(std::vector<std::shared_ptr<SyncMsg>> &vectorToPopulateFrom);
    void CommittAllEarlierEntries(const unsigned int opNumber);
    unsigned int GetHighestCommittedOpNumber();
    void Print() const;
  private:
    bool AddOrOverwriteDependingOnView(const LogReplicationMsg &msg);
    std::unique_ptr<LogEntry> & GetEntryWithOpNumber(const unsigned int opNumber);
    std::unique_ptr<LogEntry> & GetLastValidEntry();
    std::vector<std::unique_ptr<LogEntry>> m_logEntries{};
    mutable std::mutex m_mutex;
    UpcallReplicatedLogCallbackType m_upcallCb{};
};

#endif //CCM_LR_REPLICATEDLOG_H
