#ifndef AIR_APPINFO_H
#define AIR_APPINFO_H
#include "ISerializable.h"
#include <memory>
#include <string>
class StorageInfo : public ISerializable
{
  public:
    static const unsigned int INVALID_VALUE{0U};
    static const unsigned int INVALID_APP_ID{INVALID_VALUE};
  private:
    struct StorageInfoData
    {
     unsigned int storageId{INVALID_APP_ID};
     char ipAddr[20];
     unsigned int sizeInBytes{INVALID_VALUE};
     unsigned int bandwidth{0U}; 
    };
  public: 
    static std::shared_ptr<StorageInfo> CreateFromRawPtr(const uint8_t * pBuffer, const uint32_t size);
    StorageInfo(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int periodInMs, const unsigned int bytesToSyncEachPeriod);
    StorageInfo(const unsigned int appId);
    StorageInfo();
    ~StorageInfo() = default;
     unsigned int GetId() const {return m_data.storageId;}
     unsigned int GetIpAddr const {return std::string{m_data.ipAddr};}
     unsigned int GetPeriodInMs() const {return m_data.periodInMs;}
     unsigned int GetBytesToSyncEachPeriod() const {return m_data.bytesToSyncEachPeriod;}
    //ISerializable
     const uint8_t *  Serialize(uint32_t &size) const override;
     const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
     bool Deserialize() override;
   void Print() const; 
  private:
     AppInfoData m_data;
};

#endif //AIR_APPINFOENTRY_H
