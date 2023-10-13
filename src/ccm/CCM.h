#ifndef CCM_CCM_H
#define CCM_CCM_H
#include "gmm/GMM.h"
#include "fd/FD.h"
#include <memory>
class CCM
{
  public:
    static std::unique_ptr<CCM> CreateAndInitForTest(const int myId);
    CCM(std::unique_ptr<GMM> &pGmm, std::unique_ptr<FD> &pFd);
   ~CCM();
   OSAStatusCode Start();
  private: 
   std::unique_ptr<GMM> m_pGmm;
   std::unique_ptr<FD> m_pFd;
   
};

#endif //CCM_CCM_H
