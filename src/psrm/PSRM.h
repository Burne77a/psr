#ifndef PSRM_H
#define PSRM_H
#include "air/AIR.h"
#include "sr/SR.h"
#include "TaskAbstraction.h"
#include <memory>

class PSRM
{
  public:
    static std::unique_ptr<PSRM> CreatePSRM(std::shared_ptr<ICCM>& pIccm );
    PSRM(std::unique_ptr<AIR>& pAir, std::unique_ptr<SR>& pSr, std::shared_ptr<ICCM>& pIccm);
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
    bool RegisterStorage(const unsigned int storageId, const std::string_view ipAddr, const unsigned int spaceInBytes, const unsigned int bandwidth)
    {
      return m_pSr->RegisterStorage(storageId, ipAddr, spaceInBytes, bandwidth);
    }
    bool DeRegisterStorage(const unsigned int storageId)
    {
     return m_pSr->DeRegisterStorage(storageId);
    }
    
    
  private:
    OSAStatusCode InstanceTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    
    std::unique_ptr<AIR> m_pAir{nullptr};
    std::unique_ptr<SR> m_pSr{nullptr};
    std::shared_ptr<ICCM> m_pIccm{nullptr}; 
   
    bool m_isRunning{false};
    uint32_t  m_periodInMs{500U};
};

#endif //PSRM_H
