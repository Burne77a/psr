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
    void HandleActivity();
    void Print() const;
  private: 
    void CheckForUnPairedAppAndTryToPair();
    bool PairWithUnUsedIfItExist(const unsigned int appId);
    bool PairWithUsed(const unsigned int appId);
    bool PairAppWithStorageIfPossible(const unsigned int appId);
    void RemoveAppStoragePair(const unsigned int appId);
    //Called from AIR
    void ApplicationRemoved(const unsigned int appId);
    void ApplicationAdded(const unsigned int appId);
    //Called from SR
    void StorageRemoved(const unsigned int storageId);
    void StorageAdded(const unsigned int storageId);
    
    bool IsStorageOnDifferentNode(const unsigned int appId,StorageInfo &storageInfo);
    std::shared_ptr<ICCM> m_pIccm{nullptr};
    const AIR &m_air;
    const SR &m_sr;
    SSPR &m_sspr;
    std::chrono::system_clock::time_point m_lastUnPairCheck;
    static const unsigned int STORAGE_USE_CNT_LIMIT = 2U;
    
    
};

#endif //ASSP_MAIN_H
