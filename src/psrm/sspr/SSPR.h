#ifndef SSPR_MAIN_H
#define SSPR_MAIN_H
#include "../../ccm/ICCM.h"
#include "AppStateStoragePair.h"
#include "AppStateStoragePairReg.h"
#include <memory>
#include <string>
#include <mutex>
#include <optional>
class SSPR
{
  public:
    static const unsigned int SERVICE_ID = 12;
    static std::unique_ptr<SSPR> CreateSSPR(std::shared_ptr<ICCM>& pIccm);
    SSPR(std::shared_ptr<ICCM>& pIccm,std::unique_ptr<AppStateStoragePairReg>& pAppStateReg);
    ~SSPR() = default;
    
    bool RegisterWithCCM();
    std::optional<AppStateStoragePair> GetEntry(const unsigned int appId) const {return m_pAppStateReg->GetEntry(appId);}
    std::optional<unsigned int> FindLowestUsedStateStorage(unsigned int &usingAppsCount) const {return m_pAppStateReg->FindLowestUsedStateStorage(usingAppsCount);}
    void GetAllAppIdThatUseThisStorageId(const unsigned int storageId, std::vector<unsigned int>& appIds) const {return m_pAppStateReg->GetAllAppIdThatUseThisStorageId(storageId,appIds);}
    
    void PostStoragePairForReplication(const AppStateStoragePair &pair);
    void Print() const;
  private: 
    std::unique_ptr<AppStateStoragePair> GetStoragePairEntryFromQueue();
    void UpcallMethod(const ClientMessage& commitedMsg);
    std::shared_ptr<ICCM> m_pIccm{nullptr};
    std::unique_ptr<AppStateStoragePairReg> m_pAppStateReg{nullptr};
    mutable std::mutex m_queueMutex;
    std::vector<std::unique_ptr<AppStateStoragePair>> m_appStoragePairQueue{};

    
    
};

#endif //SSPR_MAIN_H
