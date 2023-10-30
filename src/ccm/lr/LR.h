#ifndef CCM_LR_H
#define CCM_LR_H
#include "LogReplicationMsg.h"
#include "ReplicatedLog.h"
#include "../gmm/GMM.h"
#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include "../csa/ClientMessage.h"
#include "../csa/ClientRequestId.h"
#include <memory>
class LR
{
  public:
    static std::unique_ptr<LR> CreateLR(GMM & gmm);
    LR(GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver, std::unique_ptr<ReplicatedLog> &pRepLog);
    ~LR();
    bool ReplicateRequest(ClientMessage &req);
    void HandleActivityAsFollower();
    void HandleActivityAsLeader();
    void BecameLeaderActivity();
    void NoLongerLeaderActivity();
    void PerformUpcalls();
    bool IsLogReplicationPending() const {return m_ongoingReqId.IsValid();}
    void Print() const;
  private:
    void HandlePrepare(const LogReplicationMsg &lrMsg);
    void HandleCommit(const LogReplicationMsg &lrMsg);
    void HandleMsgAsFollower();
    void RcvFlush();
    bool RcvMsg(IReceiver &rcv, LogReplicationMsg &msg);
    void SendPrepareOK(const LogReplicationMsg & prepareMsg);
    bool SendToLeader(const LogReplicationMsg & msgToSend);
    GMM & m_gmm;
    std::vector<std::unique_ptr<ISender>> m_senders; 
    std::unique_ptr<IReceiver> m_pReceiver;
    std::unique_ptr<ReplicatedLog> m_pRepLog;
    ClientRequestId m_ongoingReqId{};
    int m_requestingClientId{0U};
    std::unique_ptr<LogReplicationMsg> m_pOngoingRepMsg{};
    
    
    
};


#endif //CCM_LR_H
