#ifndef CCM_CCM_H
#define CCM_CCM_H
#include "gmm/GMM.h"
#include "fd/FD.h"
#include "le/LE.h"
#include <memory>
class CCM
{
  public:
    static std::unique_ptr<CCM> CreateAndInitForTest(const int myId);
    CCM(std::unique_ptr<GMM> &pGmm, std::unique_ptr<FD> &pFd, std::unique_ptr<LE> &pLe);
   ~CCM();
   OSAStatusCode Start();
   void Stop();
   
   void Print() const;
   
  private: 
   OSAStatusCode StartCCMTask();
   OSAStatusCode CCMMethod();
   OSAStatusCode InstanceTaskMethod();
   static OSAStatusCode ClassTaskMethod(void * const pInstance);
   uint32_t  m_periodInMs{500U};
   std::unique_ptr<GMM> m_pGmm;
   std::unique_ptr<FD> m_pFd;
   std::unique_ptr<LE> m_pLe;
   bool m_isRunning{false};
   
};

#endif //CCM_CCM_H
