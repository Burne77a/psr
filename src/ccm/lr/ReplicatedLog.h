#ifndef CCM_LR_REPLICATEDLOG_H
#define CCM_LR_REPLICATEDLOG_H
#include "LogEntry.h"
#include "LogReplicationMsg.h"
#include <vector>
#include <memory>
#include <mutex>
class ReplicatedLog
{
   public:
    ReplicatedLog();
    ~ReplicatedLog() = default;    
    bool ArePreviousEntriesInLog(const unsigned int opNumber, const unsigned int viewNumber);
    bool ArePreviousEntriesInLog(const LogReplicationMsg &msg){return ArePreviousEntriesInLog(msg.GetOpNumber(),msg.GetViewNumber());}
    bool IsEntryAlreadyExisting(const unsigned int opNumber, const unsigned int viewNumber);
    bool IsEntryAlreadyExisting(const LogReplicationMsg &msg) {return IsEntryAlreadyExisting(msg.GetOpNumber(),msg.GetViewNumber());}
    bool AddEntryToLogIfNotAlreadyIn(const LogReplicationMsg &msgToMakeEntryFrom);
    
    void Print() const;
  private:
    bool AddOrOverwriteDependingOnView(const LogReplicationMsg &msg);
    std::unique_ptr<LogEntry> & GetEntryWithOpNumber(const unsigned int opNumber);
    std::unique_ptr<LogEntry> & GetLastValidEntry();
    std::vector<std::unique_ptr<LogEntry>> m_logEntries{};
    mutable std::mutex m_mutex;
};

#endif //CCM_LR_REPLICATEDLOG_H
