#ifndef CCM_CCM_H
#define CCM_CCM_H
#include "gmm/GMM.h"
#include "fd/FD.h"
#include "le/LE.h"
#include <memory>
class CCM : public LE::ILeaderRoleChangeCallbacks,public std::enable_shared_from_this<CCM>
{
  public:
    static std::shared_ptr<CCM> CreateAndInitForTest(const int myId);
    CCM(std::unique_ptr<GMM> &pGmm, std::unique_ptr<FD> &pFd, std::unique_ptr<LE> &pLe);
   ~CCM();
   OSAStatusCode Start();
   void Stop();
   void Print() const;
   
  private: 
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
   std::shared_ptr<LE::ILeaderRoleChangeCallbacks> m_leaderCb{nullptr};
   bool m_isRunning{false};
   unsigned int m_viewNumber{0U};
   unsigned int m_operationNumber{0U};
};

#endif //CCM_CCM_H
