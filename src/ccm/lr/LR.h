#ifndef CCM_LR_H
#define CCM_LR_H
#include "LogReplicationMsg.h"
#include "../gmm/GMM.h"
#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include <memory>
class LR
{
  public:
    static std::unique_ptr<LR> CreateLR(GMM & gmm);
    LR(GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver);
    ~LR();
    void HandleActivityAsFollower();
    void HandleActivityAsLeader();
    void BecameLeaderActivity();
    void NoLongerLeaderActivity();
    void Print() const;
  private:
    void RcvFlush();
    bool RcvMsg(IReceiver &rcv, LogReplicationMsg &msg);
    GMM & m_gmm;
    std::vector<std::unique_ptr<ISender>> m_senders; 
    std::unique_ptr<IReceiver> m_pReceiver;
    
};


#endif //CCM_LR_H
