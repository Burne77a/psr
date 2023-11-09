#ifndef SR_MAIN_H
#define SR_MAIN_H
#include "StorageReg.h"
#include "StorageInfoMsg.h"
#include "../../ccm/ICCM.h"
#include <memory>
#include <string>

using StorageInfoChangeStorageAddedCallbackType = std::function<void(const unsigned int storageId)>;
using StorageInfoChangeStorageRemovedCallbackType = std::function<void(const unsigned int storageId)>;

class SR
{
  public:
    static const unsigned int SERVICE_ID = 11;
    static std::unique_ptr<SR> CreateSR(std::shared_ptr<ICCM>& pIccm);
    SR(std::shared_ptr<ICCM>& pIccm,std::unique_ptr<StorageReg>& pStorageReg);
    ~SR() = default;
    bool RegisterWithCCM();
    bool RegisterStorage(const unsigned int storageId, const std::string_view ipAddr, const unsigned int spaceInBytes, const unsigned int bandwidth);
    bool DeRegisterStorage(const unsigned int storageId);
    void RegisterStorageAddedCb(StorageInfoChangeStorageAddedCallbackType callback);
    void RegisterStorageRemovedCb(StorageInfoChangeStorageRemovedCallbackType callback);
    std::optional<StorageInfo> GetStorageInfo(const unsigned int storageId) const {return m_pStorageReg->GetStorageInfo(storageId);}
    void GetAllStorageIds(std::vector<unsigned int> &storageIds) const {return m_pStorageReg->GetAllStorageIds(storageIds);}
    void Print() const;
  private:
    bool PostRequestToCCM(const std::shared_ptr<ISerializable>& pPayload, const StorageInfoMsg::MsgType type);
    void HandleRemoveReq(StorageInfoMsg & msg);
    void HandleAddReq(StorageInfoMsg & msg);
    void HandleUpcallMessage(StorageInfoMsg & msg);
    void UpcallMethod(const ClientMessage& commitedMsg);
    
    std::shared_ptr<ICCM> m_pIccm{nullptr};
    std::unique_ptr<StorageReg> m_pStorageReg{nullptr};
    StorageInfoChangeStorageAddedCallbackType m_storageAddedCb{nullptr};
    StorageInfoChangeStorageRemovedCallbackType m_storageRemovedCb{nullptr};
    
};

#endif //SR_MAIN_H
