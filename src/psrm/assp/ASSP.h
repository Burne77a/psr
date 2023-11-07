#ifndef ASSP_MAIN_H
#define ASSP_MAIN_H
#include "../../ccm/ICCM.h"
#include "../air/AIR.h"
#include "../sr/SR.h"
#include "../sspr/SSPR.h"
#include <memory>
#include <string>
class ASSP
{
  public:
    static std::unique_ptr<ASSP> CreateASSP(std::shared_ptr<ICCM>& pIccm, AIR &air, SR &sr,SSPR &sspr);
    ASSP(std::shared_ptr<ICCM>& pIccm,const AIR &air, const SR &sr,SSPR &sspr);
    ~ASSP() = default;
    
    void Print() const;
  private: 
    void PairAppWithStorageIfPossible(const unsigned int appId);
    //Called from AIR
    void ApplicationRemoved(const unsigned int appId);
    void ApplicationAdded(const unsigned int appId);
    std::shared_ptr<ICCM> m_pIccm{nullptr};
    const AIR &m_air;
    const SR &m_sr;
    const SSPR &m_sspr;
    static const unsigned int STORAGE_USE_CNT_LIMIT = 2U;
    
    
};

#endif //ASSP_MAIN_H
