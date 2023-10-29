#ifndef CCM_CCM_H
#define CCM_CCM_H
#include "gmm/GMM.h"
#include "fd/FD.h"
#include "le/LE.h"
#include "lr/LR.h"
#include "csa/ClientMessage.h"
#include <memory>
class CCM : public LE::ILeaderRoleChangeCallbacks,public std::enable_shared_from_this<CCM>
{
  public:
    static std::shared_ptr<CCM> CreateAndInitForTest(const int myId);
    CCM(std::unique_ptr<GMM> &pGmm, std::unique_ptr<FD> &pFd, std::unique_ptr<LE> &pLe, std::unique_ptr<LR> &pLr);
   ~CCM();
   OSAStatusCode Start();
   void Stop();
   int GetMyId(){return m_pGmm->GetMyId();}
   static std::string_view GetLeaderIp();
   void Print() const;
  
   
  private: 
   void MakeUpcalls();
   
   //ILeaderRoleChangeCallbacks
   void EnteredLeaderRole() override; 
   void LeftLeaderRole() override;
   OSAStatusCode StartCCMTask();
   OSAStatusCode CCMMethod();
   OSAStatusCode InstanceTaskMethod();
   static OSAStatusCode ClassTaskMethod(void * const pInstance);
   uint32_t  m_periodInMs{500U};
   std::unique_ptr<GMM> m_pGmm;
   std::unique_ptr<FD> m_pFd;
   std::unique_ptr<LE> m_pLe;
   std::unique_ptr<LR> m_pLr;
   std::shared_ptr<LE::ILeaderRoleChangeCallbacks> m_leaderCb{nullptr};
   bool m_isRunning{false};
   unsigned int m_viewNumber{0U};
   unsigned int m_operationNumber{0U};
};

#endif //CCM_CCM_H
