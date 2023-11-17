#ifndef CCM_CCM_H
#define CCM_CCM_H
#include "ICCM.h"
#include "gmm/GMM.h"
#include "fd/FD.h"
#include "le/LE.h"
#include "lr/LR.h"
#include "csa/ClientMessage.h"
#include "csa/CSA.h"
#include <memory>
class CCM : public LE::ILeaderRoleChangeCallbacks,public std::enable_shared_from_this<CCM>, public ICCM
{
  public:
    static std::shared_ptr<CCM> CreateAndInitForTest(const int myId);
    CCM(std::unique_ptr<GMM> &pGmm, std::unique_ptr<FD> &pFd, std::unique_ptr<LE> &pLe, std::unique_ptr<LR> &pLr,std::unique_ptr<CSA>& pCsa);
   ~CCM();
   OSAStatusCode Start();
   void Stop();
   int GetMyId(){return m_pGmm->GetMyId();}
   
   
   //ICCM
   bool ReplicateRequest(const ClientMessage & msg) override {return m_pCsa->ReplicateRequest(msg);}
   bool RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb) override {return m_pCsa->RegisterService(serviceId, upcallCb);}
   unsigned int GetMyId() const override{return m_pGmm->GetMyId();}
   std::string GetIp(const unsigned int id) const override {return m_pGmm->GetIp((int)id);}
   ClientRequestId CreateUniqueId() override {return m_pCsa->CreateUniqueId();}
   bool IsFullySyncLeader() override {return ((m_pLe->GetCurrentStateValue()==StateBaseLE::StateValue::Leader) && (m_pLr->HasLatestEntries()));}
   bool IsThereALeader() const override {return m_pGmm->IsLeaderAvailable();}
   void ReqDoneCbFromLr(const ClientRequestId& reqId,const RequestStatus reqSts);
   void ForceUpcalls(){m_pLr->PerformUpcalls(true);}
   void Print() const; 
  private: 
   void MakeUpcalls();
   void HandleIncomingClientRequestToLeader();
   bool IsUpdatedWithLatestEntries();
   
   //ILeaderRoleChangeCallbacks
   void EnteredLeaderRole() override; 
   void LeftLeaderRole() override;
   OSAStatusCode StartCCMTask();
   OSAStatusCode InstanceTaskMethod();
   static OSAStatusCode ClassTaskMethod(void * const pInstance);
   uint32_t  m_periodInMs{500U};
   std::unique_ptr<GMM> m_pGmm;
   std::unique_ptr<FD> m_pFd;
   std::unique_ptr<LE> m_pLe;
   std::unique_ptr<LR> m_pLr;
   std::unique_ptr<CSA> m_pCsa;
   std::shared_ptr<LE::ILeaderRoleChangeCallbacks> m_leaderCb{nullptr};
   
   ClientRequestId m_currentReqId{};   
   bool m_isRunning{false};
};

#endif //CCM_CCM_H
