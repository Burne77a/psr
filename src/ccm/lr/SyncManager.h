#ifndef CCM_LR_SYNCMANAGER_H
#define CCM_LR_SYNCMANAGER_H
#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include "../gmm/GMM.h"
#include "SyncMsg.h"
#include "ReplicatedLog.h"
#include <memory>
#include <vector>

class SyncManager
{
  public:
    static std::unique_ptr<SyncManager> CreateSyncManager(GMM & gmm, ReplicatedLog & replicatedLog);
    SyncManager(GMM & gmm, ReplicatedLog & replicatedLog, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver);
    ~SyncManager();
    OSAStatusCode Start();
    void Stop();
    void TriggerSync(){m_isSyncTriggered = true;}
  private:
    void ServeIncomingSyncRequests(const SyncMsg &intiatingMsg);
    void HandleIncomingSyncRequestFromOthers();
    void HandleSyncReqToUpdateThisLog();
    void SendEntriesToRequester(std::string_view requesterIp, std::vector<std::shared_ptr<SyncMsg>> entriesToSend);
    void IssueRequestToSyncToInstanceWithLargestOp();
    void WaitForAndProcessIncomingLogEntries();
    void HandleIncomingSyncMsg(bool &isComplete, std::vector<std::shared_ptr<SyncMsg>> & rcvdSyncMessages, bool &isToAbortDueToFailure,const SyncMsg &incomingMsg);
    void SendRequestForLog(std::string_view ipAddr);
    void RcvFlush();
    bool RcvMsg(IReceiver &rcv, SyncMsg &msg);
    
    OSAStatusCode SyncTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    bool m_isRunning{false};
    bool m_isSyncTriggered{false};
    uint32_t  m_periodInMs{200U};
    GMM & m_gmm;
    ReplicatedLog & m_replicatedLog;
    std::vector<std::unique_ptr<ISender>> m_senders; 
    std::unique_ptr<IReceiver> m_pReceiver;
};


#endif//CCM_LR_SYNCMANAGER_H
