#ifndef CCM_LR_H
#define CCM_LR_H
#include "LogReplicationMsg.h"
#include "ReplicatedLog.h"
#include "SyncManager.h"
#include "../gmm/GMM.h"
#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include "../csa/ClientMessage.h"
#include "../csa/ClientRequestId.h"
#include <memory>
#include <chrono>

enum class RequestStatus
{
    Committed = 0,
    Aborted    
};

using RequestDoneCallbackType = std::function<void(const ClientRequestId&,const RequestStatus)>;

class LR
{
  public:
    static std::unique_ptr<LR> CreateLR(GMM & gmm,UpcallReplicatedLogCallbackType upcallCb);
    LR(GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver, std::unique_ptr<ReplicatedLog> &pRepLog,  std::unique_ptr<SyncManager>& pSyncMgr);
    ~LR();
    bool ReplicateRequest(ClientMessage &req,RequestDoneCallbackType reqDoneCb);
    void HandleActivityAsFollower();
    void HandleActivityAsLeader();
    void BecameLeaderActivity();
    void NoLongerLeaderActivity();
    void PerformUpcalls(const bool isForce);
    bool IsLogReplicationPending() const {return m_ongoingReqId.IsValid();}
    bool HasLatestEntries();
    
    OSAStatusCode Start() {return m_pSyncMgr->Start();}
    void Stop() {m_pSyncMgr->Stop();}
    void Print() const;
  private:
    void TriggerSync() {m_pSyncMgr->TriggerSync();}
    void CheckIfSyncShouldBeTriggeredAndTriggerIfNeeded();
    //Leader handling
    void HandlePrepareOk(const LogReplicationMsg &lrMsg);
    void HandleMsgAsLeader();
   
    //Follower handling
    void HandlePrepare(const LogReplicationMsg &lrMsg);
    void HandleCommit(const LogReplicationMsg &lrMsg);
    void HandleMsgAsFollower();
    void CheckAndHandleUncommitted();
    void RcvFlush();
    bool RcvMsg(IReceiver &rcv, LogReplicationMsg &msg);
    
   
    
    void CallReqDoneCallback(const RequestStatus status);
    void SendCommitToAll(const LogReplicationMsg & prepareMsgToCommit);
    void SendPrepareOK(const LogReplicationMsg & prepareMsg);
    bool SendToLeader(const LogReplicationMsg & msgToSend);
    
    
    GMM & m_gmm;
    std::vector<std::unique_ptr<ISender>> m_senders; 
    std::unique_ptr<IReceiver> m_pReceiver;
    std::unique_ptr<ReplicatedLog> m_pRepLog;
    std::unique_ptr<SyncManager> m_pSyncMgr;
    ClientRequestId m_ongoingReqId{};
    int m_requestingClientId{0U};
    std::unique_ptr<LogReplicationMsg> m_pOngoingRepMsg{};
    RequestDoneCallbackType m_reqDoneCb{nullptr};
    std::chrono::system_clock::time_point m_lastInSync;
    
    
    
};


#endif //CCM_LR_H
