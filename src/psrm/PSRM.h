#ifndef PSRM_H
#define PSRM_H
#include "air/AIR.h"
#include "TaskAbstraction.h"
#include <memory>

class PSRM
{
  public:
    static std::unique_ptr<PSRM> CreatePSRM(std::shared_ptr<ICCM>& pIccm );
    PSRM(std::unique_ptr<AIR>& pAir, std::shared_ptr<ICCM>& pIccm);
    OSAStatusCode Start();
    void Stop();
    void Print() const;
    bool RegisterApplication(const unsigned int appId, const unsigned int primaryInfo, 
        const unsigned int bytes, const unsigned int periodInMs)
    {
      return m_pAir->RegisterApplication(appId, primaryInfo, bytes, periodInMs);
    }
    bool DeRegisterApplication(const unsigned int appId)
    {
     return m_pAir->DeRegisterApplication(appId);
    }
  private:
    OSAStatusCode InstanceTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    
    std::unique_ptr<AIR> m_pAir{nullptr};
    std::shared_ptr<ICCM> m_pIccm{nullptr}; 
   
    bool m_isRunning{false};
    uint32_t  m_periodInMs{500U};
};

#endif //PSRM_H
