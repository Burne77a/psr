#ifndef SSPR_APPSTATE_STORAGE_PAIR_H
#define SSPR_APPSTATE_STORAGE_PAIR_H
#include "ISerializable.h"
#include <memory>
#include <string>
class AppStateStoragePair: public ISerializable
{
  public:
    static const unsigned int INVALID_VALUE{0U};
  private:
    struct AppStateStorageInfoData
    {
     unsigned int appId{INVALID_VALUE};
     unsigned int primaryNodeId{INVALID_VALUE};
     unsigned int storageId{INVALID_VALUE};
     unsigned int storageNodeId{INVALID_VALUE};
     char storageIpAddr[20];
    };
  public: 
    static std::shared_ptr<AppStateStoragePair> CreateFromRawPtr(const uint8_t * pBuffer, const uint32_t size);
    AppStateStoragePair(const unsigned int appId, const unsigned int storageId, std::string_view stateStorageIpAddr, const unsigned int storageNodeId,const unsigned int primaryNodeId);
    AppStateStoragePair(const unsigned int appId);
    AppStateStoragePair();
    ~AppStateStoragePair() = default;
    unsigned int GetAppId() const {return m_data.appId;}
    unsigned int GetStorageId() const {return m_data.storageId;}
    unsigned int GetStorageNodeId() const{return m_data.storageNodeId;}
    unsigned int GetPrimaryAppNodeId() const {return m_data.primaryNodeId;}
    bool IsPaired() const {return m_data.storageId != INVALID_VALUE;}
    void SetStorage(const unsigned int storageId, std::string_view storageIpAddr,const unsigned int storageNodeId,const unsigned int primaryNodeId);
    std::string GetIpAddr() const {return std::string{m_data.storageIpAddr};}
    
    //ISerializable
    const uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    void Print() const; 
  private:
   AppStateStorageInfoData m_data;
};

#endif //SSPR_APPSTATE_STORAGE_PAIR_H
