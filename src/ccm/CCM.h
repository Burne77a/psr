#ifndef CCM_CCM_H
#define CCM_CCM_H
#include "gmm\GMM.h"
class CCM
{
  public:
    CCM();
   ~CCM();
   void InitForTest();
  private: 
   GMM m_gmm{};
   
};

#endif //CCM_CCM_H
