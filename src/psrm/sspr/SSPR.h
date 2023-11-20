#ifndef SSPR_MAIN_H
#define SSPR_MAIN_H
#include "../../ccm/ICCM.h"
#include "AppStateStoragePair.h"
#include "AppStateStoragePairReg.h"
#include "AppStateStoragePairMsg.h"
#include <memory>
#include <string>
#include <mutex>
#include <optional>

using StateStorageChangeCallbackType = std::function<void(const AppStateStoragePair &affectedPair, bool isRemoved)>;

class SSPR
{
  public:
    static const unsigned int SERVICE_ID = 12;
    static std::unique_ptr<SSPR> CreateSSPR(std::shared_ptr<ICCM>& pIccm);
    SSPR(std::shared_ptr<ICCM>& pIccm,std::unique_ptr<AppStateStoragePairReg>& pAppStateReg);
    ~SSPR() = default;
    bool RegisterWithCCM();
    
    void HandleActivity();
    
    std::optional<AppStateStoragePair> GetEntry(const unsigned int appId) const {return m_pAppStateReg->GetEntry(appId);}
    std::optional<unsigned int> FindLowestUsedStateStorageOnDifferentNode(unsigned int &usingAppsCount, const unsigned int nodeId) const {return m_pAppStateReg->FindLowestUsedStateStorageOnDifferentNode(usingAppsCount,nodeId);}
    std::optional<unsigned int> FindLowestUsedStateStorage(unsigned int &usingAppsCount) const {return m_pAppStateReg->FindLowestUsedStateStorage(usingAppsCount);}
    void GetAllAppIdThatUseThisStorageId(const unsigned int storageId, std::vector<unsigned int>& appIds) const {return m_pAppStateReg->GetAllAppIdThatUseThisStorageId(storageId,appIds);}
    
    void InstallChangeCallback(StateStorageChangeCallbackType cb);
    
    void PostStoragePairForReplication(const AppStateStoragePair &pair);
    void Print() const;
  private: 
    void CallCallback(const AppStateStoragePair &affectedPair, bool isRemoved);
    void PrintQueueWithLock() const;
    bool PostRequestToCCM(const std::shared_ptr<ISerializable>& pPayload, const AppStateStoragePairMsg::MsgType type);
    void DeRegisterPairing(AppStateStoragePair &pairingToDeReg);
    void RegisterPairing(AppStateStoragePair &pairingToReg);
    void HandleIncomingPairingRequest(AppStateStoragePair &incoming);
    void ProcessIncomingParingRequestIfLeaderFlushOtherwise();
    std::unique_ptr<AppStateStoragePair> GetStoragePairEntryFromQueue();
    void HandleAddReq(AppStateStoragePairMsg& msg);
    void HandleRemoveReq(AppStateStoragePairMsg& msg);
    void HandleUpcallMessage(AppStateStoragePairMsg& msg);
    void UpcallMethod(const ClientMessage& commitedMsg);
    std::shared_ptr<ICCM> m_pIccm{nullptr};
    std::unique_ptr<AppStateStoragePairReg> m_pAppStateReg{nullptr};
    mutable std::mutex m_queueMutex;
    std::vector<std::unique_ptr<AppStateStoragePair>> m_appStoragePairQueue{};
    StateStorageChangeCallbackType m_changeCb{nullptr};

    
    
};

#endif //SSPR_MAIN_H
